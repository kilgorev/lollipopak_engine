//
//  Rigidbody.h
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//

#ifndef Rigidbody_h
#define Rigidbody_h

#include "box2d/box2d.h"
#include <string>
 
struct Actor;

class Rigidbody {
public:
    std::string key = "Rigidbody";
    std::string type = "Rigidbody";
    bool enabled = true;
 
    float x = 0.0f;
    float y = 0.0f;
    std::string body_type = "dynamic";
    bool precise = true;
    float gravity_scale = 1.0f;
    float density = 1.0f;
    float angular_friction = 0.3f;
    float rotation = 0.0f;
    bool has_collider = true;
    bool has_trigger = true;
    float width  = 1.0f;
    float height = 1.0f;
    std::string collider_type = "box";
    float radius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;
    std::string trigger_type = "box";
    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;
 
    b2Body* body = nullptr;
    Actor* owner = nullptr;
 
    void OnStart();
 
    b2Vec2 GetPosition() const;
    float  GetRotation() const;
    void AddForce(b2Vec2 force);
    void SetVelocity(b2Vec2 vel);
    void SetPosition(b2Vec2 pos);
    void SetRotation(float degrees_clockwise);
    void SetAngularVelocity(float degrees_clockwise);
    void SetGravityScale(float scale);
    void SetUpDirection(b2Vec2 direction);
    void SetRightDirection(b2Vec2 direction);
    void OnDestroy();
    b2Vec2 GetVelocity() const;
    float  GetAngularVelocity() const;
    float  GetGravityScale() const;
    b2Vec2 GetUpDirection() const;
    b2Vec2 GetRightDirection() const;
};


#endif /* Rigidbody_h */
