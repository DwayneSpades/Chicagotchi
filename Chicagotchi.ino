#include <SPIFFS.h>
#include "FS.h"
#include <LittleFS.h>
#include <lapi.h>
#include <lauxlib.h>
#include <lcode.h>
#include <ldebug.h>
#include <ldo.h>
#include <lfunc.h>
#include <lgc.h>
#include <llex.h>
#include <llimits.h>
#include <lmem.h>
#include <lobject.h>
#include <lopcodes.h>
#include <lparser.h>
#include <lstate.h>
#include <lstring.h>
#include <ltable.h>
#include <ltm.h>
#include <lua.h>
#include <luaconf.h>
#include <lualib.h>
#include <lundump.h>
#include <lvm.h>
#include <lzio.h>

#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ST7796S.h>
#include <Adafruit_ST77xx.h>

#include <stdio.h>

/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit ESP32-S2 TFT Feather
    ----> http://www.adafruit.com/products/5300

  Check out the links above for our tutorials and wiring diagrams.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789




// Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;
GFXcanvas16 canvas(240, 135);
using namespace std;

int lua_drawCircle(lua_State* L)
{
	int16_t x = (int16_t)lua_tonumber(L, 1);
  int16_t y = (int16_t)lua_tonumber(L, 2);
  int16_t r = (int16_t)lua_tonumber(L, 3);
  int16_t num = stoi("0xffff00", nullptr, 16);

  //num = (floor(num * (16 - 1) + 0.5)) / (24 - 1);
	//create the drawable and push into the map
  canvas.drawCircle(x, y, r, 0xFB94); //.drawCircle(x, y, r, ST77XX_WHITE);
	//return the values reutrned in this stack
  return 1;
}

int lua_convertHex(lua_State* L)
{
  const char* stuff = lua_tostring(L, 1);
  int16_t num = stoi(stuff, nullptr, 16);
  //tft.println(stuff);
  //tft.println(num);
  lua_pushnumber(L,num);
  return 1;
}

int lua_drawPixel(lua_State* L)
{
	int16_t x = (int16_t)lua_tonumber(L, 1);
  int16_t y = (int16_t)lua_tonumber(L, 2);
  const char* stuff = lua_tostring(L, 3);
  unsigned int num = stoi(stuff, nullptr, 16);

	//create the drawable and push into the map
  tft.drawPixel(x, y, num); //.drawCircle(x, y, r, ST77XX_WHITE);
	//return the values reutrned in this stack
  return 1;
}

int lua_println(lua_State* L)
{
	const char* stuff = lua_tostring(L, 1);

	//create the drawable and push into the map
  tft.println(stuff);//.drawCircle(x, y, r, ST77XX_WHITE);
	//return the values reutrned in this stack
  return 1;
}


int lua_setTextColor(lua_State* L)
{
	const char* stuff = lua_tostring(L, 1);
  int16_t num = stoi(stuff, nullptr, 16);
  //num = (floor(num * (16 - 1) + 0.5)) / (24 - 1);

	//create the drawable and push into the map
  tft.setTextColor(num);//.drawCircle(x, y, r, ST77XX_WHITE);
	//return the values reutrned in this stack
  return 1;
}

int lua_print(lua_State* L)
{
	const char* stuff = lua_tostring(L, 1);
	//create the drawable and push into the map
  tft.print(stuff);//.drawCircle(x, y, r, ST77XX_WHITE);
	//return the values reutrned in this stack
  return 1;
}

int lua_require(lua_State* L)
{
  const char* fileName = lua_tostring(L, 1);
  tft.println("attempting to load script from funciton");
  //delay(2000);

  const char* fs = "/littlefs/";
  char newName[300]; 
  strcpy(newName,fs);
  strcat(newName,fileName);

  tft.println(newName);
  //delay(2000);

  if (luaL_dofile(L,newName) != LUA_OK) {
    tft.println("Lua error: ");
    tft.println(lua_tostring(L, -1));
    delay(2000);

    lua_pop(L, 1);
    lua_close(L);
    return -1;
  }
  return 1;
}


//SET UP LUA State AS A GLOBAL
lua_State* L = luaL_newstate();

void runScript(const char* fileName)
{

  tft.println("attempting to load script from funciton");
  //delay(2000);

  const char* fs = "/littlefs/";
  char newName[300]; 
  strcpy(newName,fs);
  strcat(newName,fileName);

  tft.println(newName);
  //delay(2000);

  if (luaL_dofile(L,newName) != LUA_OK) {
    tft.println("Lua error: ");
    tft.println(lua_tostring(L, -1));
    delay(2000);

    lua_pop(L, 1);
    lua_close(L);
    return;
  }
}


void setup(void) {
  Serial.begin(115200);
  Serial.print(F("Hello! Feather TFT Test"));

  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  LittleFS.begin(false,"/littlefs",5,"spiffs");

  //tft.println("File Content:");
  //delay(2000);

  // large block of text
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);

  luaL_openlibs(L);
  lua_register(L, "drawCircle", lua_drawCircle);
  lua_register(L, "drawPixel", lua_drawPixel);
  lua_register(L, "convertHex", lua_convertHex);
  lua_register(L, "myrtleRequire", lua_require);
  lua_register(L, "myrtlePrintln", lua_println);
  lua_register(L, "myrtlePrint", lua_print);
  lua_register(L, "myrtleSetTextColor", lua_setTextColor);

  tft.println("Loaded Lua functions successfully");
  //delay(2000);

  //luaL_dostring(L,"package.path = package.path .. ';./?.lua;/littlefs/?.lua'");
  runScript("data.lua");
  tft.println("Loaded Lua scripts successfully");
  delay(2000);

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);

  lua_getglobal(L, "myrtle_load");
  if (lua_isfunction(L, -1))
  {
    //tft.println("ran myrtle load successfully");
    lua_pcall(L, 0, 0, 0);
  }
  
  
  delay(16000);

  tft.println("ran Game Load successfully");
  //tft.drawCircle(32, 32, 32, ST77XX_BLUE);
  
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  
}

void loop() {
  //canvas lets the draw to screen be not have flicker
  canvas.fillScreen(0);
  canvas.setCursor(0, 0);
  
  

  lua_getglobal(L, "myrtle_update");
  if (lua_isfunction(L, -1))
  {
    lua_pcall(L, 0, 0, 0);
  }

  lua_getglobal(L, "myrtle_draw");
  if (lua_isfunction(L, -1))
  {
    lua_pcall(L, 0, 0, 0);
  }
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
}
