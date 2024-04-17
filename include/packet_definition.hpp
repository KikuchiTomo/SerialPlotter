#ifndef S_PACKET_DEFINITION_HPP__
#define S_PACKET_DEFINITION_HPP__

#define S_PREFIX (0xA5)

namespace Serial::Definition {
union Packet {
    struct {
        unsigned char pfx;
        unsigned char dst;
        unsigned char src;
        unsigned long int seq;
        unsigned long int pag;
        unsigned long int pos;
        unsigned long int tm;
        long int ch;
        long int val;
    };

    unsigned char bin[27];
};

Packet createPacket(unsigned char dst, unsigned char src,
                    unsigned long int seq, unsigned long int page,
                    unsigned long int pos, unsigned long int time,
                    long int ch, long int val) {
    Packet packed;
    packed.pfx = S_PREFIX;
    packed.dst = dst;
    packed.src = src;
    packed.seq = seq;
    packed.pag = page;
    packed.pos = pos;
    packed.tm = time;
    packed.ch = ch;
    packed.val = val;

    return packed;
}
}; // namespace Serial::Definition

#endif