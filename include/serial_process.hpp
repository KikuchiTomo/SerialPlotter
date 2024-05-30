#ifndef S_SERIAL_PROCESS_HPP__
#define S_SERIAL_PROCESS_HPP__

#include "config.hpp"
#include "logger.hpp"
#include "pack_type.hpp"
#include "packet_codable.hpp"
#include "packet_definition.hpp"
#include "plotter.hpp"
#include "serial.hpp"
#include "serial_config.hpp"
#include "shared_mem.hpp"
#include <thread>
#define PLOT_N (10)

namespace Serial {
// プロットの処理がわにmsgqueueを作ってSertialProcessが増えたら描画数を増やす必要あり
static IPC::SharedMemory shm = IPC::SharedMemory("~/.shm", 51);

class SerialProcess {
  private:
    SerialDevice *dev_ = NULL;
    unsigned char *tmp_ = NULL;
    unsigned char *buf_ = NULL;
    bool is_threading_ = true;
    Codable::COBSCoder *coder_ = NULL;
    // IPC::SharedMemory::Container<PlotterType, 1024> *p_ = NULL;
    // bool isPlotFirst_ = true;

    std::thread th_;
    FILE *fp_ = NULL;
    bool recording_ = false;
    IPC::SharedMemory::Container<PlotterType, 1024> *container_;
    IPC::SharedMemory *shm_;
    bool prev_recording_ = false;
    int index_ = 0;
    int id_;
    int plot_count_x_ = 0;
    int plot_count_ = 0;
    int plot_count_y_ = 0;
    int plot_count_z_ = 0;

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
        shm.get();
        container_ = shm.attach<PlotterType>();
        SNotice("Shared Mem -> Get");
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
        // shm_->get();
        // container_ = shm_->attach<PlotterType>();
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

            if (recording_) {
                if (fp_ != NULL) {
                    fprintf(fp_, "%u %u %u %u %u %u\n", data.packet.seq,
                            data.packet.pag, data.packet.pos, data.packet.tm,
                            data.packet.ch, data.packet.val);
                }
            }

            double v = (double)data.packet.val / 4096.0;
            double t = (double)data.packet.tm / 1000.0;
            if (data.packet.ch == 1 && plot_count_x_ == 0) {
                shm.write<PlotterType>(container_,
                                       {v, 0.0, 0.0, true, false, false, t});
                plot_count_x_ = PLOT_N;
            }

            if (data.packet.ch == 2 && plot_count_y_ == 0) {
                shm.write<PlotterType>(container_,
                                       {0.0, v, 0.0, false, true, false, t});
                plot_count_y_ = PLOT_N;
            }

            if (data.packet.ch == 3 && plot_count_z_ == 0) {
                shm.write<PlotterType>(container_,
                                       {0.0, 0.0, v, false, false, true, t});
                plot_count_z_ = PLOT_N;
            }

            // TODO: plot_count_x_ -> will_plot_x にrenameしてboolにかえる
            if (plot_count_x_ == PLOT_N && plot_count_y_ == PLOT_N &&
                plot_count_z_ == PLOT_N) {
                plot_count_++;
            }

            if (plot_count_ > PLOT_N) {
                plot_count_x_ = 0;
                plot_count_y_ = 0;
                plot_count_z_ = 0;
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