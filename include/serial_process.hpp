#ifndef S_SERIAL_PROCESS_HPP__
#define S_SERIAL_PROCESS_HPP__

#include "config.hpp"
#include "logger.hpp"
#include "packet_codable.hpp"
#include "packet_definition.hpp"
#include "serial.hpp"
#include "serial_config.hpp"
#include <thread>

namespace Serial {
class SerialProcess {
  private:
    SerialDevice *dev_ = NULL;
    unsigned char *tmp_ = NULL;
    unsigned char *buf_ = NULL;
    bool is_threading_ = true;
    Codable::COBSCoder *coder_ = NULL;
    std::thread th_;
    FILE *fp_ = NULL;
    bool recording_ = false;
    bool prev_recording_ = false;
    int index_ = 0;
    int id_;

  public:
    SerialProcess(int id) : th_{[this] { __serialRoutine(); }} {
        id_ = id;
        tmp_ = (unsigned char *)malloc(sizeof(unsigned char) * 1024);
        buf_ = (unsigned char *)malloc(sizeof(unsigned char) * 1024);
        coder_ = new Codable::COBSCoder();
        init();
        SNotice("Launch SerialProcess: %d", id_);
    }

    ~SerialProcess() {
        free(tmp_);
        free(buf_);
        delete coder_;
        disconnect();
        SNotice("Finished SerialProcess: %d", id_);
    }

    int connectionId() { return id_; }
    void init() { dev_ = new Serial::SerialDevice(); }
    void selectPort(const char *port_name) { dev_->setPort(port_name); }
    bool connect(int baud_rate) {
        return dev_->begin(baud_rate, Serial::FormatType(Serial::Format::S8N1));
    }

    void disconnect() { dev_->end(); }

    void timeout(long long int sec, long long int usec) {
        dev_->setTimeout(sec, usec);
    }

    void setRecord(const char *fpath) {
        fp_ = fopen(fpath, "a+");
        if (fp_ == NULL) {
            SError("Couldn't open file : %s", fpath);
            return;
        }

        recording_ = true;
    }

    void unsetRecord() { recording_ = false; }

    void run() {
        is_threading_ = true;

        // thread_ = &process;
        th_.join();
    }

    void stop() { is_threading_ = false; }

  private:
    void __loop() {
        int i;
        int tmp_index = 0;

        int len = dev_->getRecvSize();
        if (len <= sizeof(Definition::BasePacket)) {
            return;
        }

        // SNotice("recieved %d", len);

        for (i = 0; i < len; i++) {
            bool fail = false;
            unsigned char byte = dev_->popRecvData(&fail);
            if (fail) {
                SError("pop failed");
                continue;
            }

            // printf(" %02X", byte);
            buf_[index_++] = byte;

            if (byte == 0x00) {
                tmp_index = index_;
                // printf("\n");
                index_ = 0;
                break;
            }
        }

        if (index_ == 0) {
            Serial::Codable::Packet packed = {buf_, (unsigned char)tmp_index};
            coder_->decode(&packed);

            Definition::Packet data;
            for (i = 0; i < sizeof(Definition::BasePacket); i++) {
                data.bin[i] = packed.body[i];
            }

            data.decodePacket();

            // SNotice("* %u %u %lu %lu %lu %lu %lu %ld", data.packet.dst,
            // data.packet.src, data.packet.seq, data.packet.pag,
            // data.packet.pos, data.packet.tm, data.packet.ch,
            // data.packet.val);
            if (recording_) {
                if(fp_ != NULL){
                    fprintf(fp_, "%u %u %u %u %u %u\n",
                            data.packet.seq, data.packet.pag, data.packet.pos, data.packet.tm,
                            data.packet.ch, data.packet.val);
                }
            }

            // on stop recording
            if ((prev_recording_ != recording_) && recording_ == false) {
                fclose(fp_);
                fp_ = NULL;
            }

            prev_recording_ = recording_;

        } else {
            SError("expected %d bytes, but size of packed.length is %d",
                   sizeof(Definition::BasePacket), tmp_index);
        }
    }

  public:
    void __serialRoutine() {
        SInfo("Serial Process Start....");
        while (is_threading_) {
            dev_->recv();
            dev_->send();
            __loop();
        }
        SInfo("Serial Process End....");
    }
};
}; // namespace Serial
#endif