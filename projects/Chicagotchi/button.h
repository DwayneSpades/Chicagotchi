#include <Arduino.h>

class button {
public:
  button(int pin)
    : pin(pin) {
  }

  void init() {
    pinMode(pin, pin == 0 ? INPUT : INPUT_PULLDOWN);
  }

  void update() {
    lastState = state;
    state = digitalRead(pin);
  }
private:
  int pin;
  int state = 1;
  int lastState = 1;

public:
  bool down() const {
    return state != LOW && lastState == LOW;
  }

  bool held() const {
    return state != LOW;
  }

  bool up() const {
    return state == LOW && lastState != LOW;
  }
};
