#pragma once

#include <esp_system.h>

#ifdef __cplusplus
extern "C" {
#endif

class MCP3002 {
  public:
    MCP3002() {}
    ~MCP3002() {}

    void init();

    uint16_t read();
};

#ifdef __cplusplus
}
#endif
