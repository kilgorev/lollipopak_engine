//
//  stateManager.h
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#ifndef stateManager_h
#define stateManager_h

#include <iostream>
#include <thread>
#include "Helper.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

class StateManager{
public:
    
    StateManager();
    
    bool isRunning() const { return running; }
    
    void setRunning(bool value) { running = value; }
    
    // APPLICATION API
    static void Quit();
    static void Sleep(const int milliseconds);
    static int GetFrame();
    static void OpenUrl(const std::string url);

private:
    bool running = true;
};

#endif /* stateManager_h */
