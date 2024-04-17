#ifndef S_SERIAL_HPP__
#define S_SERIAL_HPP__

#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "logger.hpp"
#include "ring_vector.hpp"
#include "serial_config.hpp"

#define BUFFER_SIZE (2048)

namespace Serial {
class SerialDevice {
  private:   
    int baud_rate_;

    char config_;
    char *port_name_;

    int fd_;
    int rv_;

    Serial::RingVector<unsigned char> *rb_;
    Serial::RingVector<unsigned char> *sb_;

    char *tmp_rb_;
    char *tmp_sb_;

    long long int timeout_s_ = 0;
    long long int timeout_us_ = 0;

    fd_set set_;
    struct timeval timeout_ = {0, 0};

    speed_t baudrate(int baud_rate) {
        switch (baud_rate) {
        case 9600:
            return B9600;
            break;
        case 19200:
            return B19200;
            break;
        case 38400:
            return B38400;
            break;
        case 57600:
            return B57600;
            break;
        case 115200:
            return B115200;
            break;
        case 230400:
            return B230400;
            break;
        default:
            SCritical("fail to convert baud rate. bye.");
            exit(-1);
            break;
        }
    }

  public:
    SerialDevice(long long int bufferSize = BUFFER_SIZE) {
        rb_ = new Serial::RingVector<unsigned char>(bufferSize * 4, 0);
        sb_ = new Serial::RingVector<unsigned char>(bufferSize * 4, 0);

        tmp_rb_ = (char *)calloc(sizeof(char), bufferSize);
        tmp_sb_ = (char *)calloc(sizeof(char), bufferSize);

        port_name_ = (char *)malloc(sizeof(char) * 512);
    }

    ~SerialDevice() {
        delete rb_;
        delete sb_;

        free(tmp_rb_);
        free(tmp_sb_);
    }

    void setPort(const char *port_name) { strcpy(port_name_, port_name); }

    void setTimeout(long long int sec, long long int usec) {
        timeout_s_ = sec;
        timeout_us_ = usec;
    }

    bool begin(int baud_rate, format_t config) {
        // Read/Write
        fd_ = open(port_name_, O_RDWR | O_NONBLOCK);
        struct termios tio;

        if (fd_ < 0) {
            SError("Cannot open device (%s)\n", port_name_);
            return false;
        }

        if (tcgetattr(fd_, &tio) < 0) {
            SError("Cannot get terminal attributes\n");
            return false;
        }

        cfsetspeed(&tio, baudrate(baud_rate));

        tio.c_cflag &= ~CSIZE;
        // 長さ設定
        switch (config.len) {
        case DataLength::Len5:
            tio.c_cflag |= CS5;
            break;
        case DataLength::Len6:
            tio.c_cflag |= CS6;
            break;
        case DataLength::Len7:
            tio.c_cflag |= CS7;
            break;
        case DataLength::Len8:
            tio.c_cflag |= CS8;
            break;
        }

        // パリティの設定
        switch (config.parity) {
        case Parity::None:
            tio.c_cflag &= ~PARENB;
            break;
        case Parity::Odd:
            tio.c_cflag |= PARENB;
            tio.c_cflag |= PARODD;
            break;
        case Parity::Even:
            tio.c_cflag |= PARENB;
            tio.c_cflag &= ~PARODD;
            break;
        }

        // ストップビットの設定
        switch (config.stop) {
        case StopBit::One:
            tio.c_cflag &= ~CSTOPB;
            break;
        case StopBit::Two:
            tio.c_cflag |= CSTOPB;
            break;
        }

        tio.c_cflag &= ~CRTSCTS;
        tio.c_cflag |= CREAD | CLOCAL;
        tio.c_iflag &= ~(IXON | IXOFF | IXANY);

        tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        tio.c_oflag &= ~OPOST;
        tio.c_cc[VMIN] = 0;
        tio.c_cc[VTIME] = 0;

        tcsetattr(fd_, TCSANOW, &tio);

        if (tcsetattr(fd_, TCSAFLUSH, &tio) < 0) {
            SError("Cannot get terminal attributes\n");
            return false;
        }

        SNotice("Opened serial port. %s", port_name_);
        return true;
    }

    void end() {
        SNotice("Closed serial port. %s", port_name_);
        close(fd_);
    }

    unsigned char popRecvData(bool *fail) { return rb_->popl(fail); }
    unsigned char readRecvData(int offset = 0) { return rb_->readl(offset); }    
    void setSendData(unsigned char *data, int len) { sb_->push(data, len); }
    int getRecvSize() { return rb_->getDataLength(); }
    int getSendSize() { return sb_->getDataLength(); }

    void clearSendBuffer() { sb_->resetIndexes(); }
    void clearRecvBuffer() { rb_->resetIndexes(); }

    void recv() {
        FD_ZERO(&set_);
        FD_SET(fd_, &set_);

        timeout_.tv_sec = timeout_s_;
        timeout_.tv_usec = timeout_us_;

        // 受信バッファに何もないなら抜ける
        rv_ = select(fd_ + 1, &set_, NULL, NULL, &timeout_);
        if (rv_ <= 0) {
            return;
        }

        // 受信
        int ret = read(fd_, tmp_rb_, BUFFER_SIZE);
        if (ret <= 0) {
            STrace("No data received.\n");
            return;
        }

        rb_->push((unsigned char*)tmp_rb_, ret);        
    }

    void send() {
        if (sb_->getDataLength() <= 0) {
            return;
        }
        tcflush(fd_, TCIOFLUSH);
        long long int send_len = sb_->getDataLength();
        for (int i = 0; i < send_len; i++) {
            tmp_sb_[i] = sb_->popl();
        }

        write(fd_, tmp_sb_, send_len);    
    }
};

} // namespace Serial
#endif