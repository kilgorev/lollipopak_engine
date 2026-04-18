//
//  EventBus.cpp
//  game_engine
//
//  Created by Tori Kilgore on 3/23/26.
//

#include "EventBus.h"

std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> EventBus::subscribers;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> EventBus::pending_subscribes;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> EventBus::pending_unsubscribes;

void EventBus::Publish(const std::string& event_type, luabridge::LuaRef event_object) {
    auto it = subscribers.find(event_type);
    if (it == subscribers.end()) return;

    auto list = it->second;
    for (auto& [component, function] : list) {
        if (function.isFunction()) {
            try {
                function(component, event_object);
            } catch (const luabridge::LuaException& e) {
                // silent
            }
        }
    }
}

void EventBus::Subscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_subscribes.push_back({ event_type, component, function });
}

void EventBus::Unsubscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_unsubscribes.push_back({ event_type, component, function });
}

void EventBus::ProcessPendingSubscriptions() {
    for (auto& [event_type, component, function] : pending_unsubscribes) {
        auto it = subscribers.find(event_type);
        if (it == subscribers.end()) continue;
        auto& list = it->second;
        for (auto lit = list.begin(); lit != list.end(); ) {
            if (lit->first == component && lit->second == function) {
                lit = list.erase(lit);
            } else {
                ++lit;
            }
        }
    }
    pending_unsubscribes.clear();

    for (auto& [event_type, component, function] : pending_subscribes) {
        subscribers[event_type].push_back({ component, function });
    }
    pending_subscribes.clear();
}
