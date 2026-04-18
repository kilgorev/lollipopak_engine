//
//  HitResult.h
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//
#ifndef hitresult_h
#define hitresult_h

#include "box2d/box2d.h"

struct Actor;

struct HitResult {
    Actor* actor    = nullptr;
    b2Vec2 point    = b2Vec2(0.0f, 0.0f);
    b2Vec2 normal   = b2Vec2(0.0f, 0.0f);
    bool is_trigger = false;
};

#endif /* hitresult_h */
