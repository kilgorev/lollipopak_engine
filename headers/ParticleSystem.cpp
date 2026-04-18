//
//  ParticleSystem.cpp
//  game_engine grade
//
//  Created by Tori Kilgore on 3/24/26.
//

#include "ParticleSystem.h"
#include "render.h"

void ParticleSystem::OnStart() {
    angle_engine  = new RandomEngine(emit_angle_min, emit_angle_max, 298);
    radius_engine = new RandomEngine(emit_radius_min, emit_radius_max, 404);
    
    rotation_distribution = new RandomEngine(rotation_min, rotation_max, 440);
    scale_distribution = new RandomEngine(start_scale_min, start_scale_max, 494);
    
    speed_distribution = new RandomEngine(start_speed_min, start_speed_max, 498);
    rotation_speed_distribution = new RandomEngine(rotation_speed_min, rotation_speed_max, 305);

    particle_texture_name = key + "_particle";
    
    if (image.empty()) {
           particle_texture_name = key + "_particle";
           if (image_db && renderer)
               image_db->CreateDefaultParticleTextureWithName(particle_texture_name, renderer);
       } else {
           particle_texture_name = image;
       }
}

void ParticleSystem::OnUpdate() {
    if (!enabled) return;
    if (!angle_engine || !radius_engine) return;
    
    int interval = frames_between_bursts < 1 ? 1 : frames_between_bursts;
    if(burst_quantity < 1) {
        burst_quantity = 1;
    }
    
    int duration = duration_frames < 1 ? 1 : duration_frames;
    
    if (is_playing && local_frame % interval == 0) {
        for (int i = 0; i < burst_quantity; ++i) {
            
            float angle_deg = angle_engine->Sample();
            float radius    = radius_engine->Sample();
            float speed     = speed_distribution->Sample();
            float angle_rad = angle_deg * (static_cast<float>(M_PI) / 180.0f);
            
            Particle p;
           
            p.x  = x + radius * glm::cos(angle_rad);
            p.y  = y + radius * glm::sin(angle_rad);
            p.vx = speed * glm::cos(angle_rad);
            p.vy = speed * glm::sin(angle_rad);
            p.rotation_speed = rotation_speed_distribution->Sample();
            p.scale = scale_distribution ? scale_distribution->Sample() : 1.0f;
            p.start_scale = p.scale;
            p.frames_alive = 0;
            p.active = true;
            
            if (!free_list.empty()) {
                int idx = free_list.front();
                free_list.pop();
                particles[idx] = p;
            }
            else {
                particles.push_back(p);
            }
        }
    }
    ++local_frame;
    
    for (int i = 0; i < (int)particles.size(); ++i) {
        Particle& p = particles[i];
        if (!p.active) continue;
        
        if (p.frames_alive >= duration) {
            p.active = false;
            free_list.push(i);
            continue;
        }
        
        p.vx += gravity_scale_x;
        p.vy += gravity_scale_y;
        
        p.vx *= drag_factor;
        p.vy *= drag_factor;
        p.rotation_speed *= angular_drag_factor;
        
        p.x += p.vx;
        p.y += p.vy;
        p.rotation += p.rotation_speed;
        
        float lifetime_progress = (duration > 0) ? (float)p.frames_alive / (float)duration : 0.0f;
        
        float current_scale = p.start_scale;
        if (has_end_scale)
            current_scale = p.start_scale + (end_scale - p.start_scale) * lifetime_progress;
        
        float cur_r = start_color_r;
        float cur_g = start_color_g;
        float cur_b = start_color_b;
        float cur_a = start_color_a;
        if (has_end_color_r) cur_r = start_color_r + (end_color_r - start_color_r) * lifetime_progress;
        if (has_end_color_g) cur_g = start_color_g + (end_color_g - start_color_g) * lifetime_progress;
        if (has_end_color_b) cur_b = start_color_b + (end_color_b - start_color_b) * lifetime_progress;
        if (has_end_color_a) cur_a = start_color_a + (end_color_a - start_color_a) * lifetime_progress;
        
        ++p.frames_alive;
        
        float pixel_scale = current_scale / Camera::GetZoomFactor();
        ImageDB::DrawEx(particle_texture_name,
                        p.x, p.y,
                        p.rotation,
                        pixel_scale, pixel_scale,
                        0.5f, 0.5f,
                        cur_r, cur_g, cur_b, cur_a,
                        sorting_order);
    }
}

void ParticleSystem::Stop() { is_playing = false; }
void ParticleSystem::Play() { is_playing = true; }

void ParticleSystem::Burst() {
    for (int i = 0; i < burst_quantity; ++i) {
        float angle_deg = angle_engine->Sample();
        float radius    = radius_engine->Sample();
        float speed     = speed_distribution->Sample();
        float angle_rad = angle_deg * (static_cast<float>(M_PI) / 180.0f);

        Particle p;
        p.x  = x + radius * glm::cos(angle_rad);
        p.y  = y + radius * glm::sin(angle_rad);
        p.vx = speed * glm::cos(angle_rad);
        p.vy = speed * glm::sin(angle_rad);
        p.rotation = rotation_distribution->Sample();
        p.rotation_speed = rotation_speed_distribution->Sample();
        p.scale = scale_distribution ? scale_distribution->Sample() : 1.0f;
        p.start_scale = p.scale;
        p.frames_alive = 0;
        p.active = true;

        if (!free_list.empty()) {
            int idx = free_list.front();
            free_list.pop();
            particles[idx] = p;
        } else {
            particles.push_back(p);
        }
    }
}
