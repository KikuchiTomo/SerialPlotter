#include <stdio.h>
#include "plotter.hpp"
#include "shared_mem.hpp"
#include "pack_type.hpp"
#include <math.h>
#include <random>

int main(void) {    
    Serial::Plotter p = Serial::Plotter();

    IPC::SharedMemory shm = IPC::SharedMemory("~/.shm", 51);
    shm.create<PlotterType>();
    auto containers = shm.attach<PlotterType>();

    p.figure();
    p.addSubplot({1, 3, 1});
    p.addSubplot({1, 3, 2}); 
    p.addSubplot({1, 3, 3}); 

    p.xtics(100.0, {1, 3, 2});   
    
    while(1){
        auto container = containers->buf;
        p.xrange({container[0].time - 1000, container[0].time}, {1, 3, 1});
        p.xrange({container[0].time - 1000, container[0].time}, {1, 3, 2});
        p.xrange({container[0].time - 1000, container[0].time}, {1, 3, 3});

        p.addData({1, 3, 1}, container[0].time, container[0].x);
        p.addData({1, 3, 2}, container[0].time, container[0].y);
        p.addData({1, 3, 3}, container[0].time, container[0].z);

        int key = p.draw(1);
        if(key == 'q'){
            break;
        }
    }

    shm.discard();
    return 0;
}