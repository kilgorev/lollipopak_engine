//
//  render.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#include <stdio.h>
#include "render.h"

const int& Camera::getWidth(){return CAM_WIDTH;}
const int& Camera::getHeight(){return CAM_HEIGHT;}
const bool& Camera::isActorFlipX(){return x_actor_flip;}

void Camera::setWidth(const int &width){CAM_WIDTH = width;}
void Camera::setHeight(const int &height){CAM_HEIGHT = height;}

void Camera::setOffsetX(const float &offx){cam_x_offset = offx;}
void Camera::setOffsetY(const float &offy){cam_y_offset = offy;}

void Camera::setEaseFactor(const float &easeFactor){ease_factor = easeFactor;}
void Camera::setZoomFactor(const float &zoomfactor){zoom_factor = zoomfactor;}
void Camera::setActorFlipX(const bool &actorFlip){x_actor_flip = actorFlip;}

void Camera::updatePosition(const glm::vec2& focus_world_pos, SceneDB &scene) {
    glm::vec2 target_pos = glm::vec2(
        focus_world_pos.x + cam_x_offset,
        focus_world_pos.y + cam_y_offset
    );
    
    if (cam_world_x == 0.0f && cam_world_y == 0.0f ) {
            cam_world_x = target_pos.x;
            cam_world_y = target_pos.y;
            return;
        }
    
    glm::vec2 current_pos = glm::vec2(cam_world_x, cam_world_y);
    
    glm::vec2 new_pos;
    
    new_pos = glm::mix(current_pos, target_pos, ease_factor);

    cam_world_x = new_pos.x;
    cam_world_y = new_pos.y;
}

glm::vec2 Camera::worldToScreen(const glm::vec2& world_pos) const {
    float screen_cx = (CAM_WIDTH/zoom_factor) * 0.5f;
    float screen_cy = (CAM_HEIGHT/zoom_factor) * 0.5f;
    
    float screen_x = screen_cx + (world_pos.x - cam_world_x) * PIXEL;
    float screen_y = screen_cy + (world_pos.y - cam_world_y) * PIXEL;
    
    return glm::vec2(screen_x, screen_y);
}

void Camera::render_scene(SceneDB& scene, SDL_Renderer* renderer, ImageDB& image_handler, Input& input, TextDB& text_handler){
    
    if (!manually_positioned){
        updatePosition(glm::vec2(0.0f, 0.0f), scene);
    }
    
    SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);
    
    image_handler.RenderAndClearAllImages(renderer, zoom_factor, glm::vec2(cam_world_x, cam_world_y), glm::ivec2(CAM_WIDTH, CAM_HEIGHT));
    
    image_handler.RenderAndClearUIImages(renderer);
    
    text_handler.renderText(renderer);
    
    
    image_handler.RenderAndClearPixels(renderer);
}
