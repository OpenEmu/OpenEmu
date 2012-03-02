#ifndef NALL_SERIAL_HPP
#define NALL_SERIAL_HPP

#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <nall/stdint.hpp>

namespace nall {
  class serial {
  public:
    //-1 on error, otherwise return bytes read
    int read(uint8_t *data, unsigned length) {
      if(port_open == false) return -1;
      return ::read(port, (void*)data, length);
    }

    //-1 on error, otherwise return bytes written
    int write(const uint8_t *data, unsigned length) {
      if(port_open == false) return -1;
      return ::write(port, (void*)data, length);
    }

    bool open(const char *portname, unsigned rate, bool flowcontrol) {
      close();

      port = ::open(portname, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
      if(port == -1) return false;

      if(ioctl(port, TIOCEXCL) == -1) { close(); return false; }
      if(fcntl(port, F_SETFL, 0) == -1) { close(); return false; }
      if(tcgetattr(port, &original_attr) == -1) { close(); return false; }

      termios attr = original_attr;
      cfmakeraw(&attr);
      cfsetspeed(&attr, rate);

      attr.c_lflag &=~ (ECHO | ECHONL | ISIG | ICANON | IEXTEN);
      attr.c_iflag &=~ (BRKINT | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
      attr.c_iflag |=  (IGNBRK | IGNPAR);
      attr.c_oflag &=~ (OPOST);
      attr.c_cflag &=~ (CSIZE | CSTOPB | PARENB | CLOCAL);
      attr.c_cflag |=  (CS8 | CREAD);
      if(flowcontrol == false) {
        attr.c_cflag &= ~CRTSCTS;
      } else {
        attr.c_cflag |=  CRTSCTS;
      }
      attr.c_cc[VTIME] = attr.c_cc[VMIN] = 0;

      if(tcsetattr(port, TCSANOW, &attr) == -1) { close(); return false; }
      return port_open = true;
    }

    void close() {
      if(port != -1) {
        tcdrain(port);
        if(port_open == true) {
          tcsetattr(port, TCSANOW, &original_attr);
          port_open = false;
        }
        ::close(port);
        port = -1;
      }
    }

    serial() {
      port = -1;
      port_open = false;
    }

    ~serial() {
      close();
    }

  private:
    int port;
    bool port_open;
    termios original_attr;
  };
}

#endif
