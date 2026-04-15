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

#include <Adafruit_ST7789.h>
#include <esp_system.h>

#include <stdio.h>
#include <stddef.h>

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


//[Myrtle framework's Design and Notes:

//[Framework Design]
//C++ side is for providing access to gfx drawing funcitons from lua scripts with wrapper functions. lua side is for programming the whole game. 
//I highly reccommend not putting any game code on the c++ side at all. 
//The reaseon for that is to minimize needing to transfer and access  data from lua to c++ via the lua stack with more complex data structures you would have to track through the stack. 
//IMO it's unneccessary casue the whole game's code can be written in lua - Slaps

//[Notes on Optimization and Lua Memory use with the Garbage Collector]
//use C++ to optimize wrapper functions to call from the lua scripts. like boosting math fuctions with SIMD and giving back the values to the scripts faster than lua can.
//^doing math in c++ would be a good idea to minimize memory use because lua generates garbage with every equation and object made. 
//This could overburden the garbage collector and cause freezes on the main thread. this is an extreme case tho.
//I don't think that will be an issue here unless we were doing something with tons of computations each frame 

//[MAKING WRAPPER FUNCTIONS for LUA]
//register a lua wrapper function to the lua vm with: ex. -> lua_register(L, "drawCircle", lua_drawCircle); 
//1st input is the lua vm instance. 
//2nd input is the name the lua function will be called by in the scripts. 
//3rd input is the name of the wrapper function in c++
//*CAUTION* always return an int in your wrapper function. its for the lua vm not the lua script. if you want to return a value to the lua script use: lua_pushnumber(L,returnValue);

//~ Slaps

#include "luaState.h"
#include "button.h"
#include "networkManager.h"

// Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
uint32_t engineTime = 0;
uint32_t previousTime = 0;
uint32_t deltaTime = 0;

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

//example of a lua wrapper function that returns a value to the lua script. 
//if you want to compute something in c++ and then give it back to lua script game code do this-> lua_pushnumber(L,num);
//you can return multiple values too if you repeat the push commands just recieve the numbers in lua with something like: local x,y = myrtleWrapperFunction(input);
int lua_convertHex(lua_State* L)
{
  const char* stuff = lua_tostring(L, 1);
  //int bitSize = lua_tostring(L, 1);
  int16_t num = stoi(stuff, nullptr, 16);
  //tft.println(stuff);
  //tft.println(num);
  lua_pushnumber(L,num);
  return 1;
}

int lua_sendBitData(lua_State* L)
{
	//arecieves: x,y,"24bit color string"
  //going to have to store the 
	//return the values reutrned in this stack
  return 1;
}

//store only the visible pixel island on a strip when storing sprite data
//store the line of pixels
//the length of the island
//the x,y start of the line segment
class pixelIsland {
  //can be up to 64 spaces long but doesn't have to be...
  //store len
public:
  pixelIsland() = default;
	pixelIsland(int x, int y);
	pixelIsland operator = (const pixelIsland &ptr);
	~pixelIsland()=default;

  alignas(16) uint16_t buffer [64];
  int len = 0 ;
  uint16_t x = 0;
  uint16_t y = 0;
  
};

//I'm thining we can store these drawn pixels once to some canvas buffer and just display that
class sprite 
{
public:
	sprite() = default;
	sprite(int w, int h, int len);
	sprite operator = (const sprite &ptr);
	~sprite()=default;

  uint16_t *pixels;
  pixelIsland *pixelIslands;
  
  int width = 0;
	int height = 0;
  int numIslands = 0;

  int16_t x = 0;
	int16_t y = 0;

  //16bit color code
	
  //store a list of the visible line islands instead so they can be memcpy'd straight up in a best case scenario
  
private:
};

sprite::sprite(int _w, int _h, int len)
{
  width = _w;
  height = _h;

  pixels = new uint16_t[len];
  pixelIslands = new pixelIsland[500];
}

//make an array of pixels to store in the sprite Storage
//store rows of pixels instead
//use memcopy ot copy the color codes in chunks
//less chunks to loop through by rows....


//unordered_map<string,sprite*> sprites;
unordered_map<string,sprite*> sprites;

int lua_createSprite(lua_State* L)
{
  const char* name = lua_tostring(L, 1);
  int len = (int)lua_tonumber(L, 2);
  int w = (int)lua_tonumber(L, 3);
  int h = (int)lua_tonumber(L, 4);

  sprites[name] = new sprite(w,h,len);
  
  return 1;
}

//load one pixel at a time....
//organize the pixels into line segment arrays...
//count the pixels Islands by their visible pixels
//if transparent pixel found then do not create or add a pixel island object to the pixel island list.
bool createIsland = true;
int xIterator = 0;
int yIterator = 0;
int lineIndex = 0;
int islandIndex = -1;

pixelIsland *currentIsland = new pixelIsland();

int lua_loadPixel(lua_State* L)
{
  const char* name = lua_tostring(L, 1);
  int index = (int)lua_tonumber(L, 2);
  //replace this with a width call.
  //simply count each row pixel by pixel by incriment the y iterator once the x Iterator reaches 63 and reset to zero
  int width = 64;

  const char* stuff = lua_tostring(L, 3);
  uint16_t color = (uint16_t)stoi(stuff, nullptr, 16);

	//create the drawable and push into the map
  sprites[name]->pixels[index] = color;
  
  xIterator += 1;
  if (xIterator > width-1)
  {
    xIterator = 0;
    yIterator += 1;
  }

  //if color is not transparent then create a 
  if (color != 0xea60)
  {
    //create island
    if(createIsland == true)
    {
      islandIndex += 1;
      createIsland = false;
      lineIndex = 0;
      
      sprites[name]->numIslands += 1;
 
      sprites[name]->pixelIslands[islandIndex].x = xIterator;
      sprites[name]->pixelIslands[islandIndex].y = yIterator;
      sprites[name]->pixelIslands[islandIndex].buffer[lineIndex] = color;
      sprites[name]->pixelIslands[islandIndex].len += 1;
      lineIndex += 1;
    }
    else
    {
      sprites[name]->pixelIslands[islandIndex].buffer[lineIndex] = color;
      sprites[name]->pixelIslands[islandIndex].len += 1;
      lineIndex += 1;
    }
  }
  else
  {
    //make a new island and reset xIterator
    createIsland = true;
    //tft.println(color);
  }

   //.drawCircle(x, y, r, ST77XX_WHITE);
	//return the values reutrned in this stack
  return 1;
}

struct screenBufferStruct {
  alignas(16) uint16_t buffer [32400];
};

screenBufferStruct _screenBuffer;

void customDrawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap,
                                 int16_t w, int16_t h) {
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (bitmap[j * w + i] != 0xea60)
      {
        //custom_memcpy(canvas.getBuffer() + ( (y+j) * canvas.width() + x), sprites[name]->pixels + ((0+j) * w + 0), sizeof(uint16_t) * 1);
        canvas.writePixel(x + i,y, bitmap[j * w + i]);
      }
    }
  }
}


int lua_drawBitmap(lua_State* L)
{
  const char* name = lua_tostring(L, 1);
  int16_t x = (int16_t)lua_tonumber(L, 2);
  int16_t y = (int16_t)lua_tonumber(L, 3);
  
  //memcpy(_screenBuffer + (y * canvas.width() + x) + 1, sprites[name], i * (sizeof(unit16_t) * 65));
  customDrawRGBBitmap(x,y,sprites[name]->pixels,64,64);
  return 1;
}

void custom_memcpy(void* dest, const void* src, int len)
{
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);
    
    for (int i = 0; i < len; ++i)
    {
      //if (s[i] != 234 && s[i] != 96) 
        //d[i] = s[i];
    }

}

uint16_t *_canvasBuffer = canvas.getBuffer();
int lua_drawSprite(lua_State* L)
{
    const char* name = lua_tostring(L, 1);
    int16_t x = (int16_t)lua_tointeger(L, 2);
    int16_t y = (int16_t)lua_tointeger(L, 3);

    auto* sprite = sprites[name];
    int islandSize = sprite->numIslands;
    pixelIsland* spritePtr = sprite->pixelIslands;

    for (int i = 0; i < islandSize; i++)
    {
        int16_t yShift = y + spritePtr[i].y;
        if (yShift < 0) continue;
        if (yShift >= 135) break; // y-sorted, done

        int16_t dstX = x + spritePtr[i].x;
        int16_t copyLen = spritePtr[i].len;
        int16_t srcOffset = 0;

        if (dstX < 0) {
            srcOffset = -dstX;
            copyLen += dstX;
            dstX = 0;
        }
        if (dstX + copyLen > 240) {
            copyLen = 240 - dstX;
        }
        if (copyLen <= 0) continue;

        memcpy(_canvasBuffer + (yShift * 240 + dstX),
               spritePtr[i].buffer + srcOffset,
               sizeof(uint16_t) * copyLen);
    }
    return 1;
}


int lua_println(lua_State* L)
{
	const char* stuff = lua_tostring(L, 1);

#if DBG_SER
  Serial.print("lua_println: ");
  Serial.println(stuff);
#endif
	//create the drawable and push into the map
  canvas.println(stuff);//.drawCircle(x, y, r, ST77XX_WHITE);
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
#if DBG_SER
  Serial.print("lua_print: ");
  Serial.println(stuff);
#endif

	//create the drawable and push into the map
  tft.setTextColor(ST77XX_WHITE);
  tft.println(stuff);//.drawCircle(x, y, r, ST77XX_WHITE);
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

#define _NETWORK_ 1

void setup(void) {
  Serial.begin(115200);
  delay(500);
  Serial.println("");
  Serial.println("");

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
  lua_register(L, "createSprite", lua_createSprite);
  lua_register(L, "loadPixel", lua_loadPixel);
  lua_register(L, "drawSprite", lua_drawSprite);
  lua_register(L, "drawBitmap", lua_drawBitmap);

  lua_register(L, "buttonDown", lua_buttonDown);
  lua_register(L, "buttonUp", lua_buttonUp);
  lua_register(L, "buttonHeld", lua_buttonHeld);
  lua_register(L, "buttonUnheld", lua_buttonUnheld);

  lua_register(L, "convertHex", lua_convertHex);
  lua_register(L, "myrtleRequire", lua_require);
  lua_register(L, "myrtlePrintln", lua_println);
  lua_register(L, "myrtlePrint", lua_print);
  lua_register(L, "myrtleSetTextColor", lua_setTextColor);

  lua_register(L, "sendMessage", lua_sendMessage);
  lua_register(L, "getPeerCount", lua_getPeerCount);
  lua_register(L, "getPeerAddr", lua_getPeerAddr);

  tft.println("Loaded Lua functions successfully");
  //delay(2000);

  //luaL_dostring(L,"package.path = package.path .. ';./?.lua;/littlefs/?.lua'");
  runScript("main.lua");
  tft.println("Loaded Lua scripts successfully");
  
  esp_reset_reason_t reason = esp_reset_reason();
  const char* strReason = "unexpected value";
  tft.print("RESET REASON: ");
  Serial.print("RESET REASON: ");
  switch (reason) {
    case esp_reset_reason_t::ESP_RST_UNKNOWN:
      strReason = "UNKNOWN";
    break;

    case esp_reset_reason_t::ESP_RST_POWERON:
      strReason = "POWERON";
    break;

    case esp_reset_reason_t::ESP_RST_SW:
      strReason = "SOFTWARE";
    break;

    case esp_reset_reason_t::ESP_RST_PANIC:
      strReason = "PANIC !!!";
    break;

    case esp_reset_reason_t::ESP_RST_INT_WDT:
    case esp_reset_reason_t::ESP_RST_TASK_WDT:
    case esp_reset_reason_t::ESP_RST_WDT:
      strReason = "WATCH DOG";
    break;

    case esp_reset_reason_t::ESP_RST_DEEPSLEEP:
      strReason = "SLEEP";
    break;

    case esp_reset_reason_t::ESP_RST_BROWNOUT:
      strReason = "BROWNOUT!!!";
    break;

    case esp_reset_reason_t::ESP_RST_SDIO:
    case esp_reset_reason_t::ESP_RST_USB:
    case esp_reset_reason_t::ESP_RST_JTAG:
      strReason = "PERIPHERAL";
      break;

    case esp_reset_reason_t::ESP_RST_EFUSE:
      strReason = "EFUSE ERROR";
      break;

    case esp_reset_reason_t::ESP_RST_PWR_GLITCH:
      strReason = "POWER GLITCH";
      break;

    case esp_reset_reason_t::ESP_RST_CPU_LOCKUP:
      strReason = "CPU LOCKUP";
      break;

    default:
      tft.print("unhandled case: ");
      tft.println(reason);
      break;
  }
  tft.println(strReason);
  Serial.println(strReason);
  delay(3000);
  networkSetup();


  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);

  //load assets and game data from engine's load function in main.lua 
  lua_getglobal(L, "myrtle_load");
  if (lua_isfunction(L, -1))
  {
    //tft.println("ran myrtle load successfully");
    lua_pcall(L, 0, 0, 0);
  }
  
#if _NETWORK_
  networkSetup();
#endif
  
  //delay(4000);

  tft.println("ran Game Load successfully");
  //tft.drawCircle(32, 32, 32, ST77XX_BLUE);
  
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  engineTime = millis();
  previousTime = millis();
}

#define CHILL_MODE 0
#if CHILL_MODE
bool chillMode = false;
#endif

void loop() {
  previousTime = engineTime;

  //canvas lets the draw to screen be not have flicker
  //memset(_screenBuffer.buffer, 0, 32400*2);
  canvas.fillScreen(0);
  canvas.setCursor(0, 0);

  updateButtons();

#if CHILL_MODE
  if (D2.down()) {
    chillMode = !chillMode;
  }
  
  // feel free to remove chill mode
  if (chillMode) {
    canvas.println("chill: ");
    engineTime = millis();
    deltaTime = engineTime - previousTime;
    canvas.println(deltaTime);

    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
  } else {
#endif
#if DBG_SER
    printLuaStack(L);
    Serial.println("pre myrtle_update");
    printLuaStack(L);
#endif

    //engine loop update from main.lua
    lua_getglobal(L, "myrtle_update");
    if (lua_isfunction(L, -1))
    {
      lua_pcall_custom(L, 0, 0, 0);
    }

#if DBG_SER
    Serial.println("pre myrtle_draw");
    printLuaStack(L);
#endif

    lua_getglobal(L, "myrtle_draw");
    if (lua_isfunction(L, -1))
    {
      lua_pcall_custom(L, 0, 0, 0);
    }

#if DBG_SER
    Serial.println("post myrtle_draw");
    printLuaStack(L);
#endif

    engineTime = millis();
    deltaTime = engineTime - previousTime;

    #if _NETWORK_
      networkClear();
      networkUpdate(deltaTime);
    #endif
#if DBG_SER
    Serial.println("post network update");
    printLuaStack(L);
#endif

  /*
    canvas.println("Frame Time (ms): ");
    canvas.println(deltaTime);

    if (peerInit) {
      canvas.println("Connected!");
    }
  */
    //tft.drawRGBBitmap(0, 0, _screenBuffer.buffer, canvas.width(), canvas.height());
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());

#if DBG_SER
    printLuaStack(L);
#endif

    // we outta memory
    if (lua_gc(L, LUA_GCCOLLECT, 0) != LUA_OK) {
      Serial.println("ERROR WHILE GC.");
    }

#if DBG_SER
    printLuaStack(L);
#endif

    if (lua_gettop(L) > 0) {
      int t = lua_type(L, 1);
      if (t == LUA_TNIL) {
        Serial.println("ruh-roh");
        lua_pop(L, -1);
      }
    }

#if DBG_SER
    Serial.printf("Free Heap: %u\n", esp_get_free_heap_size());
    Serial.printf("Min Heap: %u\n", esp_get_minimum_free_heap_size());
    Serial.printf("Stack: %u\n", uxTaskGetStackHighWaterMark(NULL) * 4);
#endif
#if CHILL_MODE
  }
#endif
}
