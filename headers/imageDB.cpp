//
//  imageDB.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/5/26.
//

#include "imageDB.h"
#include <iostream>

std::vector<ImageDB::ImageDrawRequest> ImageDB::image_draw_request_queue;
std::vector<ImageDB::ImageDrawRequest> ImageDB::ui_draw_request_queue;
std::vector<ImageDB::PixelDrawRequest> ImageDB::pixel_draw_request_queue;

// this one has a lot of issues
// TODO: fix me
void ImageDB::findImages(rapidjson::Document &image, bool array, const char * member) {
    if(array){
        if(image.HasMember(member) && !image[member].Empty()){
            const auto introImageArray = image[member].GetArray();
            
            for(int i = 0; i < introImageArray.Size(); ++i){
                
                string img_name = introImageArray[i].GetString();
                
                EngineUtils::validPath("resources/images/" + img_name + ".png", "error: missing image " + img_name);
                
            }
            
        }
    }
    else {
        if(image.HasMember(member)){
            if(image[member].IsString()){
                string img_name = image[member].GetString();
                
                EngineUtils::validPath("resources/images/" + img_name + ".png", "error: missing image " + img_name);
                
                images[member] = img_name;
            }
        }
    }
}

SDL_Texture* ImageDB::getTexture(SDL_Renderer* renderer, const std::string& image_name) {
    if (texture_cache.find(image_name) == texture_cache.end()) {
        std::string path = "resources/images/" + image_name + ".png";
        texture_cache[image_name] = IMG_LoadTexture(renderer, path.c_str());
    }
    return texture_cache[image_name];
}

bool ImageDB::compare_image_requests(const ImageDrawRequest& a, const ImageDrawRequest& b) {
    return a.sorting_order < b.sorting_order;
}

void ImageDB::RenderAndClearAllImages(SDL_Renderer* renderer, float zoom_factor, glm::vec2 cam_pos, glm::ivec2 cam_dimensions) {
    std::stable_sort(image_draw_request_queue.begin(), image_draw_request_queue.end(), compare_image_requests);

    SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);

    for (auto& request : image_draw_request_queue) {
        const int pixels_per_meter = 100;
        glm::vec2 final_pos = glm::vec2(request.x, request.y) - cam_pos;

        SDL_Texture* tex = getTexture(renderer, request.image_name);
        float w = 0, h = 0;
        Helper::SDL_QueryTexture(tex, &w, &h);

        int flip_mode = SDL_FLIP_NONE;
        if (request.scale_x < 0) flip_mode |= SDL_FLIP_HORIZONTAL;
        if (request.scale_y < 0) flip_mode |= SDL_FLIP_VERTICAL;

        w *= glm::abs(request.scale_x);
        h *= glm::abs(request.scale_y);

        SDL_FPoint pivot_point = {
            request.pivot_x * w,
            request.pivot_y * h
        };

        SDL_FRect tex_rect = {
            final_pos.x * pixels_per_meter + cam_dimensions.x * 0.5f * (1.0f / zoom_factor) - pivot_point.x,
            final_pos.y * pixels_per_meter + cam_dimensions.y * 0.5f * (1.0f / zoom_factor) - pivot_point.y,
            w, h
        };

        SDL_SetTextureColorMod(tex, request.r, request.g, request.b);
        SDL_SetTextureAlphaMod(tex, request.a);
        
        Helper::SDL_RenderCopyEx(0, "", renderer, tex, NULL, &tex_rect, request.rotation_degrees, &pivot_point, static_cast<SDL_RendererFlip>(flip_mode));

        SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);

        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, 255);
    }

    SDL_RenderSetScale(renderer, 1, 1);
    image_draw_request_queue.clear();
}

void ImageDB::RenderAndClearUIImages(SDL_Renderer* renderer) {
    std::stable_sort(ui_draw_request_queue.begin(), ui_draw_request_queue.end(), compare_image_requests);

    SDL_RenderSetScale(renderer, 1, 1);

    for (auto& request : ui_draw_request_queue) {
        SDL_Texture* tex = getTexture(renderer, request.image_name);
        float w = 0, h = 0;
        Helper::SDL_QueryTexture(tex, &w, &h);

        w *= glm::abs(request.scale_x);
        h *= glm::abs(request.scale_y);

        int flip_mode = SDL_FLIP_NONE;
        if (request.scale_x < 0) flip_mode |= SDL_FLIP_HORIZONTAL;
        if (request.scale_y < 0) flip_mode |= SDL_FLIP_VERTICAL;

        SDL_FPoint pivot_point = {
            request.pivot_x * w,
            request.pivot_y * h
        };

        SDL_FRect dst = {
            request.x - pivot_point.x,
            request.y - pivot_point.y,
            w, h
        };

        SDL_SetTextureColorMod(tex, request.r, request.g, request.b);
        SDL_SetTextureAlphaMod(tex, request.a);

        Helper::SDL_RenderCopyEx(0, "", renderer, tex, NULL, &dst, request.rotation_degrees, &pivot_point, static_cast<SDL_RendererFlip>(flip_mode));

        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, 255);
    }

    ui_draw_request_queue.clear();
}

void ImageDB::DrawUIEx(const std::string& image_name, float x, float y, float r, float g, float b, float a, float sorting_order) {
    ui_draw_request_queue.push_back({image_name, x, y, 0, 1.0f, 1.0f, 0.0f, 0.0f, (int)r, (int)g, (int)b, (int)a, (int)sorting_order});
}

void ImageDB::DrawUI(const std::string& image_name, float x, float y) {
    ui_draw_request_queue.push_back({image_name, x, y, 0, 1.0f, 1.0f, 0.0f, 0.0f, 255, 255, 255, 255, 0});
}

void ImageDB::DrawEx(const std::string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order) {
    image_draw_request_queue.push_back({image_name, x, y, (int)rotation_degrees, scale_x, scale_y, pivot_x, pivot_y, (int)r, (int)g, (int)b, (int)a, (int)sorting_order});
}

void ImageDB::Draw(const std::string& image_name, float x, float y) {
    image_draw_request_queue.push_back({image_name, x, y, 0, 1.0f, 1.0f, 0.5f, 0.5f, 255, 255, 255, 255, 0});
}

void ImageDB::DrawPixel(float x, float y, float r, float g, float b, float a) {
    pixel_draw_request_queue.push_back({(int)x, (int)y, (int)r, (int)g, (int)b, (int)a});
}

void ImageDB::RenderAndClearPixels(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (auto& p : pixel_draw_request_queue) {
        SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, p.a);
        SDL_RenderDrawPoint(renderer, p.x, p.y);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    pixel_draw_request_queue.clear();
}

void ImageDB::CreateDefaultParticleTextureWithName(const std::string &name, SDL_Renderer* renderer){
    if(texture_cache.find(name) != texture_cache.end()){
        return;
    }
    
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);
    
    Uint32 white_color = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
    SDL_FillRect(surface, NULL, white_color);
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_FreeSurface(surface);
    texture_cache[name] = texture;
    
}
