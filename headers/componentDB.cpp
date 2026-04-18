//
//  componentDB.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/25/26.
//
#include <iostream>
#include "engineUtils.h"
#include "componentDB.h"
#include "Rigidbody.h"
#include "ParticleSystem.h"
#include "Physics.h"

void ComponentDB::establishInheritance(luabridge::LuaRef & instance_table, luabridge::LuaRef & parent_table){
    luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
    new_metatable["__index"] = parent_table;
    
    instance_table.push(lua_state);
    new_metatable.push(lua_state);
    lua_setmetatable(lua_state, -2);
    lua_pop(lua_state, 1);
}

std::shared_ptr<Component> ComponentDB::createBuiltin(const std::string& key,
                                                       const std::string& component_name) {
    if (component_name == "Rigidbody") {
        Physics::Init();
        Rigidbody* rb = new Rigidbody();
        rb->key = key;
        auto instance = std::make_shared<luabridge::LuaRef>(luabridge::newTable(lua_state));
        (*instance)["key"]     = key;
        (*instance)["type"]    = component_name;
        (*instance)["enabled"] = true;
        (*instance)["rb"]      = rb;
        auto component = std::make_shared<Component>();
        component->type          = component_name;
        component->componentRef  = instance;
        component->rigidbody     = rb;
        component->hasStart      = true;
        component->hasUpdate     = false;
        component->hasLateUpdate = false;
        return component;
    }

    if (component_name == "ParticleSystem") {
        ParticleSystem* ps = new ParticleSystem();
        ps->key = key;
        auto instance = std::make_shared<luabridge::LuaRef>(luabridge::newTable(lua_state));
        (*instance)["key"]     = key;
        (*instance)["type"]    = component_name;
        (*instance)["enabled"] = true;
        (*instance)["ps"]      = ps;
        auto component = std::make_shared<Component>();
        component->type          = component_name;
        component->componentRef  = instance;
        component->particlesystem = ps;
        component->hasStart      = true;
        component->hasUpdate     = true;
        component->hasLateUpdate = false;
        return component;
    }

    return nullptr;
}

shared_ptr<Component> ComponentDB::getComponent(const std::string& key, const std::string& component_name) {
    auto builtin = createBuiltin(key, component_name);
    if (builtin) return builtin;
 
    auto it = component_map.find(component_name);
    if (it == component_map.end()) {
        addComponent(key, component_name);
    }
    return component_map[component_name];
}

void ComponentDB::addComponent(const std::string &key, const std::string &component_name){
    std::string path = "resources/component_types/" + component_name + ".lua";
    EngineUtils::validPath(path, "error: failed to locate component " + component_name);
    
    if(luaL_dofile(lua_state, path.c_str()) != LUA_OK){
        cout << "problem with lua file " << filesystem::path(path).stem().string();
        exit(0);
    }
    
    luabridge::LuaRef parent_table = luabridge::getGlobal(lua_state, component_name.c_str());
    auto instance = std::make_shared<luabridge::LuaRef>(luabridge::newTable(lua_state));
    
    establishInheritance(*instance, parent_table);
    
    (*instance)["key"] = key;
    
    auto component = std::make_shared<Component>();
    component->type = component_name;
    component->componentRef = instance;
    component->hasStart = parent_table["OnStart"].isFunction();
    component->hasUpdate = parent_table["OnUpdate"].isFunction();
    component->hasLateUpdate = parent_table["OnLateUpdate"].isFunction();

    
    // here i should override some values
    // open the json and find them
    // or is the json already open?
    
    component_map[component_name] = component;
}

void ComponentDB::CppLog(const std::string& message){
    cout << message << endl;
}

void ComponentDB::CppLogError(const std::string& message){
    cout << message << endl;
}

void ComponentDB::ReportError(const std::string & actor_name, const luabridge::LuaException &e){
    std::string error_message = e.what();
    
    std::replace(error_message.begin(), error_message.end(), '\\', '/');
    
    std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
}
