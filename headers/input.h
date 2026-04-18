//
//  input.h
//  game_engine
//
//  Created by Tori Kilgore on 1/19/26.
//

#ifndef INPUT_H
#define INPUT_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include "SDL.h"
#include "glm/glm/glm.hpp"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"


using namespace std;

enum INPUT_STATE { INPUT_STATE_UP, INPUT_STATE_JUST_BECAME_DOWN, INPUT_STATE_DOWN, INPUT_STATE_JUST_BECAME_UP };

class Input
{
public:
    static void Init(); // Call before main loop begins.
    static void ProcessEvent(const SDL_Event & e); // Call every frame at start of event loop.
    static void LateUpdate();

    static bool GetKey(SDL_Scancode keycode);
    static bool GetKeyDown(SDL_Scancode keycode);
    static bool GetKeyUp(SDL_Scancode keycode);
    
    static glm::vec2 GetMousePosition();
    
    static bool GetMouseButton(int button);
    static bool GetMouseButtonDown(int button);
    static bool GetMouseButtonUp(int button);
    static float GetMouseScrollDelta();
    
    static void HideCursor();
    static void ShowCursor();
    
    // input for lua
    static bool GetKeyStr(const std::string& keycode) {
        auto it = special_keys.find(keycode);
        if (it != special_keys.end())
            return GetKey(it->second);
        
        return GetKey(SDL_GetScancodeFromName(keycode.c_str()));
    }
    static bool GetKeyDownStr(const std::string& keycode) {
        auto it = special_keys.find(keycode);
        if (it != special_keys.end())
            return GetKeyDown(it->second);
        return GetKeyDown(SDL_GetScancodeFromName(keycode.c_str()));
    }
    static bool GetKeyUpStr(const std::string& keycode) {
        auto it = special_keys.find(keycode);
        if (it != special_keys.end())
            return GetKeyUp(it->second);
        return GetKeyUp(SDL_GetScancodeFromName(keycode.c_str()));
    }

private:
    static inline const unordered_map<string, SDL_Scancode> special_keys = {
        {"up", SDL_SCANCODE_UP},
        {"down", SDL_SCANCODE_DOWN},
        {"left", SDL_SCANCODE_LEFT},
        {"right", SDL_SCANCODE_RIGHT},
        {"space", SDL_SCANCODE_SPACE},
        {"return", SDL_SCANCODE_RETURN},
        {"escape", SDL_SCANCODE_ESCAPE},
        {"lshift", SDL_SCANCODE_LSHIFT},
        {"rshift", SDL_SCANCODE_RSHIFT},
        {"lctrl", SDL_SCANCODE_LCTRL},
        {"rctrl", SDL_SCANCODE_RCTRL},
        {"lalt", SDL_SCANCODE_LALT},
        {"ralt", SDL_SCANCODE_RALT},
        {"tab", SDL_SCANCODE_TAB},
        {"backspace", SDL_SCANCODE_BACKSPACE},
        {"enter", SDL_SCANCODE_RETURN},
        {"delete", SDL_SCANCODE_DELETE},
        {"insert", SDL_SCANCODE_INSERT},
        {"a", SDL_SCANCODE_A},
        {"b", SDL_SCANCODE_B},
        {"c", SDL_SCANCODE_C},
        {"d", SDL_SCANCODE_D},
        {"e", SDL_SCANCODE_E},
        {"f", SDL_SCANCODE_F},
        {"g", SDL_SCANCODE_G},
        {"h", SDL_SCANCODE_H},
        {"i", SDL_SCANCODE_I},
        {"j", SDL_SCANCODE_J},
        {"k", SDL_SCANCODE_K},
        {"l", SDL_SCANCODE_L},
        {"m", SDL_SCANCODE_M},
        {"n", SDL_SCANCODE_N},
        {"o", SDL_SCANCODE_O},
        {"p", SDL_SCANCODE_P},
        {"q", SDL_SCANCODE_Q},
        {"r", SDL_SCANCODE_R},
        {"s", SDL_SCANCODE_S},
        {"t", SDL_SCANCODE_T},
        {"u", SDL_SCANCODE_U},
        {"v", SDL_SCANCODE_V},
        {"w", SDL_SCANCODE_W},
        {"x", SDL_SCANCODE_X},
        {"y", SDL_SCANCODE_Y},
        {"z", SDL_SCANCODE_Z},
        {"0", SDL_SCANCODE_0},
        {"1", SDL_SCANCODE_1},
        {"2", SDL_SCANCODE_2},
        {"3", SDL_SCANCODE_3},
        {"4", SDL_SCANCODE_4},
        {"5", SDL_SCANCODE_5},
        {"6", SDL_SCANCODE_6},
        {"7", SDL_SCANCODE_7},
        {"8", SDL_SCANCODE_8},
        {"9", SDL_SCANCODE_9},
        {"/", SDL_SCANCODE_SLASH},
        {";", SDL_SCANCODE_SEMICOLON},
        {"=", SDL_SCANCODE_EQUALS},
        {"-", SDL_SCANCODE_MINUS},
        {".", SDL_SCANCODE_PERIOD},
        {",", SDL_SCANCODE_COMMA},
        {"[", SDL_SCANCODE_LEFTBRACKET},
        {"]", SDL_SCANCODE_RIGHTBRACKET},
        {"\\", SDL_SCANCODE_BACKSLASH},
        {"'", SDL_SCANCODE_APOSTROPHE}
    };
    
    static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
    static inline std::vector<SDL_Scancode> just_became_down_scancodes;
    static inline std::vector<SDL_Scancode> just_became_up_scancodes;
    
    static inline glm::vec2 mouse_position;
    static inline std::unordered_map<int, INPUT_STATE> mouse_button_states;
    static inline std::vector<int> just_became_down_buttons;
    static inline std::vector<int> just_became_up_buttons;
    
    static inline float mouse_scroll_this_frame = 0;

};

#endif /* input_h */
