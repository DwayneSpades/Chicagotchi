#include <Arduino.h>
#include <lapi.h>

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

#define BUTTON_COUNT 3
button buttons[BUTTON_COUNT] {
  button{0},
  button{1},
  button{2},
};

// named for convenience
button& D0 = buttons[0];
button& D1 = buttons[1];
button& D2 = buttons[2];

void updateButtons() {
  D0.update();
  D1.update();
  D2.update();
}

bool checkButtonIndex(uint8_t index) {
  if (index >= BUTTON_COUNT) {
    Serial.print("Error! Expected a button index value between 0, and ");
    Serial.print(BUTTON_COUNT - 1);
    Serial.print("(inclusive)");
    Serial.print(". got");
    Serial.print(index);
    Serial.println(".");

    return false;
  }

  return true;
}

int lua_buttonDown(lua_State* L) {
  uint8_t index = (uint8_t)lua_tointeger(L, 1);
  if (checkButtonIndex(index)) {
    lua_pushboolean(L, buttons[index].down());
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

int lua_buttonUp(lua_State* L) {
  uint8_t index = (uint8_t)lua_tointeger(L, 1);
  if (checkButtonIndex(index)) {
    lua_pushnumber(L, buttons[index].up());
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

int lua_buttonHeld(lua_State* L) {
  uint8_t index = (uint8_t)lua_tointeger(L, 1);
  if (checkButtonIndex(index)) {
    lua_pushnumber(L, buttons[index].held());
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

int lua_buttonUnheld(lua_State* L) {
  uint8_t index = (uint8_t)lua_tointeger(L, 1);
  if (checkButtonIndex(index)) {
    lua_pushnumber(L, !buttons[index].held());
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}
