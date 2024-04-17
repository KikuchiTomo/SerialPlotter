#ifndef S_RING_VECTOR_HPP__
#define S_RING_VECTOR_HPP__

#ifdef __APPLE__
#include <stdlib.h>
#elif __GNUC__
#include <malloc.h>
#endif

#include <algorithm>
#include <mutex>

namespace Serial {
template <class T> class RingVector {
  private:
    T *b_;

    // buffer capacity
    long long int s_;
    // data length
    long long int l_;
    // head index
    long long int h_;
    // tail index
    long long int t_;
    // default value
    T d_;

    std::mutex m_;

  public:
    RingVector(long long int size, T defaultValue) {
        s_ = size;
        l_ = 0;

        h_ = 0;
        t_ = 0;

        b_ = (T *)calloc(sizeof(T), size);
        d_ = defaultValue;
    }

    ~RingVector() { free(b_); }

    long long int getBufferCapacity() { return s_; }

    long long int getDataLength() { return l_; }

    void push(T data, bool *fail = NULL) {
        // if (!m_.try_lock()) {
        //     __fail(fail, true);
        //     return;
        // }

        __fail(fail, true);
        if (s_ > 0 && l_ < s_) {
            h_ = (s_ + h_ - 1) % s_;
            b_[h_] = data;
            l_++;
            __fail(fail, false);
        }

        // m_.unlock();
    }

    void push(T *data, int count, bool *fail = NULL) {
        // if (!m_.try_lock()) {
        //     __fail(fail, true);
        //     return;
        // }

        __fail(fail, true);
        if (s_ > 0 && l_ + count < s_) {
            // TODO: 折り返し分の位置を計算して, memcopy or simdで行うべき
            for (int i = 0; i < count; i++) {
                h_ = (s_ + h_ - 1) % s_;
                b_[h_] = data[i];
            }
            l_ += count;
            __fail(fail, false);
        }

        // m_.unlock();
    }

    void write(T data, int offset, bool *fail = NULL) {
        if (!m_.try_lock()) {
            __fail(fail, true);
            return;
        }

        __fail(fail, true);
        if (l_ > 0 && s_ > 0 && l_ > offset) {
            int tmp = h_;
            tmp = (tmp + s_ + offset) % s_;
            b_[tmp] = data;
            __fail(fail, false);
        }

        m_.unlock();
    }

    T pop(bool *fail = NULL) {
        if (!m_.try_lock()) {
            __fail(fail, true);
            return d_;
        }

        __fail(fail, true);

        T tmp = d_;
        if (l_ > 0 && s_ > 0) {
            tmp = b_[h_];
            h_ = (s_ + h_ + 1) % s_;
            l_--;
            __fail(fail, false);
        }

        m_.unlock();
        return tmp;
    }

    T read(int offset, bool *fail = NULL) {
        if (!m_.try_lock()) {
            __fail(fail, true);
            return;
        }

        __fail(fail, true);

        T tmp = d_;

        if (l_ > 0 && s_ > 0) {
            int idx = h_;
            idx = (idx + s_ + offset) % s_;
            tmp = b_[idx];

            __fail(fail, false);
        }

        m_.unlock();
        return tmp;
    }

    void pushl(T data, bool *fail = NULL) {
        if (!m_.try_lock()) {
            __fail(fail, true);
            return;
        }

        __fail(fail, true);

        if (s_ > 0 && l_ < s_) {
            b_[t_] = data;
            t_ = (s_ + t_ + 1) % s_;
            l_++;

            __fail(fail, false);
        }

        m_.unlock();
    }

    void pushl(T *data, int count, bool *fail = NULL) {
        if (!m_.try_lock()) {
            __fail(fail, true);
            return;
        }

        __fail(fail, true);

        if (s_ > 0 && l_ + count < s_) {
            // TODO: 折り返し分の位置を計算して, memcopy or simdを行うべき
            for (int i = 0; i < count; i++) {
                b_[t_] = data[i];
                t_ = (s_ + t_ + 1) % s_;
            }

            l_ += count;

            __fail(fail, false);
        }

        m_.unlock();
    }

    void writel(T data, int offset, bool *fail = NULL) {
        if (!m_.try_lock()) {
            __fail(fail, true);
            return;
        }

        __fail(fail, true);
        if (l_ > 0 && s_ > 0) {
            int tmp = t_;
            tmp = (tmp + s_ - offset - 1) % s_;
            b_[tmp] = data;

            __fail(fail, false);
        }
        m_.unlock();
    }

    T popl(bool *fail = NULL) {
        // printf("aaa\n");
        // if (!m_.try_lock()) {
        //     __fail(fail, true);
        //     return d_;
        // }

        // printf("bbb\n");
        __fail(fail, true);
        T tmp = d_;
        // printf("ccc\n");

        if (l_ > 0 && s_ > 0) {
            t_ = (s_ + t_ - 1) % s_;
            tmp = b_[t_];
            l_--;
            // printf("ddd\n");
            __fail(fail, false);
        }
        // m_.unlock();
        return tmp;
    }

    T readl(int offset, bool *fail = NULL) {
        if (!m_.try_lock()) {
            __fail(fail, true);
            return d_;
        }

        __fail(fail, true);
        T tmp = d_;

        if (l_ > 0 && s_ > 0) {
            int idx = t_;
            idx = (idx + s_ - offset - 1) % s_;
            tmp = b_[idx];
            __fail(fail, false);
        }
        m_.unlock();
        return tmp;
    }

    void resetIndexes() { h_ = t_ = l_ = 0; }

    void resetBuffer() {
        if (!m_.try_lock()) {
            return;
        }
        for (long long int i = 0; i < s_; i++) {
            b_[i] = d_;
        }

        m_.unlock();
    }

  private:
    inline void __fail(bool *fail, bool set) {
        if (fail != NULL) {
            *fail = set;
        }
    }

    bool __extend(long long int size) {
        if (!m_.try_lock()) {
            return true;
        }

        bool exceed_cap = (s_ > size);
        bool exceed_len = (size < l_);
        if (exceed_cap || exceed_len) {
            m_.unlock();
            return true;
        }

        // サイズが確保できたらコピーして無かったことにする
        T *tmp_buffer = (T *)malloc(sizeof(T) * size);
        if (tmp_buffer == NULL) {
            m_.unlock();
            return false;
        }

        // 現在の領域をコピー
        memcpy(tmp_buffer, b_, sizeof(T) * l_);
        b_ = tmp_buffer;
        s_ = size;

        // 前の領域を削除
        free(b_);
        m_.unlock();
        return true;
    }
};
}; // namespace Serial

#endif