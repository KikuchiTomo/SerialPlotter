#include <stdio.h>
#include "plotter.hpp"
#include <math.h>
#include <random>

int main(void) {
    Serial::Plotter p = Serial::Plotter();

    srand(time(NULL));

    p.figure();

    // 2x2もできる
    // p.addSubplot({2, 2, 1});
    // p.addSubplot({2, 2, 2});  
    // p.addSubplot({2, 2, 3}); 
    // p.addSubplot({2, 2, 4}); 

    // 1x2とか
    // p.addSubplot({2, 1, 1});
    // p.addSubplot({2, 1, 2});  

    // 3x1とか
   p.addSubplot({1, 3, 1});
    p.addSubplot({1, 3, 2}); 
    p.addSubplot({1, 3, 3}); 

   
    p.xtics(6.28, {1, 3, 2});
    p.xdeci(2, {1, 3, 2});
    double x = 0;
    p.addData({1, 3, 2}, x - 31.4, 0.0);
    p.xrange({x - 15.7, x + 15.7}, {1, 3, 2});   

    while (1) {
        double r = sin(x * 3.0);
        p.addData({1, 3, 2}, x + 15.7, r);
        p.xrange({x - 15.7, x + 15.7}, {1, 3, 2});
        int key = p.draw(10);
        if(key == 'q'){
            break;
        }
      
        x += 0.01;
    }

    return 0;
}