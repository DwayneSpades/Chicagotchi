# Chicagotchi
Chicago Tomogachi project running on the Myrtle framework.

[Myrtle framework's Design and Notes:

[Framework Design]
C++ side is for providing access to gfx drawing funcitons from lua scripts with wrapper functions. lua side is for programming the whole game. 
I highly reccommend not putting any game code on the c++ side at all. 
The reaseon for that is to minimize needing to transfer and access  data from lua to c++ via the lua stack with more complex data structures you would have to track through the stack. 
IMO it's unneccessary casue the whole game's code can be written in lua - Slaps

[Notes on Optimization and Lua Memory use with the Garbage Collector]
use C++ to optimize wrapper functions to call from the lua scripts. like boosting math fuctions with SIMD and giving back the values to the scripts faster than lua can.
^doing math in c++ would be a good idea to minimize memory use because lua generates garbage with every equation and object made. 
This could overburden the garbage collector and cause freezes on the main thread. this is an extreme case tho.
I don't think that will be an issue here unless we were doing something with tons of computations each frame 

[MAKING WRAPPER FUNCTIONS for LUA]
register a lua wrapper function to the lua vm with: ex. -> lua_register(L, "drawCircle", lua_drawCircle); 
1st input is the lua vm instance. 
2nd input is the name the lua function will be called by in the scripts. 
3rd input is the name of the wrapper function in c++
*CAUTION* always return an int in your wrapper function. its for the lua vm not the lua script. if you want to return a value to the lua script use: lua_pushnumber(L,returnValue);

~ Slaps
