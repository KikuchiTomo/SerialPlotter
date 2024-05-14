#ifndef S_SERIAL_PROCESSER_HPP__
#define S_SERIAL_PROCESSER_HPP__

#include "config.hpp"
#include "logger.hpp"
#include "serial.hpp"
#include "serial_process.hpp"
#include "serial_config.hpp"
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

class SerialProcesserDelegate {
  public:
    virtual ~SerialProcesserDelegate(){};
    virtual void serialProcesserConnected(int id) = 0;
    virtual void serialProcesserDisconnected(int id) = 0;
    virtual void serialProcesserTimeout(int id) = 0;
    virtual void serialProcesserFailToConnect(int id) = 0;
};

class SerialProcesser {
  private:
    SerialProcesser() { SNotice("Launch SerialProcesser") }
    ~SerialProcesser() { discardAllConnection(); }

    static SerialProcesser *instance;
    std::vector<Serial::SerialProcess *> processes_;
    int latest_connection_id_ = 0;

  public:
    SerialProcesserDelegate *delegate = NULL;

    SerialProcesser(const SerialProcesser &) = delete;
    SerialProcesser &operator=(const SerialProcesser &) = delete;
    SerialProcesser(SerialProcesser &&) = delete;
    SerialProcesser &operator=(SerialProcesser &&) = delete;

    static SerialProcesser &shared() { return *instance; }

    static void init() {
        if (!instance) {
            instance = new SerialProcesser;
        }
    }

    static void dispose() {
        delete instance;        
        instance = nullptr;
    }

    int getConnectionCount() { return processes_.size(); }
    // For MacOS/Linux
    bool getPorts(std::vector<std::string> &ports) {
        using namespace std::filesystem;
        std::string directory = SERIAL_DEVICE_PORTS_PATH;
        directory_iterator iter(directory), end;
        std::error_code err;

    //  TODO: 適当すぎるので後で直す
        std::string needs_0 = "cu";
        std::string needs_1 = "usb";
        for (; iter != end && !err; iter.increment(err)) {
            const directory_entry entry = *iter;
            std::string path = entry.path().string();
            bool found_0 = path.find(needs_0) != std::string::npos;
            bool found_1 = path.find(needs_1) != std::string::npos;

            if(!found_0 && !found_1) {
                continue;
            }
            
            ports.push_back(entry.path().string());
        }

        if (err) {
            SError("%s", err.message().c_str());
            return false;
        }

        return true;
    }

    int createConnection(const char *port_name, int baud) {
        latest_connection_id_ += 1;
        Serial::SerialProcess *connection =
            new Serial::SerialProcess(latest_connection_id_);
        connection->selectPort(port_name);
        bool is_connected = connection->connect(baud);

        if (is_connected) {
            // 接続できたらコネクションズに追加．
            if (delegate != NULL) {
                delegate->serialProcesserConnected(latest_connection_id_);
            }

            processes_.push_back(connection);
        } else {
            // 接続できないのならコネクションを消す
            if (delegate != NULL) {
                delegate->serialProcesserFailToConnect(latest_connection_id_);
            }

            connection->disconnect();
            delete connection;
            return -1;
        }

        return latest_connection_id_;
    }

    void runConnection(int id) {
        for (Serial::SerialProcess *connection : processes_) {
            if (connection->connectionId() == id) {
                connection->run();
            }
        }
    }

    void startRecording(){
        time_t t;
        char fname[64];


        t = time(NULL);
        strftime(fname, sizeof(fname), "data-%y%m%d-%H%M%S.csv", localtime(&t));
        for (Serial::SerialProcess *connection : processes_) {
            char fpath[64];
            sprintf(fpath, "outputs/%d-%s", connection->connectionId(), fname);
            connection->setRecord(fpath);
        }
    }

    void endRecording(){
        for (Serial::SerialProcess *connection : processes_) {
            connection->unsetRecord();
        }
    }

    void discardConnection(int id) {
        // TODO: リファクタ
        bool is_contain = false;
        int index = 0;
        for (Serial::SerialProcess *connection : processes_) {
            if (connection->connectionId() == id) {
                connection->stop();
                connection->disconnect();
                is_contain = true;
                index++;
                break;
            }
        }

        if(is_contain){
            processes_.erase(processes_.begin() + index);
        }
    }

    void discardAllConnection(){
        int index = 0;
        for (Serial::SerialProcess *connection : processes_) {
            connection->stop();
            connection->disconnect();
            delete connection;
        }
    }
};

SerialProcesser *SerialProcesser::instance = nullptr;

#endif