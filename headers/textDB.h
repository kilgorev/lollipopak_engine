//
//  textDB.h
//  game_engine
//
//  Created by Tori Kilgore on 2/6/26.
//

#ifndef textDB_h
#define textDB_h

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include "imageDB.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL.h"
#include "Helper.h"
#include "engineUtils.h"

using namespace std;

class TextDB {
public:
    struct TextRequest {
        std::string content;
        int x, y;
        std::string font_name;
        int font_size;
        SDL_Color color;
    };
    
    static std::vector<TextRequest> draw_queue;
    static std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> font_cache;
    
    static void Draw(const std::string& content, float x, float y,
                     const std::string& font_name, float font_size,
                     float r, float g, float b, float a);
    
    static TTF_Font* getFont(const std::string& font_name, int font_size);
    static void renderText(SDL_Renderer* renderer);
    
    static std::string font_name;
    static bool font_exists;
    void findText(rapidjson::Document &text, vector<string> &vec, const char* member, bool &exists);
};

#endif
