//
//  Physics.cpp
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//
//
//  Physics.cpp
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//

#include "Physics.h"

b2World* Physics::world = nullptr;
ContactListener Physics::contact_listener;
lua_State* Physics::lua_state = nullptr;

class RaycastCallback : public b2RayCastCallback {
public:
    HitResult result;
    bool      hit     = false;
    float     closest = 1.0f;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                        const b2Vec2& normal, float fraction) override {
        if (fixture->GetFilterData().categoryBits == 0x0000) return -1;
        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
        if (!actor) return -1;
        if (fraction < closest) {
            closest           = fraction;
            result.actor      = actor;
            result.point      = point;
            result.normal     = normal;
            result.is_trigger = fixture->IsSensor();
            hit               = true;
        }
        return 1;
    }
};

class RaycastAllCallback : public b2RayCastCallback {
public:
    std::vector<std::pair<float, HitResult>> hits;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                        const b2Vec2& normal, float fraction) override {
        if (fixture->GetFilterData().categoryBits == 0x0000) return -1;
        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
        if (!actor) return -1;
        HitResult hr;
        hr.actor      = actor;
        hr.point      = point;
        hr.normal     = normal;
        hr.is_trigger = fixture->IsSensor();
        hits.push_back({ fraction, hr });
        return 1;
    }
};

luabridge::LuaRef Physics::Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
    if (!world || dist <= 0.0f)
        return luabridge::LuaRef(lua_state);

    dir.Normalize();
    b2Vec2 end = pos + dist * dir;

    RaycastCallback callback;
    world->RayCast(&callback, pos, end);

    if (!callback.hit)
        return luabridge::LuaRef(lua_state);

    return luabridge::LuaRef(lua_state, callback.result);
}

luabridge::LuaRef Physics::RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
    if (!world || dist <= 0.0f)
        return luabridge::LuaRef(lua_state);

    dir.Normalize();
    b2Vec2 end = pos + dist * dir;

    RaycastAllCallback callback;
    world->RayCast(&callback, pos, end);

    std::sort(callback.hits.begin(), callback.hits.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    luabridge::LuaRef table = luabridge::newTable(lua_state);
    int index = 1;
    for (auto& [fraction, hr] : callback.hits) {
        table[index++] = hr;
    }
    return table;
}
