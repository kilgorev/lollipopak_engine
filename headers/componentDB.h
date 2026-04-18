//
//  componentDB.h
//  game_engine
//
//  Created by Tori Kilgore on 2/25/26.
//
#ifndef componentDB_h
#define componentDB_h

#include <queue>
#include <string>
#include <unordered_map>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "component.h"


class ComponentDB {
public:
    std::unordered_map<std::string, shared_ptr<Component>> component_map;
    std::unordered_map<std::string, shared_ptr<Component>> component_files_map;
    
    lua_State * lua_state = nullptr;
    
    void establishInheritance(luabridge::LuaRef & instance_table, luabridge::LuaRef & parent_table);
    
    shared_ptr<Component> createBuiltin(const std::string& key,
                                                          const std::string& component_name);
    
    shared_ptr<Component> getComponent(const std::string &key, const std::string &component_name);
    
    void addComponent(const std::string &key, const std::string &component_name);
    
    static void CppLog(const std::string& message);
    static void CppLogError(const std::string& message);
    static void ReportError(const std::string & actor_name, const luabridge::LuaException &e);
    
    
    queue<Component> on_start_queue;
    vector<Component> update;
    vector<Component> late_update;
};

#endif
