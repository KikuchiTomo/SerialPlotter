#ifndef S_PACKET_DEFINITION_HPP__
#define S_PACKET_DEFINITION_HPP__

#define S_PREFIX (0x11)

namespace SSerial::Definition {
  typedef struct {
        uint8_t pfx;
        uint8_t dst;
        uint8_t src;
        uint32_t seq;
        uint32_t pag;
        uint32_t pos;
        uint32_t tm;
        uint32_t ch;
        uint32_t val;
  } BasePacket;
typedef struct{
    BasePacket packet;
    uint8_t bin[27];

    void createBinary(){
        int i=0;
        bin[i++] = packet.pfx;
        bin[i++] = packet.dst;
        bin[i++] = packet.src;
        bin[i++] = (packet.seq >> 24);
        bin[i++] = (packet.seq >> 16);
        bin[i++] = (packet.seq >> 8 );
        bin[i++] = (packet.seq >> 0 );
        bin[i++] = (packet.pag >> 24);
        bin[i++] = (packet.pag >> 16);
        bin[i++] = (packet.pag >> 8 );
        bin[i++] = (packet.pag >> 0 );
        bin[i++] = (packet.pos >> 24);
        bin[i++] = (packet.pos >> 16);
        bin[i++] = (packet.pos >> 8 );
        bin[i++] = (packet.pos >> 0 );
        bin[i++] = (packet.tm >> 24);
        bin[i++] = (packet.tm >> 16);
        bin[i++] = (packet.tm >> 8 );
        bin[i++] = (packet.tm >> 0 );
        bin[i++] = (packet.ch >> 24);
        bin[i++] = (packet.ch >> 16);
        bin[i++] = (packet.ch >> 8 );
        bin[i++] = (packet.ch >> 0 );
        bin[i++] = (packet.val >> 24);
        bin[i++] = (packet.val >> 16);
        bin[i++] = (packet.val >> 8 );
        bin[i++] = (packet.val >> 0 );
    }
} Packet;

Packet createPacket(uint8_t dst, uint8_t src,
                    uint32_t seq, uint32_t page,
                    uint32_t pos, uint32_t time,
                    uint32_t ch, uint32_t val) {
    Packet packed;
    packed.packet.pfx = S_PREFIX;
    packed.packet.dst = dst;
    packed.packet.src = src;
    packed.packet.seq = seq;
    packed.packet.pag = page;
    packed.packet.pos = pos;
    packed.packet.tm  = time;
    packed.packet.ch  = ch;
    packed.packet.val = val;

    packed.createBinary();
    return packed;
}
}; // namespace Serial::Definition

#endif