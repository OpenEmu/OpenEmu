#include <sfc/sfc.hpp>

#define LINK_HPP
namespace SuperFamicom {

Link link;

void Link::Enter() { link.enter(); }

void Link::enter() {
  while(true) {
    cpu.synchronize_coprocessors();
    unsigned clocks = 1;
    if(link_run) clocks = link_run();
    step(clocks);
    synchronize_cpu();
  }
}

void Link::init() {
}

void Link::load() {
  if(opened()) close();
  if(open("link.so", interface->path(0))) {
    link_power = sym("link_power");
    link_reset = sym("link_reset");
    link_run   = sym("link_run"  );
    link_read  = sym("link_read" );
    link_write = sym("link_write");
  }
}

void Link::unload() {
  if(opened()) close();
}

void Link::power() {
  if(link_power) link_power();
}

void Link::reset() {
  if(link_reset) link_reset();
  create(Link::Enter, frequency);
}

uint8 Link::read(unsigned addr) {
  if(link_read) return link_read(addr);
  return cpu.regs.mdr;
}

void Link::write(unsigned addr, uint8 data) {
  if(link_write) return link_write(addr, data);
}

}
