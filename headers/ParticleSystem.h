//
//  ParticleSystem.h
//  game_engine
//
//  Created by Tori Kilgore on 3/24/26.
//

#ifndef ParticleSystem_h
#define ParticleSystem_h

#include <iostream>
#include "SDL.h"
#include "Helper.h"
#include "imageDB.h"

class Camera;

struct Actor;

struct Particle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
    float scale = 1.0f;
    float start_scale = 1.0f;
    int frames_alive = 0;
    bool active = false;
};

class ParticleSystem {
public:
    Actor* owner = nullptr;
    
    std::string key = "ParticleSystem";
    std::string type = "ParticleSystem";
    bool enabled = true;
    
    float emit_angle_min  = 0.0f;
    float emit_angle_max  = 360.0f;
    float emit_radius_min = 0.0f;
    float emit_radius_max = 0.5f;

    
    float start_scale_min = 1.0f;
    float start_scale_max = 1.0f;
    float rotation_min = 0.0f;
    float rotation_max = 0.0f;
    float start_speed_min = 0.0f;
    float start_speed_max = 0.0f;
    float rotation_speed_min = 0.0f;
    float rotation_speed_max = 0.0f;
    float gravity_scale_x = 0.0f;
    float gravity_scale_y = 0.0f;
    float drag_factor = 1.0f;
    
    float angular_drag_factor = 1.0f;

    bool has_end_scale = false;
    float end_scale = 1.0f;

    bool has_end_color_r = false;
    bool has_end_color_g = false;
    bool has_end_color_b = false;
    bool has_end_color_a = false;
    int end_color_r = 255;
    int end_color_g = 255;
    int end_color_b = 255;
    int end_color_a = 255;

    float scale_x = 1.0f;
    float scale_y = 1.0f;
    int   start_color_r = 255,start_color_g = 255, start_color_b = 255, start_color_a = 255;
    
    float x, y = 0.0f;
    
    bool is_playing = true;
    
    int   sorting_order = 9999;

    int burst_quantity  = 1;
    int burst_interval  = 1;
    
    int frames_between_bursts = 1;
    
    SDL_Renderer* renderer    = nullptr;
    SDL_Texture*  texture     = nullptr;
    static const int PARTICLE_SIZE = 8;
    
    RandomEngine* angle_engine  = nullptr;
    RandomEngine* radius_engine = nullptr;
    RandomEngine*  rotation_distribution = nullptr;
    RandomEngine* scale_distribution = nullptr;
    RandomEngine* speed_distribution = nullptr;
    RandomEngine* rotation_speed_distribution = nullptr;


    ImageDB* image_db = nullptr;
    
    std::string image = "";

    std::string particle_texture_name = "";
    
    int local_frame = 0;
    
    int duration_frames = 300;
    
    std::queue<int> free_list;
    std::vector<Particle> particles;

  
    void OnStart();
    void OnUpdate();
    void Stop();
    void Play();
    void Burst();

};

#endif
