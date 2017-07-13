#pragma once

#include <glm/glm.hpp>
#include "stb_image.hpp"
#include <string>
#include <iostream>
#include <vector>
#include "lodepng/lodepng.h"


struct Texture{
    Texture() : _w(0), _h(0), _comp(0), _data(nullptr){
    }
    Texture(char* filename){
        loadFile(filename);
    }

    void loadFile(char* filename){
        //_data = stbi_load(filename, &_w, &_h, &_comp, STBI_rgb_alpha);
        lodepng_decode32_file(&_data, &_w, &_h, (const char*)filename);
        //std::vector<unsigned char> out;

        //lodepng::decode(out, _w, _h, filename);

        //_data = reinterpret_cast<unsigned char*> (out.data());

        if (_data == nullptr){
            std::cout << ("Failed to load texture") << std::endl;
        }
    }

    unsigned int _w;
    unsigned int _h;
    int _comp;
    unsigned char* _data;
};