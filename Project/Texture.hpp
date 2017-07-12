#pragma once

#include <glm/glm.hpp>
#include "stb_image.hpp"
#include <string>
#include <iostream>


struct Texture{
    Texture() : _w(0), _h(0), _comp(0), _data(nullptr){
    }
    Texture(char* filename){
        loadFile(filename);
    }

    void loadFile(char* filename){
        _data = stbi_load(filename, &_w, &_h, &_comp, STBI_rgb_alpha);
        if (_data == nullptr){
            std::cout << ("Failed to load texture") << std::endl;
        }
    }

    int _w;
    int _h;
    int _comp;
    unsigned char* _data;
};