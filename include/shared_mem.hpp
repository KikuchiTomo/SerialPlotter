#ifndef SHARED_MEM_HPP__
#define SHARED_MEM_HPP__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

namespace IPC {
class SharedMemory {
    public:
     template <typename T, int N> struct container {
        T buf[N];
        const long long int len = N;
        pthread_mutex_t mutex;
    };

    template <typename T, int N> using Container = struct container<T, N>;

  private:
    key_t key_;
    int seg_id_ = 0;
    void *ptr_ = NULL;

  public:
    SharedMemory(std::string key, int id) {
        FILE *fp = fopen(key.c_str(), "a");
        fclose(fp);

        key_ = ftok(key.c_str(), id);
        if (key_ == -1) {
            // TODO: error handling
        }
    }

    ~SharedMemory() {}

    template <typename T, int N = 1024> bool create() {
        int size = sizeof(Container<T, N>);
        seg_id_ = shmget(key_, size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
        if (seg_id_ == -1) {
            return false;
        }

        return true;
    }

    bool get() {
        seg_id_ = shmget(key_, 0, 0);
        if (seg_id_ == -1) {
            return false;
        }

        return true;
    }

    template <typename T, int N = 1024> Container<T, N> *attach() {
        Container<T, N> *memory = NULL;
        ptr_ = shmat(seg_id_, 0, 0);
        memory = (Container<T, N> *)ptr_;
        return memory;
    }

    void detach() {
        if (ptr_ != NULL) {
            shmdt(ptr_);
            ptr_ = NULL;
        }
    }
    void discard() { shmctl(seg_id_, IPC_RMID, NULL); }
};

class MessageQueue {
    public:
    template <int N = 1024> struct container {
        long mtype;
        char mtext[N];     
    };

    template <int N>
    using Container = struct container<N>;

  private:
    key_t key_;
    int seg_id_ = 0;
    void *ptr_ = NULL;

public:
      MessageQueue(std::string key, int id) {
        FILE *fp = fopen(key.c_str(), "a");
        fclose(fp);

        key_ = ftok(key.c_str(), id);
        if (key_ == -1) {
            // TODO: error handling
        }
    }

    ~MessageQueue() {}

    bool create() {       
        seg_id_ = msgget(key_, IPC_CREAT | 0666);        
        if (seg_id_ == -1) {
            return false;
        }

        return true;
    }

    bool get() {
        seg_id_ = msgget(key_, 0666);        
        if (seg_id_ == -1) {
            return false;
        }

        return true;
    }

    template <int N = 1024> 
    bool receive(Container<N> *data, bool blocking = true) {     
        int result = msgrcv(seg_id_, data, sizeof(data->mtext), 0,
                   (blocking) ? 0 : IPC_NOWAIT);
        if(result <= 0){                      
            return false;
        }else{           
            return true;
        }
    }

    template <int N = 1024> 
    void send(Container<N> *data){
        if(seg_id_ > 0){
            data->mtype = (long) getpid();
            msgsnd(seg_id_, (const void *)data, sizeof(data->mtext), 0);           
        }
    }
   
    void discard() { msgctl(seg_id_, IPC_RMID, 0); }    

};
} // namespace IPC

#endif