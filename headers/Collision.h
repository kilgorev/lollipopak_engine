#ifndef collision_h
#define collision_h

#include "box2d/box2d.h"

struct Actor;

struct Collision {
    Actor* other = nullptr;
    b2Vec2 point = b2Vec2(0.0f, 0.0f);
    b2Vec2 relative_velocity = b2Vec2(0.0f, 0.0f);
    b2Vec2 normal = b2Vec2(0.0f, 0.0f);
};

#endif /* collision_h */
