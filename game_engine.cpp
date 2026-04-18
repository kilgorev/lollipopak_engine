//
//  main.cpp
//  game_engine
//
//  Created by Tori Kilgore on 1/19/26.
//

#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>


#include "rapidjson/document.h"
#include "glm/glm/glm.hpp"
#include "sceneDB.h"
#include "input.h"
#include "render.h"
#include "engineUtils.h"
#include "Helper.h"
#include "AudioHelper.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "imageDB.h"
#include "textDB.h"
#include "audioDB.h"
#include "dialogue.h"
#include "engine.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
//grade

using namespace std;
int main(int argc, char* argv[]) {

    // lua_State* lua_state = luaL_newstate();
    // luaL_openlibs(lua_state);
    // luaL_dofile(lua_state, "resources/component_types/OutputMessage.lua");    
   // luabridge::LuaRef my_table = luabridge::getGlobal(lua_state, "OutputMessage");
    
   // cout << my_table["message"].cast<std::string>() << endl;
    
    
    Engine engine;
    engine.initialize();
    engine.run();
    return EXIT_SUCCESS;
}
