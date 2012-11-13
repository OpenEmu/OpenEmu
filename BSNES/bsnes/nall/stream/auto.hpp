#ifndef NALL_STREAM_AUTO_HPP
#define NALL_STREAM_AUTO_HPP

namespace nall {

#define autostream(...) (*makestream(__VA_ARGS__))

inline std::unique_ptr<stream> makestream(const string &path) {
  if(path.ibeginswith("http://")) return std::unique_ptr<stream>(new httpstream(path, 80));
  if(path.iendswith(".gz")) return std::unique_ptr<stream>(new gzipstream(filestream{path}));
  if(path.iendswith(".zip")) return std::unique_ptr<stream>(new zipstream(filestream{path}));
  return std::unique_ptr<stream>(new mmapstream(path));
}

inline std::unique_ptr<stream> makestream(uint8_t *data, unsigned size) {
  return std::unique_ptr<stream>(new memorystream(data, size));
}

inline std::unique_ptr<stream> makestream(const uint8_t *data, unsigned size) {
  return std::unique_ptr<stream>(new memorystream(data, size));
}

}

#endif
