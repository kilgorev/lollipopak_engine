//
//  engine.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#include "engine.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
static Engine* g_engine = nullptr;
static void emscripten_loop() {
    g_engine->handleEvents();
    g_engine->update();
    g_engine->render();
}
#endif



/*
 
grade grade grade lin
 
*/

vector<Actor*> SceneDB::pending_destroy;
vector<Actor*>* Actor::actor_list = nullptr;
vector<Actor*>* Actor::pending_actors_list = nullptr;

unordered_map<string, Actor*> Actor::actor_map;

int component_counter = 0;
int SceneDB::n = 0;
std::string SceneDB::next_scene = "";

lua_State* Actor::componentManager_lua_state = nullptr;
SceneDB* SceneDB::instance = nullptr;
ComponentDB* SceneDB::componentDB = nullptr;
Camera* Camera::instance = nullptr;

using namespace std;

// constructors and destructors

Engine::Engine()
: window(nullptr), renderer(nullptr), width(640), height(360), clear_r(255), clear_g(255), clear_b(255) {}

Engine::~Engine(){
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);
}

// initialize

void Engine::initialize(){
    initLua();
    TTF_Init();
    AudioDB::Init();
    loadGameConfig();
    loadRenderConfig();
    loadScene(true);
    loadResources();

    cam.setWidth(640);
    cam.setHeight(360);
    
    Camera::instance = &cam;
    
    input.Init();
    initSDL();
    
    if (!audioHandler.intro_bgm.empty()) {
        AudioDB::PlayChannel(0, audioHandler.intro_bgm, true);
    }
    
    sceneHandler.loadActors(sceneJson, componentManager, renderer, current_scene);
    
    while(!componentManager.on_start_queue.empty()){
           Component *key = &componentManager.on_start_queue.front();
           componentManager.on_start_queue.pop();
           if(key->isEnabled()){
               if(key->rigidbody){
                   key->rigidbody->OnStart();
               }
               else if(key->particlesystem){
                   key->particlesystem->renderer = renderer;
                   key->particlesystem->image_db = &imageHandler;
                   key->particlesystem->OnStart();
               }

               else {
                   try {
                       (*key->componentRef)["OnStart"](*key->componentRef);
                   } catch (const luabridge::LuaException& e) {
                       componentManager.ReportError(key->actor_name, e);
                   }
               }
           }
       }
    
    cam.render_scene(sceneHandler, renderer, imageHandler, input, textHandler);
}

void Engine::loadScene(bool isInitScene){
    if(isInitScene){
        EngineUtils::validPath("resources/scenes/" + current_scene + ".scene", "error: initial_scene unspecified");
    }
    else{
        EngineUtils::validPath("resources/scenes/" + current_scene + ".scene", "error: scene " + current_scene + " is missing");
    }
    
    EngineUtils::ReadJsonFile("resources/scenes/" + current_scene + ".scene", sceneJson);
}

void Engine::loadGameConfig(){
    EngineUtils::validPath("resources/", "error: resources/ missing");
    EngineUtils::validPath("resources/game.config", "error: resources/game.config missing");
    
    // findAndPrint actually writes in gameConfigJson
    if(!EngineUtils::findAndPrintJson(gameConfigJson, "resources/game.config", "initial_scene", false)){
        cout << "error: initial_scene unspecified\n";
        exit(0);
    }
    
    // set the current_scene to initial_scene
    current_scene = gameConfigJson["initial_scene"].GetString();
    EngineUtils::validPath("resources/scenes/" + current_scene + ".scene", "error: scene " + current_scene + " is missing");
    
    game_title = gameConfigJson.HasMember("game_title") ? gameConfigJson["game_title"].GetString() : "";
    
}

void Engine::loadRenderConfig(){
    if(!filesystem::exists("resources/rendering.config")) return;
    
    EngineUtils::ReadJsonFile("resources/rendering.config", renderingConfigJson);
    
    // checking for all vars in the rendering.config
    if(renderingConfigJson.HasMember(("x_resolution")) && renderingConfigJson["x_resolution"].IsInt()){
        cam.setWidth(renderingConfigJson["x_resolution"].GetInt());
        width = cam.getWidth();
    }
    
    if(renderingConfigJson.HasMember(("y_resolution")) && renderingConfigJson["y_resolution"].IsInt()){
        cam.setHeight(renderingConfigJson["y_resolution"].GetInt());
        height = cam.getHeight();
    }
    
    if(renderingConfigJson.HasMember("clear_color_r") && renderingConfigJson["clear_color_r"].IsInt()){
        clear_r = renderingConfigJson["clear_color_r"].GetInt();
    }
    
    if(renderingConfigJson.HasMember("clear_color_g") && renderingConfigJson["clear_color_g"].IsInt()){
        clear_g = renderingConfigJson["clear_color_g"].GetInt();
    }
    
    if(renderingConfigJson.HasMember("clear_color_b") && renderingConfigJson["clear_color_b"].IsInt()){
        clear_b = renderingConfigJson["clear_color_b"].GetInt();
    }
    
    if(renderingConfigJson.HasMember("cam_offset_x") && (renderingConfigJson["cam_offset_x"].IsFloat() || renderingConfigJson["cam_offset_x"].IsInt())){
        cam.setOffsetX(renderingConfigJson["cam_offset_x"].GetFloat());
    }
    
    if(renderingConfigJson.HasMember("cam_offset_y") && (renderingConfigJson["cam_offset_y"].IsFloat() || renderingConfigJson["cam_offset_y"].IsInt())){
        cam.setOffsetY(renderingConfigJson["cam_offset_y"].GetFloat());
    }
    
    if(renderingConfigJson.HasMember("zoom_factor") &&
       (renderingConfigJson["zoom_factor"].IsFloat() || renderingConfigJson["zoom_factor"].IsInt())){
        cam.setZoomFactor(renderingConfigJson["zoom_factor"].GetFloat());
    }
    
    if(renderingConfigJson.HasMember("cam_ease_factor") &&
       (renderingConfigJson["cam_ease_factor"].IsFloat() || renderingConfigJson["cam_ease_factor"].IsInt())){
        cam.setEaseFactor(renderingConfigJson["cam_ease_factor"].GetFloat());
    }
    
}



void Engine::initLua(){
    componentManager.lua_state = luaL_newstate();
    luaL_openlibs(componentManager.lua_state);
    Actor::componentManager_lua_state = componentManager.lua_state;
    
    Physics::lua_state = componentManager.lua_state;
    
    luabridge::getGlobalNamespace(componentManager.lua_state)
        .beginNamespace("Debug")
            .addFunction("Log", ComponentDB::CppLog)
            .addFunction("LogError", ComponentDB::CppLogError)
        .endNamespace()
        .beginClass<glm::vec2>("vec2")
            .addProperty("x", &glm::vec2::x)
            .addProperty("y", &glm::vec2::y)
        .endClass()
        .beginNamespace("Actor")
            .addFunction("Find", &Actor::Find)
            .addFunction("FindAll", &Actor::FindAll)
            .addFunction("Instantiate", &SceneDB::Instantiate)
            .addFunction("Destroy", &SceneDB::Destroy)
        .endNamespace()
        .beginClass<Actor>("Actor_Class")
            .addFunction("GetName", &Actor::GetName)
            .addFunction("GetID", &Actor::GetID)
            .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
            .addFunction("GetComponent", &Actor::GetComponent)
            .addFunction("GetComponents", &Actor::GetComponents)
            .addFunction("AddComponent", &Actor::AddComponent)
            .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .endClass()
        .beginNamespace("Application")
            .addFunction("Quit", &StateManager::Quit)
            .addFunction("Sleep", &StateManager::Sleep)
            .addFunction("GetFrame", &StateManager::GetFrame)
            .addFunction("OpenUrl", &StateManager::OpenUrl)
        .endNamespace()
        .beginNamespace("Input")
            .addFunction("GetKey", &Input::GetKeyStr)
            .addFunction("GetKeyDown", &Input::GetKeyDownStr)
            .addFunction("GetKeyUp", &Input::GetKeyUpStr)
            .addFunction("GetMousePosition", &Input::GetMousePosition)
            .addFunction("GetMouseButton", &Input::GetMouseButton)
            .addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
            .addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
            .addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
            .addFunction("ShowCursor", &Input::ShowCursor)
            .addFunction("HideCursor", &Input::HideCursor)
        .endNamespace()
        .beginNamespace("Text")
            .addFunction("Draw", &TextDB::Draw)
        .endNamespace()
        .beginNamespace("Audio")
            .addFunction("Play", &AudioDB::PlayChannel)
            .addFunction("Halt", &AudioDB::HaltChannel)
            .addFunction("SetVolume", &AudioDB::SetVolume)
        .endNamespace()
        .beginNamespace("Image")
            .addFunction("DrawUI", &ImageDB::DrawUI)
            .addFunction("DrawUIEx", &ImageDB::DrawUIEx)
            .addFunction("Draw", &ImageDB::Draw)
            .addFunction("DrawEx", &ImageDB::DrawEx)
            .addFunction("DrawPixel", &ImageDB::DrawPixel)
        .endNamespace()
        .beginNamespace("Camera")
            .addFunction("SetPosition", &Camera::SetPosition)
            .addFunction("GetPositionX", &Camera::GetPositionX)
            .addFunction("GetPositionY", &Camera::GetPositionY)
            .addFunction("SetZoom", &Camera::SetZoom)
            .addFunction("GetZoom", &Camera::GetZoomFactor)
        .endNamespace()
        .beginNamespace("Scene")
            .addFunction("Load", &SceneDB::Load)
            .addFunction("GetCurrent", &SceneDB::GetCurrent)
            .addFunction("DontDestroy", &SceneDB::DontDestroy)
        .endNamespace()
        .beginClass<b2Vec2>("Vector2")
            .addConstructor<void(*) (float,float)>()
            .addProperty("x", &b2Vec2::x)
            .addProperty("y", &b2Vec2::y)
            .addFunction("Normalize", &b2Vec2::Normalize)
            .addFunction("Length", &b2Vec2::Length)
            .addFunction("__add", &b2Vec2::operator_add)
            .addFunction("__sub", &b2Vec2::operator_sub)
            .addFunction("__mul", &b2Vec2::operator_mul)
            .addStaticFunction("Distance", static_cast<float (*) (const b2Vec2&, const b2Vec2&)>(b2Distance))
            .addStaticFunction("Dot", static_cast<float (*) (const b2Vec2&, const b2Vec2&)>(b2Dot))
        .endClass()
        .beginClass<Rigidbody>("Rigidbody")
            .addConstructor<void(*) ()>()
            .addProperty("x", &Rigidbody::x)
            .addProperty("y", &Rigidbody::y)
            .addProperty("width", &Rigidbody::width)
            .addProperty("height", &Rigidbody::height)
            .addProperty("key", &Rigidbody::key)
            .addProperty("type", &Rigidbody::type)
            .addProperty("enabled", &Rigidbody::enabled)
            .addProperty("body_type", &Rigidbody::body_type)
            .addProperty("precise", &Rigidbody::precise)
            .addProperty("gravity_scale", &Rigidbody::gravity_scale)
            .addProperty("density", &Rigidbody::density)
            .addProperty("angular_friction", &Rigidbody::angular_friction)
            .addProperty("rotation", &Rigidbody::rotation)
            .addProperty("has_collider", &Rigidbody::has_collider)
            .addProperty("has_trigger", &Rigidbody::has_trigger)
            .addFunction("GetPosition", &Rigidbody::GetPosition)
            .addFunction("GetRotation", &Rigidbody::GetRotation)
            .addFunction("AddForce",&Rigidbody::AddForce)
            .addFunction("SetVelocity", &Rigidbody::SetVelocity)
            .addFunction("SetPosition", &Rigidbody::SetPosition)
            .addFunction("SetRotation", &Rigidbody::SetRotation)
            .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
            .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
            .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
            .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
            .addFunction("GetVelocity", &Rigidbody::GetVelocity)
            .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
            .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
            .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
            .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
            .addProperty("collider_type", &Rigidbody::collider_type)
            .addProperty("radius", &Rigidbody::radius)
            .addProperty("friction", &Rigidbody::friction)
            .addProperty("bounciness", &Rigidbody::bounciness)
            .addProperty("trigger_type", &Rigidbody::trigger_type)
            .addProperty("trigger_width", &Rigidbody::trigger_width)
            .addProperty("trigger_height", &Rigidbody::trigger_height)
            .addProperty("trigger_radius", &Rigidbody::trigger_radius)
        .endClass()
        .beginClass<Collision>("Collision")
            .addProperty("other", &Collision::other)
            .addProperty("point", &Collision::point)
            .addProperty("relative_velocity", &Collision::relative_velocity)
            .addProperty("normal", &Collision::normal)
        .endClass()
        .beginClass<HitResult>("HitResult")
            .addProperty("actor", &HitResult::actor)
            .addProperty("point", &HitResult::point)
            .addProperty("normal", &HitResult::normal)
            .addProperty("is_trigger", &HitResult::is_trigger)
        .endClass()
        .beginNamespace("Physics")
            .addFunction("Raycast", &Physics::Raycast)
            .addFunction("RaycastAll", &Physics::RaycastAll)
        .endNamespace()
        .beginNamespace("Event")
            .addFunction("Publish",     &EventBus::Publish)
            .addFunction("Subscribe",   &EventBus::Subscribe)
            .addFunction("Unsubscribe", &EventBus::Unsubscribe)
        .endNamespace()
        .beginClass<ParticleSystem>("ParticleSystem")
            .addProperty("key",     &ParticleSystem::key)
            .addProperty("type",    &ParticleSystem::type)
            .addProperty("enabled", &ParticleSystem::enabled)
            .addProperty("x",       &ParticleSystem::x)
            .addProperty("y",       &ParticleSystem::y)
            .addProperty("frames_between_bursts", &ParticleSystem::frames_between_bursts)
            .addProperty("burst_quantity", &ParticleSystem::burst_quantity)
            .addProperty("start_scale_min", &ParticleSystem::start_scale_min)
            .addProperty("start_scale_max", &ParticleSystem::start_scale_max)
            .addProperty("start_color_r", &ParticleSystem::start_color_r)
            .addProperty("start_color_g", &ParticleSystem::start_color_g)
            .addProperty("start_color_b", &ParticleSystem::start_color_b)
            .addProperty("start_color_a", &ParticleSystem::start_color_a)
            .addProperty("emit_radius_min", &ParticleSystem::emit_radius_min)
            .addProperty("emit_radius_max", &ParticleSystem::emit_radius_max)
            .addProperty("emit_angle_min", &ParticleSystem::emit_angle_min)
            .addProperty("emit_angle_max", &ParticleSystem::emit_angle_max)
            .addProperty("image", &ParticleSystem::image)
            .addProperty("sorting_order", &ParticleSystem::sorting_order)
            .addProperty("duration_frames", &ParticleSystem::duration_frames)
            .addProperty("start_speed_min", &ParticleSystem::start_speed_min)
            .addProperty("start_speed_max", &ParticleSystem::start_speed_max)
            .addProperty("rotation_speed_min", &ParticleSystem::rotation_speed_min)
            .addProperty("rotation_speed_max", &ParticleSystem::rotation_speed_max)
            .addProperty("gravity_scale_x", &ParticleSystem::gravity_scale_x)
            .addProperty("gravity_scale_y", &ParticleSystem::gravity_scale_y)
            .addProperty("drag_factor", &ParticleSystem::drag_factor)
            .addProperty("angular_drag_factor", &ParticleSystem::angular_drag_factor)
            .addProperty("end_scale", &ParticleSystem::end_scale)
            .addProperty("end_color_r", &ParticleSystem::end_color_r)
            .addProperty("end_color_g", &ParticleSystem::end_color_g)
            .addProperty("end_color_b", &ParticleSystem::end_color_b)
            .addProperty("end_color_a", &ParticleSystem::end_color_a)
            .addFunction("Stop", &ParticleSystem::Stop)
            .addFunction("Play", &ParticleSystem::Play)
            .addFunction("Burst", &ParticleSystem::Burst)
        .endClass()
    ;
}

// set up window and renderer
void Engine::initSDL(){
    window = Helper::SDL_CreateWindow(
                                      game_title.c_str(),
                                      100,
                                      100,
                                      width,
                                      height,
                                      SDL_WINDOW_SHOWN
                                      );
    
    renderer = Helper::SDL_CreateRenderer(window,
                                          -1,
                                          SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
                                          );
}

void Engine::loadResources(){
    // find images
    
    // find audios
    
    //find font
}

/*
LOOP
*/

void Engine::run() {
    #ifdef __EMSCRIPTEN__
        g_engine = this;
        emscripten_set_main_loop(emscripten_loop, 0, 1);
    #else
        while(isRunning()){
            handleEvents();
            update();
            render();
        }
    #endif
}

bool Engine::isRunning() const {
    return stateManager.isRunning();
}

/*
HANDLE EVENTS
*/

void Engine::handleEvents(){
    
    SDL_Event e;
    
    while(Helper::SDL_PollEvent(&e)){
        input.ProcessEvent(e);
        
        if(e.type == SDL_QUIT) {
            stateManager.setRunning(false);
            return;
        }
        
    }
    
}

/*
Update Logic
*/

void Engine::update(){
    if (!SceneDB::next_scene.empty()) {
        std::vector<Actor*> persistent;
        for (auto* a : sceneHandler.actors) {
            if (a->dont_destroy) persistent.push_back(a);
        }
        
        for (auto* a : sceneHandler.actors) {
            if (a->dont_destroy) continue;
            for (auto& key : a->keys) {
                auto it = a->components.find(key);
                if (it == a->components.end()) continue;
                auto& comp = it->second;
                if (comp.rigidbody) {
                    comp.rigidbody->OnDestroy();
                }
                else if (comp.componentRef) {
                    luabridge::LuaRef func = (*comp.componentRef)["OnDestroy"];
                    if (func.isFunction()) {
                        try { func(*comp.componentRef); }
                        catch (const luabridge::LuaException& e) {
                            componentManager.ReportError(comp.actor_name, e);
                        }
                    }
                }
            }
        }
        
        componentManager.update.clear();
        componentManager.late_update.clear();
        Actor::actor_map.clear();
        
        current_scene = SceneDB::next_scene;
        SceneDB::next_scene = "";
        loadScene(false);
        sceneHandler.loadActors(sceneJson, componentManager, renderer, current_scene);
        
        for (auto* a : persistent) {
            sceneHandler.actors.push_back(a);
            Actor::actor_map[a->actor_name] = a;
            for (auto& key : a->keys) {
                auto& comp = a->components[key];
                if (comp.hasUpdate) componentManager.update.push_back(comp);
                if (comp.hasLateUpdate) componentManager.late_update.push_back(comp);
            }
        }
        
        while (!componentManager.on_start_queue.empty()) {
            Component* c = &componentManager.on_start_queue.front();
            componentManager.on_start_queue.pop();
            if (c->isEnabled()) {
                if (c->rigidbody) {
                    c->rigidbody->OnStart();
                }
                else if(c->particlesystem){
                    c->particlesystem->renderer = renderer;
                    c->particlesystem->image_db = &imageHandler;
                    c->particlesystem->OnStart();
                }
                else {
                    try { (*c->componentRef)["OnStart"](*c->componentRef); }
                    catch (const luabridge::LuaException& e) {
                        componentManager.ReportError(c->actor_name, e);
                    }
                }
            }
        }
    }
    
    for (auto* a : sceneHandler.pending_actors) {
        sceneHandler.actors.push_back(a);
        Actor::actor_map[a->actor_name] = a;
        for (auto& key : a->keys) {
            auto& comp = a->components[key];
            comp.actor_name = a->actor_name;
            (*comp.componentRef)["actor"] = a;
            if (comp.rigidbody) comp.rigidbody->owner = a;
            if (comp.hasStart && comp.isEnabled()) {
                if (comp.rigidbody) {
                    comp.rigidbody->OnStart();
                }
                else if (comp.particlesystem) {
                    comp.particlesystem->renderer = renderer;
                    comp.particlesystem->image_db = &imageHandler;
                    comp.particlesystem->OnStart();
                }
                else {
                    try { (*comp.componentRef)["OnStart"](*comp.componentRef); }
                    catch (const luabridge::LuaException& e) {
                        componentManager.ReportError(comp.actor_name, e);
                    }
                }
            }
            if (comp.hasUpdate) componentManager.update.push_back(comp);
            if (comp.hasLateUpdate) componentManager.late_update.push_back(comp);
        }
    }
    sceneHandler.pending_actors.clear();
    
    

    for (auto& a : sceneHandler.actors) {
        for (auto& ref : a->pending_remove) {
            string key = ref["key"].cast<string>();
            auto it = a->components.find(key);
            if (it != a->components.end()) {
                auto& comp = it->second;
                if (comp.rigidbody) {
                    comp.rigidbody->OnDestroy();
                }
                else if (comp.componentRef) {
                    luabridge::LuaRef func = (*comp.componentRef)["OnDestroy"];
                    if (func.isFunction()) {
                        try { func(*comp.componentRef); }
                        catch (const luabridge::LuaException& e) {
                            componentManager.ReportError(comp.actor_name, e);
                        }
                    }
                }
            }
            a->components.erase(key);
            a->keys.erase(std::remove(a->keys.begin(), a->keys.end(), key), a->keys.end());
            auto& u = componentManager.update;
            u.erase(std::remove_if(u.begin(), u.end(), [&](Component& c){
                return (*c.componentRef)["key"].cast<string>() == key && c.actor_name == a->actor_name;
            }), u.end());
            auto& lu = componentManager.late_update;
            lu.erase(std::remove_if(lu.begin(), lu.end(), [&](Component& c){
                return (*c.componentRef)["key"].cast<string>() == key && c.actor_name == a->actor_name;
            }), lu.end());
        }
        a->pending_remove.clear();
        
        
        
        for (auto& comp : a->pending_components) {
            string key = (*comp.componentRef)["key"].cast<string>();
            a->components[key] = comp;
            a->keys.push_back(key);
            sort(a->keys.begin(), a->keys.end());
            if (comp.hasStart && comp.isEnabled()) {
                if (comp.rigidbody) {
                    comp.rigidbody->OnStart();
                }
                else if (comp.particlesystem) {
                    comp.particlesystem->renderer = renderer;
                    comp.particlesystem->image_db = &imageHandler;
                    comp.particlesystem->OnStart();
                }
                else {
                    try { (*comp.componentRef)["OnStart"](*comp.componentRef); }
                    catch (const luabridge::LuaException& e) {
                        componentManager.ReportError(comp.actor_name, e);
                    }
                }
            }
            if (comp.hasUpdate) componentManager.update.push_back(comp);
            if (comp.hasLateUpdate) componentManager.late_update.push_back(comp);
        }
        a->pending_components.clear();
    }

    // UPDATE
    int update_size = componentManager.update.size();
    for (int i = 0; i < update_size; ++i) {
        Component& key = componentManager.update[i];
        if (key.isEnabled()) {
            if (key.particlesystem) {
                key.particlesystem->OnUpdate();
            } else {
                try { (*key.componentRef)["OnUpdate"](*key.componentRef); }
                catch (const luabridge::LuaException& e) {
                    componentManager.ReportError(key.actor_name, e);
                }
            }
        }
    }

    // LATE UPDATE
    for (int i = 0; i < componentManager.late_update.size(); ++i) {
        Component* key = &componentManager.late_update[i];
        if (key->isEnabled()) {
            try { (*key->componentRef)["OnLateUpdate"](*key->componentRef); }
            catch (const luabridge::LuaException& e) {
                componentManager.ReportError(key->actor_name, e);
            }
        }
    }
    
    for (auto& a : sceneHandler.actors) {
        for (auto& ref : a->pending_remove) {
            string key = ref["key"].cast<string>();
            auto it = a->components.find(key);
            if (it != a->components.end()) {
                auto& comp = it->second;
                if (comp.rigidbody) {
                    comp.rigidbody->OnDestroy();
                } else if (comp.componentRef) {
                    luabridge::LuaRef func = (*comp.componentRef)["OnDestroy"];
                    if (func.isFunction()) {
                        try { func(*comp.componentRef); }
                        catch (const luabridge::LuaException& e) {
                            componentManager.ReportError(comp.actor_name, e);
                        }
                    }
                }
            }
            a->components.erase(key);
            a->keys.erase(std::remove(a->keys.begin(), a->keys.end(), key), a->keys.end());
            auto& u = componentManager.update;
            u.erase(std::remove_if(u.begin(), u.end(), [&](Component& c){
                return (*c.componentRef)["key"].cast<string>() == key && c.actor_name == a->actor_name;
            }), u.end());
            auto& lu = componentManager.late_update;
            lu.erase(std::remove_if(lu.begin(), lu.end(), [&](Component& c){
                return (*c.componentRef)["key"].cast<string>() == key && c.actor_name == a->actor_name;
            }), lu.end());
        }
        a->pending_remove.clear();
    }

    EventBus::ProcessPendingSubscriptions();


    Physics::Step();
}
    
/*
Rendering
*/
    
void Engine::render(){
    SDL_SetRenderDrawColor(renderer, clear_r, clear_g, clear_b, 255);
    SDL_RenderClear(renderer);
    
    cam.render_scene(sceneHandler, renderer, imageHandler, input, textHandler);
    
    Helper::SDL_RenderPresent(renderer);

    input.LateUpdate();
}

