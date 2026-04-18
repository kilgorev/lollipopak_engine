#include "ContactListener.h"
#include "Collision.h"
#include "sceneDB.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

static Actor* getActor(b2Fixture* fixture) {
    return reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
}

static void dispatchCollision(Actor* actor, const std::string& func_name,
                               const Collision& collision) {
    if (!actor) return;
    for (const auto& key : actor->keys) {
        auto it = actor->components.find(key);
        if (it == actor->components.end()) continue;
        auto& comp = it->second;
        if (!comp.componentRef) continue;
        luabridge::LuaRef ref = *comp.componentRef;
        luabridge::LuaRef func = ref[func_name];
        if (func.isFunction()) {
            try {
                func(ref, collision);
            } catch (const luabridge::LuaException& e) {
                //silent
            }
        }
    }
}

void ContactListener::BeginContact(b2Contact* contact) {
    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();

    Actor* actor_a = getActor(fixture_a);
    Actor* actor_b = getActor(fixture_b);

    if (fixture_a->IsSensor() && fixture_b->IsSensor()) {
        // Both sensors — trigger event
        const b2Vec2 sentinel(-999.0f, -999.0f);

        Collision collision_a;
        collision_a.other = actor_b;
        collision_a.point = sentinel;
        collision_a.normal = sentinel;
        collision_a.relative_velocity = fixture_a->GetBody()->GetLinearVelocity()
                                      - fixture_b->GetBody()->GetLinearVelocity();

        Collision collision_b;
        collision_b.other = actor_a;
        collision_b.point = sentinel;
        collision_b.normal = sentinel;
        collision_b.relative_velocity = collision_a.relative_velocity;

        dispatchCollision(actor_a, "OnTriggerEnter", collision_a);
        dispatchCollision(actor_b, "OnTriggerEnter", collision_b);
    }
    else if (!fixture_a->IsSensor() && !fixture_b->IsSensor()) {
        // Both non-sensors — collision event
        b2WorldManifold world_manifold;
        contact->GetWorldManifold(&world_manifold);

        b2Vec2 relative_velocity = fixture_a->GetBody()->GetLinearVelocity()
                                 - fixture_b->GetBody()->GetLinearVelocity();

        Collision collision_a;
        collision_a.other = actor_b;
        collision_a.point = world_manifold.points[0];
        collision_a.normal = world_manifold.normal;
        collision_a.relative_velocity = relative_velocity;

        Collision collision_b;
        collision_b.other = actor_a;
        collision_b.point = world_manifold.points[0];
        collision_b.normal = world_manifold.normal;
        collision_b.relative_velocity = relative_velocity;

        dispatchCollision(actor_a, "OnCollisionEnter", collision_a);
        dispatchCollision(actor_b, "OnCollisionEnter", collision_b);
    }
}

void ContactListener::EndContact(b2Contact* contact) {
    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();

    Actor* actor_a = getActor(fixture_a);
    Actor* actor_b = getActor(fixture_b);

    const b2Vec2 sentinel(-999.0f, -999.0f);

    b2Vec2 relative_velocity = fixture_a->GetBody()->GetLinearVelocity()
                             - fixture_b->GetBody()->GetLinearVelocity();

    if (fixture_a->IsSensor() && fixture_b->IsSensor()) {
        Collision collision_a;
        collision_a.other = actor_b;
        collision_a.point = sentinel;
        collision_a.normal = sentinel;
        collision_a.relative_velocity = relative_velocity;

        Collision collision_b;
        collision_b.other = actor_a;
        collision_b.point = sentinel;
        collision_b.normal = sentinel;
        collision_b.relative_velocity = relative_velocity;

        dispatchCollision(actor_a, "OnTriggerExit", collision_a);
        dispatchCollision(actor_b, "OnTriggerExit", collision_b);
    }
    else if (!fixture_a->IsSensor() && !fixture_b->IsSensor()) {
        Collision collision_a;
        collision_a.other = actor_b;
        collision_a.point = sentinel;
        collision_a.normal = sentinel;
        collision_a.relative_velocity = relative_velocity;

        Collision collision_b;
        collision_b.other = actor_a;
        collision_b.point = sentinel;
        collision_b.normal = sentinel;
        collision_b.relative_velocity = relative_velocity;

        dispatchCollision(actor_a, "OnCollisionExit", collision_a);
        dispatchCollision(actor_b, "OnCollisionExit", collision_b);
    }
}
