#include <Arduino.h>

class button {
public:
  // D0 is pullup - active LOW
  // D1/D2 are pulldown - active HIGH
  button(int pin)
    : pin(pin) {
    pinMode(pin, pin == 0 ? INPUT_PULLUP : INPUT_PULLDOWN);
    onState = pin == 0 ? LOW : HIGH;
    offState = pin == 0 ? HIGH : LOW;
  }

  void update() {
    lastState = state;
    state = digitalRead(pin);
  }
private:
  int pin;
  int state = 1;
  int lastState = 1;

  int onState;
  int offState;

public:
  bool down() const {
    return state == onState && lastState == offState;
  }

  bool held() const {
    return state == onState;
  }

  bool up() const {
    return state == offState && lastState == onState;
  }
};
