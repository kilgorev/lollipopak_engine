//
//  render.h
//  game_engine
//
//  Created by Tori Kilgore on 1/20/26.
//

#ifndef RENDER_H
#define RENDER_H

#include <iostream>
#include <sstream>
#include "sceneDB.h"
#include "SDL.h"
#include "Helper.h"
#include "imageDB.h"
#include "textDB.h"
#include "glm/glm/glm.hpp"
#include "input.h"

using namespace std;

class Camera {
private:
    int CAM_WIDTH;
    int CAM_HEIGHT;
    
    float cam_x_offset = 0.0f;
    float cam_y_offset = 0.0f;
    
    float cam_world_x = 0.0f;
    float cam_world_y = 0.0f;
    
    float ease_factor = 1.0f;
    float zoom_factor = 1.0f;
    
    bool x_actor_flip = false;
    
    const float PIXEL = 100.0f;
    
    
    
public:
    bool debug_mode = false;
    
    const int& getWidth();
    const int& getHeight();
    const bool& isActorFlipX();
    void setWidth(const int &width);
    void setHeight(const int &height);
    void setOffsetX(const float &offx);
    void setOffsetY(const float &offy);
    void setZoomFactor(const float &zoom_factor);
    void setEaseFactor(const float &easeFactor);
    void setActorFlipX(const bool &actorFlip);
    
    
    static Camera* instance;

    bool manually_positioned = false;

    static void SetPosition(float x, float y) {
        instance->cam_world_x = x;
        instance->cam_world_y = y;
        instance->manually_positioned = true;
    }

    static float GetPositionX() { return instance->cam_world_x; }
    static float GetPositionY() { return instance->cam_world_y; }
    static float GetZoomFactor() { return instance->zoom_factor; }
    static void SetZoom(float zoom) { instance->zoom_factor = zoom; }
    
    void debug(SDL_Renderer *renderer, Actor* a, float collider_w, float collider_h, Uint8 r, Uint8 g, Uint8 b);
    
    void updatePosition(const glm::vec2& focus_world_pos, SceneDB &scene);
    glm::vec2 worldToScreen(const glm::vec2& world_pos) const;
    
    void renderImage(SceneDB &scene, SDL_Renderer *renderer, SDL_Texture *texture, int amount);
    void render_scene(SceneDB& scene, SDL_Renderer *renderer, ImageDB& image_handler, Input &input, TextDB& text_handler);
};
#endif /* render_h */
