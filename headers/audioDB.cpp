//
//  audioDB.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/7/26.
//

#include "audioDB.h"

Mix_Chunk* AudioDB::findAudio(const string &audio_name) {
    if (audio_name.empty()) return nullptr;
    
    
    auto it = loaded_audio.find(audio_name);
    if (it != loaded_audio.end()) {
        return it->second;
    }

    string wav_path = "resources/audio/" + audio_name + ".wav";
    string ogg_path = "resources/audio/" + audio_name + ".ogg";

    Mix_Chunk* chunk = nullptr;
    if (filesystem::exists(wav_path)) {
        chunk = AudioHelper::Mix_LoadWAV(wav_path.c_str());
    }
    else if (filesystem::exists(ogg_path)) {
        chunk = AudioHelper::Mix_LoadWAV(ogg_path.c_str());
    }

    if (!chunk) return nullptr;

    loaded_audio[audio_name] = chunk;
    return chunk;
}

int AudioDB::PlayChannel(int channel, const std::string& audio_clip_name, bool does_loop) {
    Mix_Chunk* chunk = findAudio(audio_clip_name);
    if (!chunk) {
        cout << "error: failed to play audio clip " + audio_clip_name;
        exit(0);
    }
    int loops = does_loop ? -1 : 0;
    return AudioHelper::Mix_PlayChannel(channel, chunk, loops);
}

int AudioDB::HaltChannel(int channel) {
    return AudioHelper::Mix_HaltChannel(channel);
}

void AudioDB::loadAudio(const string &audio, int loops){
    if (audio.empty()) return;
    
    Mix_Chunk* chunk = findAudio(audio);
    if (!chunk) {
        cout << "error: failed to play audio clip " + audio;
        exit(0);
    }

    AudioHelper::Mix_PlayChannel(0, chunk, loops);
    playing = true;
}

void AudioDB::stopAudio(){
    if (playing) {
        AudioHelper::Mix_HaltChannel(0);
        playing = false;
    }
}

void AudioDB::SetVolume(int channel, float volume) {
    AudioHelper::Mix_Volume(channel, (int)volume);
}

void AudioDB::Init() {
    AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    AudioHelper::Mix_AllocateChannels(50);
    loaded_audio.clear();
}
