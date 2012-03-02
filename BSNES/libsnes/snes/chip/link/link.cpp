#include <snes/snes.hpp>

#define LINK_HPP
namespace SNES {

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
  string basename = interface->path(Cartridge::Slot::Base, "");
  string name = program != "" ? program : notdir(basename);
  string path = dir(basename);
  if(open(name, path)) {
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
