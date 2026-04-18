//
//  textDB.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/6/26.
//

#include <stdio.h>
#include "textDB.h"

std::vector<TextDB::TextRequest> TextDB::draw_queue;
std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> TextDB::font_cache;

TTF_Font* TextDB::getFont(const std::string& font_name, int font_size) {
    auto& size_map = font_cache[font_name];
    auto it = size_map.find(font_size);
    if (it != size_map.end()) return it->second;
    
    std::string path = "resources/fonts/" + font_name + ".ttf";
    if (!std::filesystem::exists(path)) {
        std::cout << "error: font " << font_name << " missing";
        exit(0);
    }
    TTF_Font* font = TTF_OpenFont(path.c_str(), font_size);
    size_map[font_size] = font;
    return font;
}

void TextDB::Draw(const std::string& content, float x, float y,
                 const std::string& font_name, float font_size,
                 float r, float g, float b, float a) {
    TextRequest req;
    req.content = content;
    req.x = (int)x;
    req.y = (int)y;
    req.font_name = font_name;
    req.font_size = (int)font_size;
    req.color = {(Uint8)(int)r, (Uint8)(int)g, (Uint8)(int)b, (Uint8)(int)a};
    draw_queue.push_back(req);
}

void TextDB::renderText(SDL_Renderer* renderer) {
    for (auto& req : draw_queue) {
        TTF_Font* font = getFont(req.font_name, req.font_size);
        if (!font) continue;
        
        SDL_Surface* surface = TTF_RenderText_Solid(font, req.content.c_str(), req.color);
        if (!surface) continue;
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) continue;
        
        float w = 0, h = 0;
        Helper::SDL_QueryTexture(texture, &w, &h);

        SDL_FRect dst = {(float)req.x, (float)req.y, w, h};
        Helper::SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }
    draw_queue.clear();
}
