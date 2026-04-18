//
//  Rigidbody.cpp
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//

#include <stdio.h>

#include "Rigidbody.h"
#include "Physics.h"
#include "glm/glm/glm.hpp"

const uint16_t CATEGORY_COLLIDER = 0x0001;
const uint16_t CATEGORY_TRIGGER  = 0x0002;

void Rigidbody::OnStart() {
    b2BodyDef bodyDef;
    
    if      (body_type == "dynamic")   bodyDef.type = b2_dynamicBody;
    else if (body_type == "static")    bodyDef.type = b2_staticBody;
    else if (body_type == "kinematic") bodyDef.type = b2_kinematicBody;
    else                               bodyDef.type = b2_dynamicBody;
    
    bodyDef.position.Set(x, y);
    
    bodyDef.angle = (rotation * (b2_pi / 180.0f));
    
    bodyDef.bullet = precise;
    bodyDef.gravityScale = gravity_scale;
    bodyDef.angularDamping = angular_friction;
    
    body = Physics::world->CreateBody(&bodyDef);
    
    if (!has_collider && !has_trigger) {
        b2PolygonShape phantom_shape;
        phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);

        b2FixtureDef phantom_fixture_def;
        phantom_fixture_def.shape = &phantom_shape;
        phantom_fixture_def.density = density;
        phantom_fixture_def.isSensor = true;
        phantom_fixture_def.filter.categoryBits = 0x0000;
        phantom_fixture_def.filter.maskBits     = 0x0000;
        b2Fixture* f = body->CreateFixture(&phantom_fixture_def);
        f->GetUserData().pointer = reinterpret_cast<uintptr_t>(owner);
    }

    if (has_collider) {
        b2FixtureDef collider_def;
        collider_def.density = density;
        collider_def.friction = friction;
        collider_def.restitution = bounciness;
        collider_def.isSensor = false;
        collider_def.filter.categoryBits = CATEGORY_COLLIDER;
        collider_def.filter.maskBits = CATEGORY_COLLIDER;  // only collide with other colliders

        if (collider_type == "box") {
            b2PolygonShape box_shape;
            box_shape.SetAsBox(width * 0.5f, height * 0.5f);
            collider_def.shape = &box_shape;
            b2Fixture* f = body->CreateFixture(&collider_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(owner);
        }
        else if (collider_type == "circle") {
            b2CircleShape circle_shape;
            circle_shape.m_radius = radius;
            collider_def.shape = &circle_shape;
            b2Fixture* f = body->CreateFixture(&collider_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(owner);
        }
    }

    if (has_trigger) {
        b2FixtureDef trigger_def;
        trigger_def.density = density;
        trigger_def.isSensor = true;
        trigger_def.filter.categoryBits = CATEGORY_TRIGGER;
        trigger_def.filter.maskBits = CATEGORY_TRIGGER;  // only detect other triggers

        if (trigger_type == "box") {
            b2PolygonShape box_shape;
            box_shape.SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
            trigger_def.shape = &box_shape;
            b2Fixture* f = body->CreateFixture(&trigger_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(owner);
        }
        else if (trigger_type == "circle") {
            b2CircleShape circle_shape;
            circle_shape.m_radius = trigger_radius;
            trigger_def.shape = &circle_shape;
            b2Fixture* f = body->CreateFixture(&trigger_def);
            f->GetUserData().pointer = reinterpret_cast<uintptr_t>(owner);
        }
    }
}

b2Vec2 Rigidbody::GetPosition() const {
    if (body) return body->GetPosition();
    return b2Vec2(x, y);
}


void Rigidbody::AddForce(b2Vec2 force) {
    body->ApplyForceToCenter(force, true);
}

void Rigidbody::SetVelocity(b2Vec2 vel) {
    body->SetLinearVelocity(vel);
}

void Rigidbody::SetPosition(b2Vec2 pos) {
    if (body) {
        body->SetTransform(pos, body->GetAngle());
    }
    else {
        x = pos.x;
        y = pos.y;
    }
}



void Rigidbody::SetGravityScale(float scale) {
    body->SetGravityScale(scale);
}

void Rigidbody::SetUpDirection(b2Vec2 direction) {
    direction.Normalize();
    float angle = glm::atan(direction.x, -direction.y);
    body->SetTransform(body->GetPosition(), angle);
}

void Rigidbody::SetRightDirection(b2Vec2 direction) {
    direction.Normalize();
    float angle = glm::atan(direction.x, -direction.y) - (b2_pi / 2.0f);
    body->SetTransform(body->GetPosition(), angle);
}

b2Vec2 Rigidbody::GetVelocity() const {
    return body->GetLinearVelocity();
}


float Rigidbody::GetGravityScale() const {
    return body->GetGravityScale();
}

b2Vec2 Rigidbody::GetUpDirection() const {
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
    result.Normalize();
    return result;
}

b2Vec2 Rigidbody::GetRightDirection() const {
    float angle = body->GetAngle();
    b2Vec2 result =  b2Vec2(glm::cos(angle), glm::sin(angle));
    result.Normalize();
    return result;
}

float Rigidbody::GetRotation() const {
    if (body) return body->GetAngle() * (180.0f / b2_pi);
    return rotation;
}

void Rigidbody::SetRotation(float degrees_clockwise) {
    body->SetTransform(body->GetPosition(), degrees_clockwise * (b2_pi / 180.0f));
}

void Rigidbody::SetAngularVelocity(float degrees_clockwise) {
    body->SetAngularVelocity(degrees_clockwise * (b2_pi / 180.0f));
}

float Rigidbody::GetAngularVelocity() const {
    return body->GetAngularVelocity() * (180.0f / b2_pi);
}

void Rigidbody::OnDestroy() {
    if (body && Physics::world) {
        Physics::world->DestroyBody(body);
        body = nullptr;
    }
}

