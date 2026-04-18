//
//  EventBus.h
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//

#ifndef eventbus_h
#define eventbus_h

#include <string>
#include <unordered_map>
#include <vector>
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

class EventBus {
public:
    static void Publish(const std::string& event_type, luabridge::LuaRef event_object);
    static void Subscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void Unsubscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function);

    static void ProcessPendingSubscriptions();

private:
    static std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> subscribers;

    static std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> pending_subscribes;
    static std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> pending_unsubscribes;
};

#endif /* eventbus_h */
