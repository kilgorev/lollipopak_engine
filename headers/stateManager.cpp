//
//  stateManager.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#include <stdio.h>
#include "stateManager.h"

#include "stateManager.h"

StateManager::StateManager()
    : running(true) {
}

void StateManager::Quit(){
    exit(0);
}

void StateManager::Sleep(const int milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

int StateManager::GetFrame(){
    return Helper::GetFrameNumber();
}

void StateManager::OpenUrl(const std::string url){
    #ifdef _WIN32
        std::system(("start " + url).c_str());
    #elif __APPLE__
        std::system(("open " + url).c_str());
    #else
        std::system(("xdg-open " + url).c_str());
    #endif
}
