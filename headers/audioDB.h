//
//  audioDB.hpp
//  game_engine
//
//  Created by Tori Kilgore on 2/7/26.
//

#ifndef audioDB_h
#define audioDB_h

#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
#include "SDL.h"
#include "AudioHelper.h"
#include "rapidjson/document.h"
#include "engineUtils.h"

class AudioDB{
public:
    unordered_map<string, string> audios;
    
    bool playing = false;
    
    string intro_bgm;
    string game_over_good_audio;
    string game_over_bad_audio;
    string gameplay_audio;
    
    string audio_file;
    static Mix_Chunk* findAudio(const string &audio_name);
    void loadAudio(const string &audio, int loops = -1);
    void stopAudio();
    
    static void Init();
    
    static int PlayChannel(int channel, const string &audio_clip_name, bool does_loop);
    static int HaltChannel(int channel);
    static void SetVolume(int channel, float volume);
    
private:
    static inline unordered_map<string, Mix_Chunk*> loaded_audio;
};

#endif /* audioDB_hpp */
