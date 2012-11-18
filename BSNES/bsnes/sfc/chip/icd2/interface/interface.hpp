void lcdScanline();
void joypWrite(bool p15, bool p14);

uint32_t videoColor(unsigned source, uint16_t red, uint16_t green, uint16_t blue);
void videoRefresh(const uint32_t *data, unsigned pitch, unsigned width, unsigned height);
void audioSample(int16_t lsample, int16_t rsample);
int16_t inputPoll(unsigned port, unsigned device, unsigned id);

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
