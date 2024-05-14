#include "packet_codable.hpp"
#include <stdio.h>
#include <stdlib.h>

void printx(const unsigned char* b, int n) { 
    for (int i = 0; i < n; i++){
        printf(" 0x%02X", b[i]);
    }
    printf("\n");
}
int main(void) {
    Serial::Codable::COBSCoder *c = new Serial::Codable::COBSCoder();

    // unsigned char buf0[512] = {0x00};
    // Serial::Codable::Packet p0 = {buf0, 1};
    // c->encode(&p0);
    // c->decode(&p0);

    // unsigned char buf1[512] = {0x00, 0x00};
    // Serial::Codable::Packet p1 = {buf1, 2};
    // c->encode(&p1);
    // c->decode(&p1);

    // unsigned char buf2[512] = {0x11, 0x22, 0x00, 0x33};
    // Serial::Codable::Packet p2 = {buf2, 4};
    // c->encode(&p2);
    // c->decode(&p2);

    // unsigned char buf3[512] = {0x11, 0x22, 0x33, 0x44};
    // Serial::Codable::Packet p3 = {buf3, 4};
    // c->encode(&p3);
    // c->decode(&p3);

    // unsigned char buf4[512] = {0x11, 0x00, 0x00, 0x00};
    // Serial::Codable::Packet p4 = {buf4, 4};
    // c->encode(&p4);
    // c->decode(&p4);

    unsigned char buf5[512] = {0x11, 0x22, 0x33, 0x00, 0x11, 0x22, 0x33, 0x00, 0x11, 0x22, 0x33, 0x00, 0x11, 0x22, 0x33, 0x00, 0x11, 0x22, 0x33, 0x00, 0x11, 0x22, 0x33, 0x00, 0x11, 0x22, 0x33};
    Serial::Codable::Packet p5 = {buf5, 27};
    printx(p5.body, p5.length);
    c->encode(&p5);
    printx(p5.body, p5.length);
    c->decode(&p5);
    printx(p5.body, p5.length);

    return 0;
}