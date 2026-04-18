//
//  dialogue.h
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#ifndef dialogue_h
#define dialogue_h

#include "sceneDB.h"
#include "audioDB.h"
#include "render.h"
#include <vector>
#include <string>


class DialogueSystem {
public:
    vector<string> dialogue_text;

    int damage_cooldown_frame = -180;

    void applyDialogueEffects(Actor* actor, int &health, int &score, bool &win, bool &lose, SceneDB &scene, AudioDB &audio, bool trigger, bool contact);

    bool extractNextScene(Actor* actor, string &next_scene, SceneDB &scene);

    void renderDialogue(SceneDB &scene, Camera &cam, TextDB &txt_handler, SDL_Renderer* renderer, const rapidjson::Document &doc, bool trigger, bool collider, const Actor* actor);

    bool processDialogue(int &health, int &score, bool &win, bool &lose,
                    rapidjson::Document &scene_doc, SceneDB &scene,
                     Camera &cam, SDL_Renderer* renderer);
};


#endif /* dialogue_h */
