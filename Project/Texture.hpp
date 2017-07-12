#pragma once

#include <glm/glm.hpp>
#include "stb_image.hpp"

struct Texture{
    Texture(){
    }
    Texture(char* filename){
        loadFile(filename);
    }

    void loadFile(char* filename){
        _data = stbi_load(filename, &_w, &_h, &_comp, STBI_rgb);
    }

    int _w;
    int _h;
    int _comp;
    unsigned char* _data;
};