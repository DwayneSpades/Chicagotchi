#include <cstdint>

struct run {
  run() = default;
  ~run() = default;

public:
  int x;
  int y;
  int w; // also a pixel count

  uint16_t pixels[64];
};
