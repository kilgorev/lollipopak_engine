#ifndef SCENEDB_H
#define SCENEDB_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "engineUtils.h"
#include "Helper.h"
#include "glm/glm/glm.hpp"
#include "component.h"
#include "componentDB.h"
#include "Rigidbody.h"
#include "Physics.h"
#include "ParticleSystem.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"


using namespace std;
extern int component_counter;

struct Actor {
    std::string actor_name = "";
    bool dont_destroy = false;
    int id = -1;
    
    vector<string> keys;
    unordered_map<string, Component> components;
    vector<Component> pending_components;
    vector<luabridge::LuaRef> pending_remove;
    
    static unordered_map<string, Actor*> actor_map;
    static vector<Actor*>* actor_list;
    static vector<Actor*>* pending_actors_list;

    static lua_State* componentManager_lua_state;
    
    Actor() = default;
    
    Actor(const std::string& actor_name)
    : actor_name(actor_name) {}
    
    // lua access
    
    std::string GetName() const { return actor_name; }
    int GetID() const { return id; }
    
    
    luabridge::LuaRef GetComponentByKey(const std::string& key) const {
        auto it = components.find(key);
        if(it == components.end() || !it->second.componentRef || !it->second.isEnabled())
            return luabridge::LuaRef(componentManager_lua_state);
        return *it->second.componentRef;
    }
    
    luabridge::LuaRef GetComponent(const std::string &type_name){
        for(const auto& k : keys){
            auto it = components.find(k);
            if(it != components.end() && it->second.type == type_name && it->second.componentRef){
                if(!it->second.isEnabled()) continue;
                if(it->second.rigidbody){
                    return luabridge::LuaRef(componentManager_lua_state, it->second.rigidbody);
                }
                if(it->second.particlesystem){
                    return luabridge::LuaRef(componentManager_lua_state, it->second.particlesystem);
                }
                return *it->second.componentRef;
            }
        }
        return luabridge::LuaRef(componentManager_lua_state);
    }
    
    luabridge::LuaRef GetComponents(const std::string &type_name){
        luabridge::LuaRef table = luabridge::newTable(componentManager_lua_state);
        int index = 1;
        for(const auto &k: keys){
            auto it = components.find(k);
            if(it != components.end() && it->second.type == type_name && it->second.componentRef) {
                if(!it->second.isEnabled()) continue;
                table[index++] = *it->second.componentRef;
            }
        }
        return table;
    }
    
    static luabridge::LuaRef Find(const std::string& name) {
        auto it = actor_map.find(name);
        if (it == actor_map.end())
            return luabridge::LuaRef(componentManager_lua_state);
        return luabridge::LuaRef(componentManager_lua_state, it->second);
    }
    
    static luabridge::LuaRef FindAll(const std::string& name) {
        luabridge::LuaRef table = luabridge::newTable(componentManager_lua_state);
        int index = 1;
        if (actor_list) {
            for (auto& a : *actor_list) {
                if (a->actor_name == name)
                    table[index++] = luabridge::LuaRef(componentManager_lua_state, a);
            }
        }
        if (pending_actors_list) {
            for (auto& a : *pending_actors_list) {
                if (a->actor_name == name)
                    table[index++] = luabridge::LuaRef(componentManager_lua_state, a);
            }
        }
        return table;
    }
    
    
    void InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref){
        (*component_ref)["actor"] = this;
    }
    
    luabridge::LuaRef AddComponent(const std::string type_name){
        string key = "r" + to_string(component_counter++);
        
        if (type_name == "Rigidbody") {
            Physics::Init();
            Rigidbody* rb = new Rigidbody();
            rb->key   = key;
            rb->type  = type_name;
            rb->owner = this;
            
            auto instance = std::make_shared<luabridge::LuaRef>(
                                                                luabridge::newTable(componentManager_lua_state));
            (*instance)["key"]     = key;
            (*instance)["type"]    = type_name;
            (*instance)["enabled"] = true;
            (*instance)["actor"]   = this;
            (*instance)["rb"]      = rb;
            
            Component component;
            component.type         = type_name;
            component.componentRef = instance;
            component.rigidbody    = rb;
            component.actor_name   = actor_name;
            component.hasStart     = true;
            component.hasUpdate    = false;
            component.hasLateUpdate = false;
            
            pending_components.push_back(component);
            return luabridge::LuaRef(componentManager_lua_state, rb);
        }
        
        if (type_name == "ParticleSystem") {
            ParticleSystem *ps =  new ParticleSystem();
            ps->key = key;
            ps->type = type_name;
            ps->owner = this;
            
            auto instance = std::make_shared<luabridge::LuaRef>(
                                                                luabridge::newTable(componentManager_lua_state));
            (*instance)["key"]     = key;
            (*instance)["type"]    = type_name;
            (*instance)["enabled"] = true;
            (*instance)["actor"]   = this;
            (*instance)["ps"]      = ps;
            
            Component component;
            component.type         = type_name;
            component.componentRef = instance;
            component.particlesystem  = ps;
            component.actor_name   = actor_name;
            component.hasStart     = true;
            component.hasUpdate    = true;
            component.hasLateUpdate = false;
            
            pending_components.push_back(component);
            return luabridge::LuaRef(componentManager_lua_state, ps);
            
        }
        
        string path = "resources/component_types/" + type_name + ".lua";
        if(luaL_dofile(componentManager_lua_state, path.c_str()) != LUA_OK){
            cout << "problem with lua file " << type_name;
            exit(0);
        }
        
        luabridge::LuaRef parent = luabridge::getGlobal(componentManager_lua_state, type_name.c_str());
        auto instance = std::make_shared<luabridge::LuaRef>(luabridge::newTable(componentManager_lua_state));
        
        luabridge::LuaRef metatable = luabridge::newTable(componentManager_lua_state);
        metatable["__index"] = parent;
        instance->push(componentManager_lua_state);
        metatable.push(componentManager_lua_state);
        lua_setmetatable(componentManager_lua_state, -2);
        lua_pop(componentManager_lua_state, 1);
        
        (*instance)["key"] = key;
        (*instance)["enabled"] = true;
        (*instance)["actor"] = this;
        
        Component component;
        component.type = type_name;
        component.componentRef = instance;
        component.actor_name = actor_name;
        component.hasStart = parent["OnStart"].isFunction();
        component.hasUpdate = parent["OnUpdate"].isFunction();
        component.hasLateUpdate = parent["OnLateUpdate"].isFunction();
        
        pending_components.push_back(component);
        
        return *instance;
    }
    
    void RemoveComponent(luabridge::LuaRef component_ref) {
        component_ref["enabled"] = false;
        
        string key = component_ref["key"].cast<string>();
        auto it = components.find(key);
        if (it != components.end() && it->second.rigidbody) {
            it->second.rigidbody->OnDestroy();
        }
        
        pending_remove.push_back(component_ref);
    }
};

struct PairHash {
    size_t operator()(const pair<int, int>& p) const noexcept {
        return (static_cast<size_t>(p.first) << 32)
             ^ static_cast<size_t>(p.second);
    }
};

class SceneDB {
public:
    static int n;
    vector<Actor*> actors;
    vector<Actor*> pending_actors;
    static vector<Actor*> pending_destroy;
    std::string current_scene_name = "";
    
    static SceneDB* instance;
    static ComponentDB* componentDB;

    unordered_map<string, rapidjson::Document> template_cache;

    Component createComponentForActor(const std::string& key,
                                      const std::string& type,
                                      ComponentDB& componentHandler,
                                      const rapidjson::Value& valueObj) {
        if (type == "Rigidbody") {
            Physics::Init();
 
            Rigidbody* rb = new Rigidbody();
            rb->key  = key;
            rb->type = type;
 
            for (auto ov = valueObj.MemberBegin(); ov != valueObj.MemberEnd(); ++ov) {
                std::string field = ov->name.GetString();
                if (field == "type") continue;
                if      (field == "x"                && (ov->value.IsFloat() || ov->value.IsInt())) rb->x                = ov->value.GetFloat();
                else if (field == "y"                && (ov->value.IsFloat() || ov->value.IsInt())) rb->y                = ov->value.GetFloat();
                else if (field == "rotation"         && (ov->value.IsFloat() || ov->value.IsInt())) rb->rotation         = ov->value.GetFloat();
                else if (field == "gravity_scale"    && (ov->value.IsFloat() || ov->value.IsInt())) rb->gravity_scale    = ov->value.GetFloat();
                else if (field == "density"          && (ov->value.IsFloat() || ov->value.IsInt())) rb->density          = ov->value.GetFloat();
                else if (field == "angular_friction" && (ov->value.IsFloat() || ov->value.IsInt())) rb->angular_friction = ov->value.GetFloat();
                else if (field == "body_type"        && ov->value.IsString())                       rb->body_type        = ov->value.GetString();
                else if (field == "precise"          && ov->value.IsBool())                         rb->precise          = ov->value.GetBool();
                else if (field == "has_collider"     && ov->value.IsBool())                         rb->has_collider     = ov->value.GetBool();
                else if (field == "has_trigger"      && ov->value.IsBool())                         rb->has_trigger      = ov->value.GetBool();
                else if (field == "enabled"          && ov->value.IsBool())                         rb->enabled          = ov->value.GetBool();
                else if (field == "width"  && (ov->value.IsFloat() || ov->value.IsInt())) rb->width  = ov->value.GetFloat();
                else if (field == "height" && (ov->value.IsFloat() || ov->value.IsInt())) rb->height = ov->value.GetFloat();
                else if (field == "collider_type" && ov->value.IsString())                       rb->collider_type = ov->value.GetString();
                else if (field == "radius"        && (ov->value.IsFloat() || ov->value.IsInt())) rb->radius        = ov->value.GetFloat();
                else if (field == "friction"      && (ov->value.IsFloat() || ov->value.IsInt())) rb->friction      = ov->value.GetFloat();
                else if (field == "bounciness"    && (ov->value.IsFloat() || ov->value.IsInt())) rb->bounciness    = ov->value.GetFloat();
                else if (field == "trigger_type"   && ov->value.IsString())                       rb->trigger_type   = ov->value.GetString();
                else if (field == "trigger_width"  && (ov->value.IsFloat() || ov->value.IsInt())) rb->trigger_width  = ov->value.GetFloat();
                else if (field == "trigger_height" && (ov->value.IsFloat() || ov->value.IsInt())) rb->trigger_height = ov->value.GetFloat();
                else if (field == "trigger_radius" && (ov->value.IsFloat() || ov->value.IsInt())) rb->trigger_radius = ov->value.GetFloat();
            }
            
            
 
            auto instance = std::make_shared<luabridge::LuaRef>(
                luabridge::newTable(componentHandler.lua_state));
            (*instance)["key"]     = key;
            (*instance)["type"]    = type;
            (*instance)["enabled"] = rb->enabled;
            (*instance)["rb"]      = rb;
 
            Component component;
            component.type          = type;
            component.componentRef  = instance;
            component.rigidbody     = rb;
            component.hasStart      = true;
            component.hasUpdate     = false;
            component.hasLateUpdate = false;
            return component;
        }
        
        if(type == "ParticleSystem") {
            ParticleSystem *ps = new ParticleSystem();
            ps->key = key;
            ps->type = type;
            
            for (auto ov = valueObj.MemberBegin(); ov != valueObj.MemberEnd(); ++ov) {
                std::string field = ov->name.GetString();
                if (field == "type") continue;
                if      (field == "x"                && (ov->value.IsFloat() || ov->value.IsInt())) ps->x                = ov->value.GetFloat();
                else if (field == "y"                && (ov->value.IsFloat() || ov->value.IsInt())) ps->y                = ov->value.GetFloat();
                else if (field == "frames_between_bursts" && ov->value.IsInt())
                    ps->frames_between_bursts = ov->value.GetInt();
                else if (field == "burst_quantity" && ov->value.IsInt())
                    ps->burst_quantity = ov->value.GetInt();
                else if (field == "start_scale_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->start_scale_min = ov->value.GetFloat();
                else if (field == "start_scale_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->start_scale_max = ov->value.GetFloat();
                else if (field == "start_color_r" && ov->value.IsInt())
                    ps->start_color_r = ov->value.GetInt();
                else if (field == "start_color_b" && ov->value.IsInt())
                    ps->start_color_b = ov->value.GetInt();
                else if (field == "start_color_g" && (ov->value.IsInt()))
                    ps->start_color_g = ov->value.GetInt();
                else if (field == "start_color_a" && (ov->value.IsInt()))
                    ps->start_color_a = ov->value.GetInt();
                else if (field == "emit_radius_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->emit_radius_min = ov->value.GetFloat();
                else if (field == "emit_radius_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->emit_radius_max = ov->value.GetFloat();
                else if (field == "emit_angle_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->emit_angle_min = ov->value.GetFloat();
                else if (field == "emit_angle_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->emit_angle_max = ov->value.GetFloat();
                else if (field == "image" && ov->value.IsString())
                    ps->image = ov->value.GetString();
                else if (field == "sorting_order" && ov->value.IsInt())
                    ps->sorting_order = ov->value.GetInt();
                else if (field == "duration_frames" && ov->value.IsInt())
                    ps->duration_frames = ov->value.GetInt();
                else if (field == "start_speed_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->start_speed_min = ov->value.GetFloat();
                else if (field == "start_speed_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->start_speed_max = ov->value.GetFloat();
                else if (field == "rotation_speed_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->rotation_speed_min = ov->value.GetFloat();
                else if (field == "rotation_speed_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->rotation_speed_max = ov->value.GetFloat();
                else if (field == "gravity_scale_x" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->gravity_scale_x = ov->value.GetFloat();
                else if (field == "gravity_scale_y" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->gravity_scale_y = ov->value.GetFloat();
                else if (field == "drag_factor" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->drag_factor = ov->value.GetFloat();
                else if (field == "angular_drag_factor" && (ov->value.IsFloat() || ov->value.IsInt()))
                    ps->angular_drag_factor = ov->value.GetFloat();
                else if (field == "end_scale" && (ov->value.IsFloat() || ov->value.IsInt())) {
                    ps->end_scale = ov->value.GetFloat();
                    ps->has_end_scale = true;
                }
                else if (field == "end_color_r" && (ov->value.IsFloat() || ov->value.IsInt())) {
                    ps->end_color_r = int(ov->value.GetFloat());
                    ps->has_end_color_r = true;
                }
                else if (field == "end_color_g" && (ov->value.IsFloat() || ov->value.IsInt())) {
                    ps->end_color_g = int(ov->value.GetFloat());
                    ps->has_end_color_g = true;
                }
                else if (field == "end_color_b" && (ov->value.IsFloat() || ov->value.IsInt())) {
                    ps->end_color_b = int(ov->value.GetFloat());
                    ps->has_end_color_b = true;
                }
                else if (field == "end_color_a" && (ov->value.IsFloat() || ov->value.IsInt())) {
                    ps->end_color_a = int(ov->value.GetFloat());
                    ps->has_end_color_a = true;
                }
            }
            
            auto instance = std::make_shared<luabridge::LuaRef>(
                luabridge::newTable(componentHandler.lua_state));
            (*instance)["key"]     = key;
            (*instance)["type"]    = type;
            (*instance)["enabled"] = ps->enabled;
            (*instance)["ps"]      = ps;
 
            Component component;
            component.type          = type;
            component.componentRef  = instance;
            component.particlesystem     = ps;
            component.hasStart      = true;
            component.hasUpdate     = true;
            component.hasLateUpdate = false;
            return component;
        }
        
        componentHandler.getComponent(key, type);
        luabridge::LuaRef parent =
            luabridge::getGlobal(componentHandler.lua_state, type.c_str());
        auto instance =
            std::make_shared<luabridge::LuaRef>(
                luabridge::newTable(componentHandler.lua_state));
        componentHandler.establishInheritance(*instance, parent);
 
        (*instance)["key"]     = key;
        (*instance)["enabled"] = true;
 
        luabridge::LuaRef table = *instance;
        for (auto ov = valueObj.MemberBegin(); ov != valueObj.MemberEnd(); ++ov) {
            std::string field = ov->name.GetString();
            if (field == "type") continue;
            if (ov->value.IsString())      table[field] = ov->value.GetString();
            else if (ov->value.IsInt())    table[field] = ov->value.GetInt();
            else if (ov->value.IsFloat())  table[field] = ov->value.GetFloat();
            else if (ov->value.IsBool())   table[field] = ov->value.GetBool();
        }
 
        Component component;
        component.type = type;
        component.componentRef = instance;
        component.hasStart = parent["OnStart"].isFunction();
        component.hasUpdate = parent["OnUpdate"].isFunction();
        component.hasLateUpdate = parent["OnLateUpdate"].isFunction();
        return component;
    }
 
    inline void applyActorFields(Actor& actor, ComponentDB& componentHandler,
                                 const rapidjson::Value& src) {
        if (src.HasMember("name"))
            actor.actor_name = src["name"].GetString();
        
        if (src.HasMember("template")) {
            string temp = src["template"].GetString();
            const auto& temp_doc = getTemplate(temp);
            applyActorFields(actor, componentHandler, temp_doc);
        }
        
        if (src.HasMember("components") && src["components"].IsObject()) {
            for (auto it = src["components"].MemberBegin();
                 it != src["components"].MemberEnd(); ++it) {
                string key = it->name.GetString();
 
                if (actor.components.find(key) != actor.components.end()) {
                    Component& existing = actor.components[key];
 
                    if (existing.rigidbody) {
                        for (auto ov = it->value.MemberBegin(); ov != it->value.MemberEnd(); ++ov) {
                            std::string field = ov->name.GetString();
                            if (field == "type") continue;
                            if      (field == "x"                && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->x                = ov->value.GetFloat();
                            else if (field == "y"                && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->y                = ov->value.GetFloat();
                            else if (field == "rotation"         && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->rotation         = ov->value.GetFloat();
                            else if (field == "gravity_scale"    && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->gravity_scale    = ov->value.GetFloat();
                            else if (field == "density"          && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->density          = ov->value.GetFloat();
                            else if (field == "angular_friction" && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->angular_friction = ov->value.GetFloat();
                            else if (field == "body_type"        && ov->value.IsString())                       existing.rigidbody->body_type        = ov->value.GetString();
                            else if (field == "precise"          && ov->value.IsBool())                         existing.rigidbody->precise          = ov->value.GetBool();
                            else if (field == "has_collider"     && ov->value.IsBool())                         existing.rigidbody->has_collider     = ov->value.GetBool();
                            else if (field == "has_trigger"      && ov->value.IsBool())                         existing.rigidbody->has_trigger      = ov->value.GetBool();
                            else if (field == "enabled"          && ov->value.IsBool())                         existing.rigidbody->enabled          = ov->value.GetBool();
                            else if (field == "width"  && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->width  = ov->value.GetFloat();
                            else if (field == "height" && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->height = ov->value.GetFloat();
                            else if (field == "collider_type" && ov->value.IsString())                       existing.rigidbody->collider_type = ov->value.GetString();
                            else if (field == "radius"        && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->radius        = ov->value.GetFloat();
                            else if (field == "friction"      && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->friction      = ov->value.GetFloat();
                            else if (field == "bounciness"    && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->bounciness    = ov->value.GetFloat();
                            else if (field == "trigger_type"   && ov->value.IsString())                       existing.rigidbody->trigger_type   = ov->value.GetString();
                            else if (field == "trigger_width"  && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->trigger_width  = ov->value.GetFloat();
                            else if (field == "trigger_height" && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->trigger_height = ov->value.GetFloat();
                            else if (field == "trigger_radius" && (ov->value.IsFloat() || ov->value.IsInt())) existing.rigidbody->trigger_radius = ov->value.GetFloat();
                            
                        }
                    }
                    else if (existing.particlesystem) {
                        for (auto ov = it->value.MemberBegin(); ov != it->value.MemberEnd(); ++ov) {
                            std::string field = ov->name.GetString();
                            if (field == "type") continue;
                            if (field == "x" && (ov->value.IsFloat() || ov->value.IsFloat()))
                                existing.particlesystem->x = ov->value.GetFloat();
                            else if (field == "y" && (ov->value.IsFloat() || ov->value.IsFloat()))
                                existing.particlesystem->y = ov->value.GetFloat();
                            else if (field == "frames_between_bursts" && ov->value.IsInt())
                                existing.particlesystem->frames_between_bursts = ov->value.GetInt();
                            else if (field == "burst_quantity" && ov->value.IsInt())
                                existing.particlesystem->burst_quantity = ov->value.GetInt();
                            else if (field == "start_scale_min" &&(ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->start_scale_min = ov->value.GetFloat();
                            else if (field == "start_scale_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->start_scale_max = ov->value.GetFloat();
                            else if (field == "start_color_r" && ov->value.IsInt())
                                existing.particlesystem->start_color_r = ov->value.GetInt();
                            else if (field == "start_color_b" && ov->value.IsInt())
                                existing.particlesystem->start_color_b = ov->value.GetInt();
                            else if (field == "start_color_g" && ov->value.IsInt())
                                existing.particlesystem->start_color_g = ov->value.GetInt();
                            else if (field == "start_color_a" && ov->value.IsInt())
                                existing.particlesystem->start_color_a = ov->value.GetInt();
                            else if (field == "emit_radius_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->emit_radius_min = ov->value.GetFloat();
                            else if (field == "emit_radius_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->emit_radius_max = ov->value.GetFloat();
                            else if (field == "emit_angle_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->emit_angle_min = ov->value.GetFloat();
                            else if (field == "emit_angle_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->emit_angle_max = ov->value.GetFloat();
                            else if (field == "image" && ov->value.IsString())
                                existing.particlesystem->image = ov->value.GetString();
                            else if (field == "sorting_order" && ov->value.IsInt())
                                existing.particlesystem->sorting_order = ov->value.GetInt();
                            else if (field == "duration_frames" && ov->value.IsInt())
                                existing.particlesystem->duration_frames = ov->value.GetInt();
                            else if (field == "start_speed_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->start_speed_min = ov->value.GetFloat();
                            else if (field == "start_speed_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->start_speed_max = ov->value.GetFloat();
                            else if (field == "rotation_speed_min" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->rotation_speed_min = ov->value.GetFloat();
                            else if (field == "rotation_speed_max" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->rotation_speed_max = ov->value.GetFloat();
                            else if (field == "gravity_scale_x" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->gravity_scale_x = ov->value.GetFloat();
                            else if (field == "gravity_scale_y" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->gravity_scale_y = ov->value.GetFloat();
                            else if (field == "drag_factor" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->drag_factor = ov->value.GetFloat();
                            else if (field == "angular_drag_factor" && (ov->value.IsFloat() || ov->value.IsInt()))
                                existing.particlesystem->angular_drag_factor = ov->value.GetFloat();
                            else if (field == "end_scale" && (ov->value.IsFloat() || ov->value.IsInt())) {
                                existing.particlesystem->end_scale = ov->value.GetFloat();
                                existing.particlesystem->has_end_scale = true;
                            }
                            else if (field == "end_color_r" && (ov->value.IsFloat() || ov->value.IsInt())) {
                                existing.particlesystem->end_color_r = int(ov->value.GetFloat());
                                existing.particlesystem->has_end_color_r = true;
                            }
                            else if (field == "end_color_g" && (ov->value.IsFloat() || ov->value.IsInt())) {
                                existing.particlesystem->end_color_g = int(ov->value.GetFloat());
                                existing.particlesystem->has_end_color_g = true;
                            }
                            else if (field == "end_color_b" && (ov->value.IsFloat() || ov->value.IsInt())) {
                                existing.particlesystem->end_color_b = int(ov->value.GetFloat());
                                existing.particlesystem->has_end_color_b = true;
                            }
                            else if (field == "end_color_a" && (ov->value.IsFloat() || ov->value.IsInt())) {
                                existing.particlesystem->end_color_a = int(ov->value.GetFloat());
                                existing.particlesystem->has_end_color_a = true;
                            }
                        }
                    }
                    else {
                        luabridge::LuaRef table = *existing.componentRef;
                        for (auto ov = it->value.MemberBegin();
                             ov != it->value.MemberEnd(); ++ov) {
                            std::string field = ov->name.GetString();
                            if (field == "type") continue;
                            if (ov->value.IsString())     table[field] = ov->value.GetString();
                            else if (ov->value.IsInt())   table[field] = ov->value.GetInt();
                            else if (ov->value.IsFloat()) table[field] = ov->value.GetFloat();
                            else if (ov->value.IsBool())  table[field] = ov->value.GetBool();
                        }
                    }
                }
                else if (it->value.HasMember("type")) {
                    string type = it->value["type"].GetString();
                    actor.components[key] =
                        createComponentForActor(key, type, componentHandler, it->value);
                }
            }
        }
    }

    const rapidjson::Document& getTemplate(const string &temp) {
        auto it = template_cache.find(temp);
        if (it != template_cache.end())
            return it->second;

        string path = "resources/actor_templates/" + temp + ".template";
        if (!filesystem::exists(path)) {
            cout << "error: template " + temp + " is missing";
            exit(0);
        }

        rapidjson::Document temp_doc;
        EngineUtils::ReadJsonFile(path, temp_doc);
        template_cache[temp] = std::move(temp_doc);
        return template_cache[temp];
    }

    void loadActors(rapidjson::Document &doc, ComponentDB &componentHandler, SDL_Renderer *renderer, const std::string scene_name) {
        SceneDB::instance = this;
        instance->current_scene_name = scene_name;
        Actor::pending_actors_list = &pending_actors;
        SceneDB::componentDB = &componentHandler;
        actors.clear();
        if (doc.HasMember("actors") && !doc["actors"].Empty()) {
            const auto actorArray = doc["actors"].GetArray();
            actors.reserve(actorArray.Size());

            for (int i = 0; i < actorArray.Size(); ++i) {
                Actor* actor = new Actor();
                actor->id = i;

                if (actorArray[i].HasMember("template")) {
                    string temp = actorArray[i]["template"].GetString();
                    const auto &temp_doc = getTemplate(temp);
                    applyActorFields(*actor, componentHandler, temp_doc);
                }
                applyActorFields(*actor, componentHandler, actorArray[i]);

                for(auto& comp : actor->components)
                    actor->keys.push_back(comp.first);
                sort(actor->keys.begin(), actor->keys.end());

                actors.push_back(actor);
                Actor* stable = actors.back();
                Actor::actor_map[stable->actor_name] = stable;

                for(auto& key : stable->keys){
                    auto& comp = stable->components[key];
                    comp.actor_name = stable->actor_name;
                    (*comp.componentRef)["actor"] = stable;
                    if(comp.rigidbody) comp.rigidbody->owner = stable;
                    if(comp.particlesystem) comp.particlesystem->owner = stable;
                    if(comp.hasStart) componentHandler.on_start_queue.push(comp);
                    if(comp.hasUpdate) componentHandler.update.push_back(comp);
                    if(comp.hasLateUpdate) componentHandler.late_update.push_back(comp);
                }
            }
    
        }
        
        Actor::actor_list = &actors;
    }
    
    static Actor* Instantiate(const std::string& template_name){
        Actor* actor = new Actor();
        actor->id = int(instance->actors.size()) + int(instance->pending_actors.size());
        
        const auto& temp_doc = instance->getTemplate(template_name);
        instance->applyActorFields(*actor, *componentDB, temp_doc);
        
        for(auto& comp : actor->components)
            actor->keys.push_back(comp.first);
        sort(actor->keys.begin(), actor->keys.end());
        
        Actor::actor_map[actor->actor_name] = actor;
        instance->pending_actors.push_back(actor);
        
        return actor;
    }
    
    static void Destroy(Actor* actor){
        for(auto& key : actor->keys){
            (*actor->components[key].componentRef)["enabled"] = false;
        }
        Actor::actor_map.erase(actor->actor_name);
        pending_destroy.push_back(actor);
    }
    
    static std::string next_scene;

    static void Load(const std::string& scene_name) {
        next_scene = scene_name;
    }

    static std::string GetCurrent() {
        return instance->current_scene_name;
    }

    static void DontDestroy(Actor* actor) {
        actor->dont_destroy = true;
    }
};

#endif
