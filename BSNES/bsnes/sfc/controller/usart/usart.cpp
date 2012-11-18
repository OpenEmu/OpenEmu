#ifdef CONTROLLER_CPP

//Synchronous serial communications cable emulation

//Hardware:
//Teensy++ 2.0 USB
//AT90USB1286

//Connection Diagram:
//SNES GND <> Teensy GND
//SNES IOBit <> Teensy B0
//SNES Latch <> Teensy D2
//SNES Data1 <> Teensy D3
//SNES Clock <> 1Kohm Resistor <> Teensy D5
//Teensy D5 <> Teensy D7

void USART::enter() {
  if(init && main) {
    init(
      {&USART::quit, this},
      {&USART::usleep, this},
      {&USART::readable, this},
      {&USART::read, this},
      {&USART::writable, this},
      {&USART::write, this}
    );
    main();
  }
  while(true) step(10000000);
}

bool USART::quit() {
  step(1);
  return false;
}

void USART::usleep(unsigned milliseconds) {
  step(10 * milliseconds);
}

bool USART::readable() {
  step(1);
  return txbuffer.size();
}

//SNES -> USART
uint8 USART::read() {
  step(1);
  while(txbuffer.size() == 0) step(1);
  uint8 data = txbuffer[0];
  txbuffer.remove(0);
  return data;
}

bool USART::writable() {
  step(1);
  return true;
}

//USART -> SNES
void USART::write(uint8 data) {
  step(1);
  rxbuffer.append(data ^ 0xff);
}

//clock
uint2 USART::data() {
  //Joypad
  if(iobit()) {
    if(counter >= 16) return 1;
    uint2 result = 0;
    if(counter < 12) result = interface->inputPoll(port, (unsigned)Input::Device::Joypad, counter);
    if(latched == 0) counter++;
    return result;
  }

  //SNES -> USART
  if(txlength == 0) {
    if(latched == 0) txlength++;
  } else if(txlength <= 8) {
    txdata = (latched << 7) | (txdata >> 1);
    txlength++;
  } else {
    if(latched == 1) txbuffer.append(txdata);
    txlength = 0;
  }

  //USART -> SNES
  if(rxlength == 0 && rxbuffer.size()) {
    data1 = 1;
    rxdata = rxbuffer[0];
    rxbuffer.remove(0);
    rxlength++;
  } else if(rxlength <= 8) {
    data1 = rxdata & 1;
    rxdata >>= 1;
    rxlength++;
  } else {
    data1 = 0;
    rxlength = 0;
  }

  return (data2 << 1) | (data1 << 0);
}

//latch
void USART::latch(bool data) {
  if(latched == data) return;
  latched = data;
  counter = 0;
}

USART::USART(bool port) : Controller(port) {
  latched = 0;
  data1 = 0;
  data2 = 0;
  counter = 0;

  rxlength = 0;
  rxdata = 0;

  txlength = 0;
  txdata = 0;

  string filename = {interface->path(0), "usart.so"};
  if(open_absolute(filename)) {
    init = sym("usart_init");
    main = sym("usart_main");
    if(init && main) create(Controller::Enter, 10000000);
  }
}

USART::~USART() {
  if(opened()) close();
}

#endif
