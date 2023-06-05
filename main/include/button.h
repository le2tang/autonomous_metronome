#pragma once

#ifdef __cplusplus
extern "C" {
#endif

class Button {
  public:
    Button() {}
    ~Button() {}

    void init(int pin);
    bool get_status();

  private:
    int pin_;
};

#ifdef __cplusplus
}
#endif
