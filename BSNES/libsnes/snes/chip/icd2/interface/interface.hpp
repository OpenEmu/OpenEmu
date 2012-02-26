void lcdScanline();
void joypWrite(bool p15, bool p14);
void videoRefresh(const uint16_t *data);
void audioSample(int16_t center, int16_t left, int16_t right);
bool inputPoll(unsigned id);

struct Packet {
  uint8 data[16];
  uint8& operator[](unsigned addr) { return data[addr & 15]; }
};
Packet packet[64];
unsigned packetsize;

unsigned joyp_id;
bool joyp15lock;
bool joyp14lock;
bool pulselock;
bool strobelock;
bool packetlock;
Packet joyp_packet;
uint8 packetoffset;
uint8 bitdata, bitoffset;
