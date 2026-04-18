//
//  input.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#include <stdio.h>
#include "input.h"

// cleaning all input states
void Input::Init(){
    keyboard_states.clear();
    just_became_up_scancodes.clear();
    just_became_down_scancodes.clear();
    mouse_button_states.clear();
    just_became_up_buttons.clear();
    just_became_down_buttons.clear();
}

// deciding the input states based off of the polling event
void Input::ProcessEvent(const SDL_Event & e){
    if (e.type == SDL_KEYDOWN) {
        SDL_Scancode scancode = e.key.keysym.scancode;
        
        auto it = keyboard_states.find(scancode);
        if (it == keyboard_states.end() || it->second != INPUT_STATE_DOWN) {
            keyboard_states[scancode] = INPUT_STATE_JUST_BECAME_DOWN;
            just_became_down_scancodes.push_back(scancode);
        }
    }
    
    if (e.type == SDL_KEYUP) {
        SDL_Scancode scancode = e.key.keysym.scancode;
        keyboard_states[scancode] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_scancodes.push_back(scancode);
    }
    
    if (e.type == SDL_MOUSEMOTION) {
        mouse_position.x = e.motion.x;
        mouse_position.y = e.motion.y;
    }
    
    if(e.type == SDL_MOUSEBUTTONDOWN) {
        int button = e.button.button;
        auto it = mouse_button_states.find(button);
        if(it == mouse_button_states.end() || it->second != INPUT_STATE_DOWN) {
            mouse_button_states[button] = INPUT_STATE_JUST_BECAME_DOWN;
            just_became_down_buttons.push_back(button);
        }
    }
    
    if(e.type == SDL_MOUSEBUTTONUP) {
        int button = e.button.button;
        auto it = mouse_button_states.find(button);
        if(it == mouse_button_states.end() || it->second != INPUT_STATE_UP) {
            mouse_button_states[button] = INPUT_STATE_JUST_BECAME_UP;
            just_became_up_buttons.push_back(button);
        }
    }
    
    if(e.type == SDL_MOUSEWHEEL){
        mouse_scroll_this_frame += e.wheel.preciseY; // += or = idk
    }
    else {
        mouse_scroll_this_frame = 0.0f;
    }
}


// call at end of frame
void Input::LateUpdate(){
    for(SDL_Scancode sc : just_became_down_scancodes)
        keyboard_states[sc] = INPUT_STATE_DOWN;
    just_became_down_scancodes.clear();

    for(SDL_Scancode sc : just_became_up_scancodes)
        keyboard_states[sc] = INPUT_STATE_UP;
    just_became_up_scancodes.clear();
    
    for(int b : just_became_down_buttons)
        mouse_button_states[b] = INPUT_STATE_DOWN;
    just_became_down_buttons.clear();
    
    for(int b : just_became_up_buttons)
        mouse_button_states[b] = INPUT_STATE_UP;
    just_became_up_buttons.clear();
    
    mouse_scroll_this_frame = 0.0f;
}

bool Input::GetKey(SDL_Scancode keycode){
    auto it = keyboard_states.find(keycode);
    if(it == keyboard_states.end()){
        return false;
    }
    return (it->second == INPUT_STATE_DOWN || it->second == INPUT_STATE_JUST_BECAME_DOWN);
}

bool Input::GetKeyDown(SDL_Scancode keycode){
    auto it = keyboard_states.find(keycode);
    if(it == keyboard_states.end()){
        return false;
    }
    return it->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyUp(SDL_Scancode keycode){
    auto it = keyboard_states.find(keycode);
    if(it == keyboard_states.end()){
        return false;
    }
    return it->second == INPUT_STATE_JUST_BECAME_UP;
}

glm::vec2 Input::GetMousePosition(){
    return mouse_position;
}

bool Input::GetMouseButton(int button){
    auto it = mouse_button_states.find(button);
    if(it == mouse_button_states.end()){
        return false;
    }
    return (it->second == INPUT_STATE_DOWN || it->second == INPUT_STATE_JUST_BECAME_DOWN);
}


bool Input::GetMouseButtonDown(int button){
    auto it = mouse_button_states.find(button);
    if(it == mouse_button_states.end()){
        return false;
    }
    return it->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonUp(int button){
    auto it = mouse_button_states.find(button);
    if(it == mouse_button_states.end()){
        return false;
    }
    return it->second == INPUT_STATE_JUST_BECAME_UP;
}

float Input::GetMouseScrollDelta(){
    return mouse_scroll_this_frame;
}

void Input::ShowCursor(){
    SDL_ShowCursor(SDL_ENABLE);
}

void Input::HideCursor(){
    SDL_ShowCursor(SDL_DISABLE);
}
