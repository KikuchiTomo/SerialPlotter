#include "packet_codable.hpp"
#include "packet_definition.hpp"

#define ARDUINO_R3
#define PC_HOST_ADDR (0xA1)
#define ME_ADDR (0xB1)
#define SEND_DATA_PINS_NUM (3)
int SEND_DATA_ANALOG_PINS[SEND_DATA_PINS_NUM] = { A0, A1, A2 };

// --------------------------------------------------------
unsigned char *buf;
uint32_t sq_ = 0;
SSerial::Codable::Packet *pack;
SSerial::Codable::COBSCoder* coder;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  buf = (unsigned char*) malloc(256);
  coder = new SSerial::Codable::COBSCoder();
  pack = (SSerial::Codable::Packet*) malloc(sizeof(SSerial::Codable::Packet) * 1);  
  pack->body = buf;

  #ifdef RAS_PICO 
    for(int i=0; i<SEND_DATA_PINS_NUM; i++){
      analogReadResolution(SEND_DATA_ANALOG_PINS[i]);      
    }
  #endif
  #ifdef ARDUINO_R3
    for(int i=0; i<SEND_DATA_PINS_NUM; i++){
      pinMode(SEND_DATA_ANALOG_PINS[i], INPUT);      
    }
  #endif
}

SSerial::Definition::Packet send(uint32_t seq, uint32_t page, uint32_t pos, uint32_t ch, uint32_t data){
  SSerial::Definition::Packet packed = SSerial::Definition::createPacket(
      PC_HOST_ADDR, 
      ME_ADDR, 
      seq, 
      page,
      pos, 
      millis(), 
      ch, 
      data);
  
  for(int i=0; i<(int)27; i++){
    buf[i] = packed.bin[i];
  }

  pack->length = (unsigned char)27;
  coder->encode(pack);

  for(int i=0; i<pack->length; i++){
    Serial.write(pack->body[i]);
  }  
}

void loop() {
  sq_ += 1;
  for(int i=0; i<SEND_DATA_PINS_NUM; i++){
    int val = analogRead(SEND_DATA_ANALOG_PINS[i]);    
    send(sq_, SEND_DATA_PINS_NUM, i+1, i+1, (uint32_t) i);
  }
}
