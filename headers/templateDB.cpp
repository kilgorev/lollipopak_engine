//
//  templateDB.cpp
//  game_engine
//
//  Created by Tori Kilgore on 2/10/26.
//

#include <stdio.h>
#include "templateDB.h"
const rapidjson::Document& TemplateDB::getTemplate(const std::string& name) {
    auto it = template_cache.find(name);
    if (it != template_cache.end()) {
        return it->second;
    }

    std::string path = "resources/actor_templates/" + name + ".template";

    if (!filesystem::exists(path)) {
        cout << "error: template " << name << " is missing\n";
        exit(0);
    }

    rapidjson::Document doc;
    EngineUtils::ReadJsonFile(path, doc);

    auto [inserted_it, _] =
        template_cache.emplace(name, std::move(doc));

    return inserted_it->second;
}
