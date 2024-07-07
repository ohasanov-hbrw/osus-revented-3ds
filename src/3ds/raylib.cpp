#include "raylib.h"
#include "globals.hpp"

bool WindowShouldClose(){
    return (Global.ds_kDown & KEY_START);
}

