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

//I'm thining we can store these drawn pixels once to some canvas buffer and just display that
class sprite 
{
public:
	sprite() = default;
	sprite(int w, int h, int len);
	sprite operator = (const sprite &ptr);
	~sprite()=default;

	//position
	int16_t x = 0;
	int16_t y = 0;
  int width = 0;
	int height = 0;
  
  //16bit color code
	uint16_t *pixels;

private:
};

sprite::sprite(int _w, int _h, int len)
{
  width = _w;
  height = _h;

  pixels = new uint16_t[len];
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

int lua_loadPixel(lua_State* L)
{
  const char* name = lua_tostring(L, 1);
  int index = (int)lua_tonumber(L, 2);

  const char* stuff = lua_tostring(L, 3);
  uint16_t color = (uint16_t)stoi(stuff, nullptr, 16);

	//create the drawable and push into the map
  sprites[name]->pixels[index] = color;
  
  
   //.drawCircle(x, y, r, ST77XX_WHITE);
	//return the values reutrned in this stack
  return 1;
}

uint16_t *_screenBuffer = canvas.getBuffer();
void customDrawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap,
                                 int16_t w, int16_t h) {
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (bitmap[j * w + i] != 0xea60)
      {
        canvas.writePixel(x + i,y, bitmap[j * w + i]);
      }
    }
  }
}


int lua_drawBitmap(lua_State* L)
{
  const char* name = lua_tostring(L, 1);
  int len = (int)lua_tonumber(L, 2);
  int16_t x = (int16_t)lua_tonumber(L, 3);
  int16_t y = (int16_t)lua_tonumber(L, 4);
  
  //memcpy(_screenBuffer + (y * canvas.width() + x) + 1, sprites[name], i * (sizeof(unit16_t) * 65));
  customDrawRGBBitmap(x,y,sprites[name]->pixels,64,64);
  return 1;
}

void custom_memcpy(void* dest, const void* src, int len)
{
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);
    
    
    int iterator = 0;
    while (len--)
    {
        if (static_cast<uint16_t>(s[0]) != 234 && static_cast<uint16_t>(s[0]) != 96)
        {
                //canvas.println(static_cast<uint16_t>(s[0]));
          *d++ = *s++;
        }
        else
        {
          *d++;
          *s++;
        }
    
        iterator += 1;
    }

}

int lua_drawSprite(lua_State* L)
{
  const char* name = lua_tostring(L, 1);
  int len = (int)lua_tonumber(L, 2);
  int16_t x = (int16_t)lua_tonumber(L, 3);
  int16_t y = (int16_t)lua_tonumber(L, 4);
  
  int16_t w = sprites[name]->width;
  int16_t h = sprites[name]->height;


  for(int i=0; i < h; i++)
  {
    
    //modify the starting point and width of the line of the pixel line segment to account for x-axis clipping
    //modify the line segment to copy the screenbuffer pixels over where the transparencies should be in the line
    /*
    for(int j=0; j < w; j++)
    {
      //overwrite the transparent pixels wiht whatever is in t he screen buffer already
      if (sprites[name]->pixels[((0+i) * w + j)] == 0xea60)
        sprites[name]->pixels[((0+i) * w + j)] = _screenBuffer[( (y+i) * canvas.width() + x + j)];

    }
    */
    if ((y+i) < canvas.height() && (y+i) >= 0 && (x+w > 0) && (x < canvas.width()))
      if(w + x >=  canvas.width())
        custom_memcpy(_screenBuffer + ( (y+i) * canvas.width() + x), sprites[name]->pixels + ((0+i) * w + 0), sizeof(uint16_t) *  (w - ((x+(w))- canvas.width())));
      else if(x <= 0)
        custom_memcpy(_screenBuffer + ( (y+i) * canvas.width() + 0), sprites[name]->pixels + ((0+i) * w - (x-1)), sizeof(uint16_t) * (w + x));
      else
        custom_memcpy(_screenBuffer + ( (y+i) * canvas.width() + x), sprites[name]->pixels + ((0+i) * w + 0), sizeof(uint16_t) * w);
  }

  return 1;
}


int lua_println(lua_State* L)
{
	const char* stuff = lua_tostring(L, 1);

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

  

  lua_register(L, "convertHex", lua_convertHex);
  lua_register(L, "myrtleRequire", lua_require);
  lua_register(L, "myrtlePrintln", lua_println);
  lua_register(L, "myrtlePrint", lua_print);
  lua_register(L, "myrtleSetTextColor", lua_setTextColor);

  tft.println("Loaded Lua functions successfully");
  //delay(2000);

  //luaL_dostring(L,"package.path = package.path .. ';./?.lua;/littlefs/?.lua'");
  runScript("main.lua");
  tft.println("Loaded Lua scripts successfully");
  delay(2000);

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
  
  
  //delay(4000);

  tft.println("ran Game Load successfully");
  //tft.drawCircle(32, 32, 32, ST77XX_BLUE);
  
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  engineTime = millis();
  previousTime = millis();
}

void clearScreen(uint16_t color) {
  memset(_screenBuffer, color, 240 * 135 * sizeof(uint16_t));
}

void clearLine(int offset, uint16_t color) {
  memset(_screenBuffer + (240 * offset), color, 240 * sizeof(uint16_t));
}

// int clearOffset = 0;

void loop() {
  previousTime = engineTime;
  
  
  //canvas lets the draw to screen be not have flicker
  // clearLine(clearOffset, 0xf000);
  // clearOffset = (clearOffset + 1) % (135);
  clearScreen(0x0000);
  canvas.setCursor(0, 0);

  //engine loop update from main.lua
  lua_getglobal(L, "myrtle_update");
  if (lua_isfunction(L, -1))
  {
    lua_pcall(L, 0, 0, 0);
  }
  //engine draw from main.lua
  lua_getglobal(L, "myrtle_draw");
  if (lua_isfunction(L, -1))
  {
    lua_pcall(L, 0, 0, 0);
  }

  engineTime = millis();
  deltaTime = engineTime - previousTime;

  canvas.println("Frame Time (ms): ");
  canvas.println(deltaTime);

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
  // tft.writePixels(canvas.getBuffer(), canvas.width() * canvas.height());
}
