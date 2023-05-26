#pragma once

#include <stdio.h>

class MCP3002 {
  public:
    MCP3002();
    ~MCP3002() {}

    uint16_t read();
};
