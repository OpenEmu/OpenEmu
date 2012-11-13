#ifndef EMULATOR_INTERFACE_HPP
#define EMULATOR_INTERFACE_HPP

namespace Emulator {

struct Interface {
  struct Information {
    string name;
    unsigned width;
    unsigned height;
    bool overscan;
    double aspectRatio;
    bool resettable;
  } information;

  struct Media {
    unsigned id;
    string name;
    string type;
    string path;
    string extension;
  };

  vector<Media> firmware;
  vector<Media> media;

  struct Memory {
    unsigned id;
    string name;
  };
  vector<Memory> memory;

  struct Device {
    unsigned id;
    unsigned portmask;
    string name;
    struct Input {
      unsigned id;
      unsigned type;  //0 = digital, 1 = analog
      string name;
      unsigned guid;
    };
    vector<Input> input;
    vector<unsigned> order;
  };

  struct Port {
    unsigned id;
    string name;
    vector<Device> device;
  };
  vector<Port> port;

  struct Bind {
    virtual void loadRequest(unsigned, const string&) {}
    virtual void loadRequest(unsigned, const string&, const string&, const string&) {}
    virtual uint32_t videoColor(unsigned, uint16_t, uint16_t, uint16_t) { return 0u; }
    virtual void videoRefresh(const uint32_t*, unsigned, unsigned, unsigned) {}
    virtual void audioSample(int16_t, int16_t) {}
    virtual int16_t inputPoll(unsigned, unsigned, unsigned) { return 0; }
    virtual unsigned dipSettings(const XML::Node&) { return 0; }
    virtual string path(unsigned) { return ""; }
  } *bind;

  //callback bindings (provided by user interface)
  void loadRequest(unsigned id, const string &path) { return bind->loadRequest(id, path); }
  void loadRequest(unsigned id, const string &name, const string &type, const string &path) { return bind->loadRequest(id, name, type, path); }
  uint32_t videoColor(unsigned source, uint16_t red, uint16_t green, uint16_t blue) { return bind->videoColor(source, red, green, blue); }
  void videoRefresh(const uint32_t *data, unsigned pitch, unsigned width, unsigned height) { return bind->videoRefresh(data, pitch, width, height); }
  void audioSample(int16_t lsample, int16_t rsample) { return bind->audioSample(lsample, rsample); }
  int16_t inputPoll(unsigned port, unsigned device, unsigned input) { return bind->inputPoll(port, device, input); }
  unsigned dipSettings(const XML::Node &node) { return bind->dipSettings(node); }
  string path(unsigned group) { return bind->path(group); }

  //information
  virtual double videoFrequency() = 0;
  virtual double audioFrequency() = 0;

  //media interface
  virtual bool loaded() { return false; }
  virtual string sha256() { return ""; }
  virtual unsigned group(unsigned id) { return 0u; }
  virtual void load(unsigned id, const stream &memory, const string &markup = "") {}
  virtual void save(unsigned id, const stream &memory) {}
  virtual void unload() {}

  //system interface
  virtual void connect(unsigned port, unsigned device) {}
  virtual void power() {}
  virtual void reset() {}
  virtual void run() {}

  //state functions
  virtual serializer serialize() = 0;
  virtual bool unserialize(serializer&) = 0;

  //cheat functions
  virtual void cheatSet(const lstring& = lstring{}) {}

  //utility functions
  virtual void updatePalette() {}

  Interface() : bind(nullptr) {}
};

}

#endif
