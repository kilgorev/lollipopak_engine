//
//  imageDB.h
//  game_engine
//
//  Created by Tori Kilgore on 2/5/26.
//

#ifndef imageDB_h
#define imageDB_h

#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include "rapidjson/document.h"
#include "engineUtils.h"
#include "SDL_image.h"
#include "Helper.h"
#include "glm/glm/glm.hpp"

class ImageDB{
public:
    struct ImageDrawRequest{
        std::string image_name;
        float x,y;
        int rotation_degrees;
        float scale_x, scale_y, pivot_x, pivot_y;
        int r, g, b, a;
        int sorting_order;
    };
    
    struct PixelDrawRequest {
        int x, y, r, g, b, a;
    };
    
    static std::vector<ImageDrawRequest> image_draw_request_queue;
    static std::vector<ImageDrawRequest> ui_draw_request_queue;
    static std::vector<PixelDrawRequest> pixel_draw_request_queue;

    
    rapidjson::Document image;
    std::vector<SDL_Texture*> image_textures;
    std::unordered_map<std::string, SDL_Texture*> texture_cache;
    unordered_map<string, string> images;
    
    void CreateDefaultParticleTextureWithName(const std::string &name, SDL_Renderer* renderer);
    SDL_Texture* getTexture(SDL_Renderer* renderer, const std::string& image_name);
     void findImages(rapidjson::Document &image, bool array, const char* member);
     void RenderAndClearAllImages(SDL_Renderer* renderer, float zoom_factor, glm::vec2 cam_pos, glm::ivec2 cam_dimensions);
     void RenderAndClearUIImages(SDL_Renderer* renderer);

    static bool compare_image_requests(const ImageDrawRequest& a, const ImageDrawRequest& b);
    static void Draw(const std::string& image_name, float x, float y);
    
     static void DrawEx(const std::string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order);

    static void DrawUI(const std::string& image_name, float x, float y);

    static void DrawUIEx(const std::string& image_name, float x, float y, float r, float g, float b, float a, float sorting_order);
    
    static void DrawPixel(float x, float y, float r, float g, float b, float a);
    
    static void RenderAndClearPixels(SDL_Renderer* renderer);
    
};

#endif /* imageDB_h */
