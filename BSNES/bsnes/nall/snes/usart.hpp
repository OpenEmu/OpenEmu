#ifndef NALL_SNES_USART_HPP
#define NALL_SNES_USART_HPP

#include <nall/platform.hpp>
#include <nall/function.hpp>
#include <nall/serial.hpp>
#include <nall/stdint.hpp>

#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#define usartproc dllexport

static nall::function<bool ()> usart_quit;
static nall::function<void (unsigned milliseconds)> usart_usleep;
static nall::function<bool ()> usart_readable;
static nall::function<uint8_t ()> usart_read;
static nall::function<bool ()> usart_writable;
static nall::function<void (uint8_t data)> usart_write;

extern "C" usartproc void usart_init(
  nall::function<bool ()> quit,
  nall::function<void (unsigned milliseconds)> usleep,
  nall::function<bool ()> readable,
  nall::function<uint8_t ()> read,
  nall::function<bool ()> writable,
  nall::function<void (uint8_t data)> write
) {
  usart_quit = quit;
  usart_usleep = usleep;
  usart_readable = readable;
  usart_read = read;
  usart_writable = writable;
  usart_write = write;
}

extern "C" usartproc void usart_main();

//

static nall::serial usart;
static bool usart_is_virtual = true;
static bool usart_sigint = false;

static bool usart_virtual() {
  return usart_is_virtual;
}

//

static bool usarthw_quit() {
  return usart_sigint;
}

static void usarthw_usleep(unsigned milliseconds) {
  usleep(milliseconds);
}

static bool usarthw_readable() {
  return usart.readable();
}

static uint8_t usarthw_read() {
  while(true) {
    uint8_t buffer[1];
    signed length = usart.read((uint8_t*)&buffer, 1);
    if(length > 0) return buffer[0];
  }
}

static bool usarthw_writable() {
  return usart.writable();
}

static void usarthw_write(uint8_t data) {
  uint8_t buffer[1] = { data };
  usart.write((uint8_t*)&buffer, 1);
}

static void sigint(int) {
  signal(SIGINT, SIG_DFL);
  usart_sigint = true;
}

int main(int argc, char **argv) {
  //requires superuser privileges; otherwise priority = +0
  setpriority(PRIO_PROCESS, 0, -20);
  signal(SIGINT, sigint);

  bool result = false;
  if(argc == 1) result = usart.open("/dev/ttyACM0", 57600, true);
  if(argc == 2) result = usart.open(argv[1], 57600, true);
  if(result == false) {
    printf("error: unable to open USART hardware device\n");
    return 0;
  }
  usart_is_virtual = false;
  usart_init(usarthw_quit, usarthw_usleep, usarthw_readable, usarthw_read, usarthw_writable, usarthw_write);
  usart_main();
  usart.close();
  return 0;
}

#endif
