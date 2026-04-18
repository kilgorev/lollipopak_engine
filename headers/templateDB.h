//
//  templateDB.h
//  game_engine
//
//  Created by Tori Kilgore on 1/30/26.
//

#ifndef templateDB_h
#define templateDB_h

#include <iostream>
#include <unordered_map>
#include <string>
#include "engineUtils.h"
#include "filesystem"
#include "rapidjson/document.h"

using namespace std;

class TemplateDB{
public:
    unordered_map<string, rapidjson::Document> template_cache;
    
    const rapidjson::Document& getTemplate(const std::string& name);
    
};

#endif /* templateDB_h */
