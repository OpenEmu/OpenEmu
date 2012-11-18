#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

template<typename... Args> inline void print(Args&&... args) {
  printf("%s", (const char*)string(std::forward<Args>(args)...));
}

}

#endif
