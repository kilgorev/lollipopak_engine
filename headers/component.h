//
//  component.h
//  game_engine
//
//  Created by Tori Kilgore on 2/25/26.
//
#ifndef COMPONENT_H
#define COMPONENT_H

#include <iostream>
#include <memory>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

class Rigidbody;
class ParticleSystem;

class Component{
public:
    Rigidbody* rigidbody = nullptr;
    ParticleSystem* particlesystem = nullptr;
    
    explicit Component();
    
    bool isEnabled() const;

    std::shared_ptr<luabridge::LuaRef> componentRef;
    std::string type;
    std::string actor_name = "";
    
    bool hasStart;
    bool hasUpdate;
    bool hasLateUpdate;
    bool hasDestroy = false;
};

#endif
