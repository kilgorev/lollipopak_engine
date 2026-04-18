//
//  engine.h
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#ifndef engine_h
#define engine_h

#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <queue>


#include "rapidjson/document.h"
#include "glm/glm/glm.hpp"
#include "sceneDB.h"
#include "input.h"
#include "render.h"
#include "engineUtils.h"
#include "Helper.h"
#include "AudioHelper.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "imageDB.h"
#include "textDB.h"
#include "audioDB.h"
#include "dialogue.h"
#include "stateManager.h"
#include "component.h"
#include "componentDB.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "box2d/box2d.h"
#include "Physics.h"
#include "Rigidbody.h"
#include "ContactListener.h"
#include "Collision.h"
#include "EventBus.h"


class Engine {
private:
    // core
    ComponentDB componentManager;
    Camera cam;
    DialogueSystem dialogueSystem;
    ImageDB imageHandler;
    TextDB textHandler;
    AudioDB audioHandler;
    SceneDB sceneHandler;
    StateManager stateManager;
    Input input;
    // json
    rapidjson::Document gameConfigJson;
    rapidjson::Document renderingConfigJson;
    rapidjson::Document sceneJson;
    
    // sdl
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    // loading everything
    void loadGameConfig();
    void loadRenderConfig();
    void loadResources();
    void loadScene(bool isInitScene);
    void initSDL();
    void initLua();
    void checkPlayerReqs();
    void moveNPC(float dx, float dy);
    
    std::string game_title;
    std::string current_scene;

    int width;
    int height;
    int clear_r, clear_g, clear_b;
    
    float move_speed = .02f;
    
    bool running;
    bool intro_was_running = false;
    bool gameplay_started = false;
    
public:
    Engine();
    ~Engine();
    
    // initialize everything
    void initialize();
    
    // game loop
    void run();
    
    // loop phases
    void handleEvents();
    void update();
    void render();
    bool isRunning() const;
    
};


#endif /* engine_h */
