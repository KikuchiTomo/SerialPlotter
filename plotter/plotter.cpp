#include <stdio.h>
#include "plotter.hpp"
#include "shared_mem.hpp"
#include "pack_type.hpp"
#include <math.h>
#include <pthread.h>
#include <random>

int main(void) {    
    Serial::Plotter p = Serial::Plotter();

    IPC::SharedMemory shm = IPC::SharedMemory("~/.shm", 51);
    shm.create<PlotterType>();        
    auto containers = shm.attach<PlotterType>();
    shm.init<PlotterType>(containers);

    p.figure();
    p.addSubplot({1, 3, 1});
    p.addSubplot({1, 3, 2}); 
    p.addSubplot({1, 3, 3}); 

    p.xdeci(2, {1, 3, 1});
    p.xdeci(2, {1, 3, 2});
    p.xdeci(2, {1, 3, 3});

    p.xtics(5.0, {1, 3, 1});     
    p.xtics(5.0, {1, 3, 2});   
    p.xtics(5.0, {1, 3, 3}); 

    p.ytics(0.2, {1, 3, 1});     
    p.ytics(0.2, {1, 3, 2});   
    p.ytics(0.2, {1, 3, 3}); 

    p.yrange({0.0, 1.0}, {1, 3, 1});
    p.yrange({0.0, 1.0}, {1, 3, 2});
    p.yrange({0.0, 1.0}, {1, 3, 3});
    
    while(1){
        auto c = shm.read<PlotterType>(containers);
        if(c.wx) p.xrange({c.time - 20, c.time}, {1, 3, 1});
        if(c.wy) p.xrange({c.time - 20, c.time}, {1, 3, 2});
        if(c.wz) p.xrange({c.time - 20, c.time}, {1, 3, 3});
        if(c.wx) p.addData({1, 3, 1}, c.time, c.x);
        if(c.wy) p.addData({1, 3, 2}, c.time, c.y);
        if(c.wz) p.addData({1, 3, 3}, c.time, c.z);

        int key = p.draw(1);
        // if(c.wx || c.wy || c.wz) printf("(%f) x: %f, y: %f, z: %f\n", c.time, c.x, c.y, c.z);       

        if (key == 'q') {
            break;
        }
    }

    shm.discard();
    return 0;
}