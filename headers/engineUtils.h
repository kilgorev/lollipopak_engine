//
//  engineUtils.h
//  game_engine
//
//  Created by Tori Kilgore on 1/26/26.
//

#ifndef engineUtils_h
#define engineUtils_h

#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <stdio.h>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include <filesystem>

using namespace std;

class EngineUtils {
public:
    static void ReadJsonFile(const std::string& path, rapidjson::Document & out_document)
    {
        FILE* file_pointer = nullptr;
    #ifdef _WIN32
        fopen_s(&file_pointer, path.c_str(), "rb");
    #else
        file_pointer = fopen(path.c_str(), "rb");
    #endif
        char buffer[65536];
        rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
        out_document.ParseStream(stream);
        std::fclose(file_pointer);

        if (out_document.HasParseError()) {
            rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
            std::cout << "error parsing json at [" << path << "]" << std::endl;
            exit(0);
        }
    }
    
    static bool findAndPrintJson(rapidjson::Document &doc, const string &file, const char* member, const bool &print){
        
        if(!filesystem::exists(file)){
            return false;
        }
        
        EngineUtils::ReadJsonFile(file, doc);

        
        if (doc.HasMember(member)){
            if(print){
                std::cout << doc[member].GetString();
            }
            
            return true;
        }
        return false;
    }
    
    static void validPath(const string &filepath, const string &error){
        if(!std::filesystem::exists(filepath)){
            cout << error;
            exit(0);
        }
    }


};
#endif /* engineUtils_h */
