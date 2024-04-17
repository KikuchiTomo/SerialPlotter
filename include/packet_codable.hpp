#ifndef S_SERIAL_PARSER_HPP__
#define S_SERIAL_PARSER_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// マイコン側でも再利用できるように依存性をなくす.

#define SERIAL_CODER_BUFFER_SIZE (512)

namespace Serial::Codable {
typedef struct {
    unsigned char *body;
    unsigned char length;
} Packet;

class BaseEncoder {
  public:
    // virtual ~BaseEncoder(){};
    virtual Packet *encode(Packet &packet);
    virtual void encode(Packet *packet);
};

class BaseDecoder {
  public:
    // virtual ~BaseDecoder(){};
    virtual Packet *decode(Packet &packet);
    virtual void decode(Packet *packet);
};

// Consistent Overhead Byte Stuffing
class COBSCoder {
  private:
    unsigned char *zero_indexes_ = NULL;
    unsigned char *zero_diffs_ = NULL;
    unsigned char *tmp_buffer_ = NULL;

    int __encoder(Packet &packet) {
        for (int i = 0; i < SERIAL_CODER_BUFFER_SIZE; i++) {
            zero_indexes_[i] = zero_diffs_[i] = tmp_buffer_[i] = 0;
        }

        // 処理ようのバッファにコピー
        memcpy(tmp_buffer_, packet.body, sizeof(unsigned char) * packet.length);
        // 末尾に0をくわえる
        tmp_buffer_[packet.length] = 0x00;
        unsigned char tmp_length = packet.length + 1;

        // ゼロの位置を探索
        unsigned char next_zero_position = 0;
        unsigned char zero_buf_length = 0;
        for (int i = 0; i < tmp_length; i++) {
            next_zero_position++;
            if (tmp_buffer_[i] == 0x00) {
                zero_indexes_[zero_buf_length] = i;
                zero_diffs_[zero_buf_length] = next_zero_position;
                zero_buf_length++;
                next_zero_position = 0;
            }
        }

        tmp_length += 1;

        // 得られた位置に挿入
        for (int i = 0; i < zero_buf_length - 1; i++) {
            tmp_buffer_[zero_indexes_[i]] = zero_diffs_[i + 1];
        }

        // ずらす.
        memmove(&tmp_buffer_[1], tmp_buffer_,
                sizeof(unsigned char) * tmp_length);
        tmp_buffer_[0] = zero_diffs_[0];

        return tmp_length;
    }

    int __decode(Packet &packet) {
        for (int i = 0; i < SERIAL_CODER_BUFFER_SIZE; i++) {
            zero_indexes_[i] = zero_diffs_[i] = tmp_buffer_[i] = 0;
        }

        memcpy(tmp_buffer_, packet.body, sizeof(unsigned char) * packet.length);
        unsigned char tmp_length = packet.length;
        unsigned char next_zero_position = tmp_buffer_[0];

        while (true) {
            unsigned char diff = tmp_buffer_[next_zero_position];
            if (diff == 0x00) {
                break;
            }
            tmp_buffer_[next_zero_position] = 0x00;
            next_zero_position += diff;
        }

        memmove(tmp_buffer_, &tmp_buffer_[1], sizeof(unsigned char) * tmp_length);
        tmp_length -= 2;

        return tmp_length;
    }

  public:
    COBSCoder() {
        // TODO: zero_*_は構造体にまとめる
        zero_indexes_ = (unsigned char *)malloc(sizeof(unsigned char) *
                                                SERIAL_CODER_BUFFER_SIZE);
        zero_diffs_ = (unsigned char *)malloc(sizeof(unsigned char) *
                                              SERIAL_CODER_BUFFER_SIZE);
        tmp_buffer_ = (unsigned char *)malloc(sizeof(unsigned char) *
                                              SERIAL_CODER_BUFFER_SIZE);
    }

    ~COBSCoder() {
        free(zero_indexes_);
        free(zero_diffs_);
        free(tmp_buffer_);
    }

    Packet *encode(Packet &packet) {
        unsigned char len = __encoder(packet);
        unsigned char *buf =
            (unsigned char *)malloc(sizeof(unsigned char) * len);
        Packet *packed = (Packet*) malloc(sizeof(Packet));
        packed->body = buf;
        packed->length = len;
        encode(packed);
        return packed;
    }

    void encode(Packet *packet) {
        unsigned char  len = __encoder(*packet);
        memcpy(packet->body, tmp_buffer_, sizeof(unsigned char) * len);
        packet->length = len;
    }

    Packet *decode(Packet &packet) { return NULL; }

    void decode(Packet *packet) {
        unsigned char  len = __decode(*packet);
        memcpy(packet->body, tmp_buffer_, sizeof(unsigned char) * len);
        packet->length = len;
    }
};
}; // namespace Serial::Codable

#endif