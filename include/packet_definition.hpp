#ifndef S_PACKET_DEFINITION_HPP__
#define S_PACKET_DEFINITION_HPP__

#define S_PREFIX (0xA5)

namespace Serial::Definition {
typedef struct basePack{
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

typedef struct extPack{
    BasePacket packet;
    uint8_t bin[27];

    void encodePacket() {
        int i = 0;
        bin[i++] = packet.pfx;
        bin[i++] = packet.dst;
        bin[i++] = packet.src;
        bin[i++] = (packet.seq >> 24);
        bin[i++] = (packet.seq >> 16);
        bin[i++] = (packet.seq >> 8);
        bin[i++] = (packet.seq >> 0);
        bin[i++] = (packet.pag >> 24);
        bin[i++] = (packet.pag >> 16);
        bin[i++] = (packet.pag >> 8);
        bin[i++] = (packet.pag >> 0);
        bin[i++] = (packet.pos >> 24);
        bin[i++] = (packet.pos >> 16);
        bin[i++] = (packet.pos >> 8);
        bin[i++] = (packet.pos >> 0);
        bin[i++] = (packet.tm >> 24);
        bin[i++] = (packet.tm >> 16);
        bin[i++] = (packet.tm >> 8);
        bin[i++] = (packet.tm >> 0);
        bin[i++] = (packet.ch >> 24);
        bin[i++] = (packet.ch >> 16);
        bin[i++] = (packet.ch >> 8);
        bin[i++] = (packet.ch >> 0);
        bin[i++] = (packet.val >> 24);
        bin[i++] = (packet.val >> 16);
        bin[i++] = (packet.val >> 8);
        bin[i++] = (packet.val >> 0);
    }

    inline uint32_t convU32(const uint8_t *b, int *i) { 
        uint32_t res = 0;
        res |= (b[(*i)++] << 24) & 0xFF000000;
        res |= (b[(*i)++] << 16) & 0x00FF0000;
        res |= (b[(*i)++] << 8) & 0x0000FF00;
        res |= (b[(*i)++] << 0) & 0x000000FF;        
        return res;
    }

    inline int32_t convS32(const uint8_t *b, int *i) { 
        int32_t res = 0;
        res |= (b[(*i)++] << 24) & 0xFF000000;
        res |= (b[(*i)++] << 16) & 0x00FF0000;
        res |= (b[(*i)++] << 8) & 0x0000FF00;
        res |= (b[(*i)++] << 0) & 0x000000FF;        
        return res;
    }

    void decodePacket() {
        int i = 0;
        packet.pfx = bin[i++];
        packet.dst = bin[i++];
        packet.src = bin[i++];
        packet.seq = convU32(bin, &i);
        packet.pag = convU32(bin, &i);
        packet.pos = convU32(bin, &i);
        packet.tm = convU32(bin, &i);
        packet.ch = convU32(bin, &i);
        packet.val = convU32(bin, &i);
    }
} Packet;

Packet createPacket(uint8_t dst, uint8_t src,
                    uint32_t seq, uint32_t page,
                    uint32_t pos, uint32_t time,
                    uint32_t ch, int32_t val) {
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

    packed.encodePacket();
    return packed;
}
}; // namespace Serial::Definition

#endif