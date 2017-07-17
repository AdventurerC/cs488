#include <glm/gtx/io.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Particle{
    glm::vec3 _pos;
    glm::vec3 _speed;
    glm::vec3 _color;
    //float size;
    float _life;

    Particle() : 
        _pos(glm::vec3(0.0f)),
        _speed(glm::vec3(0.0, -0.1, 0.0)),
        _life(100){

        }

};

