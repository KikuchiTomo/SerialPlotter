#include <stdio.h>
#include "plotter.hpp"
#include "shared_mem.hpp"
#include "pack_type.hpp"
#include <math.h>
#include <pthread.h>
#include <random>

int main(void) {       
    IPC::SharedMemory shm = IPC::SharedMemory("~/.shm", 51);

    sleep(4);
    shm.get();
    auto containers = shm.attach<PlotterType>();       
    double x = 0;

    while(1){
        double r0 = sin(x * 3.0);
        double r1 = sin(x * 6.0);
        double r2 = sin(x * 12.0);

        shm.write<PlotterType>(containers, {r0, r1, r2, true, true, true, x});        

        x += 0.005;

        usleep(1000);
    }

    return 0;
}