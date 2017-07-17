#include <glm/gtx/io.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Particle{
    glm::vec3 _pos;
    glm::vec3 _speed;
    glm::vec3 _color;
    //float size;
    float _life;
    glm::mat4 _trans;

    Particle(glm::mat4 trans, glm::vec3 pos) :
        _trans(trans), 
        _pos(pos),
        _speed(glm::vec3(0.0, -0.1, 0.0)),
        _life(100000){
            _trans = glm::scale(glm::vec3(0.1, 0.1, 0.1)) * glm::mat4();
            _trans = trans * _trans;

            _trans = glm::translate(pos) * _trans;
        }

    void move(){
        //_pos += _speed;
        _trans = glm::translate(_speed) * _trans;
    }

};

