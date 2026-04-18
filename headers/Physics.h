//
//  Physics.h
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//

#ifndef Physics_h
#define Physics_h

#include "HitResult.h"
#include "ContactListener.h"
#include <vector>
#include <algorithm>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

class Physics {
public:
    static b2World* world;
    static lua_State* lua_state;
    
    struct Actor;


    static ContactListener contact_listener;

    static void Init() {
        if (!world) {
            b2Vec2 gravity(0.0f, 9.8f);
            world = new b2World(gravity);
            world->SetContactListener(&contact_listener);
        }
    }
 
    static void Step() {
        if (world) {
            world->Step(1.0f / 60.0f, 8, 3);
        }
    }
    
    static luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist);
    static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist);
 
};
 

#endif /* Physics_h */
