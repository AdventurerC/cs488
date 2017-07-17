#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>


#include "GeometryNode.hpp"


struct Shot {
    GeometryNode* _player;
    GeometryNode* _self;
    std::string _meshId;
    glm::mat4 _trans;
    double _speed;
    Shot(GeometryNode* player, int id) : 
        _player(player), 
        _meshId("plane"),
        _trans(glm::mat4()),
        _speed(1.0) 
    {
        _trans = glm::scale(glm::vec3(0.1, 1.0, 1.0)) * glm::translate(glm::vec3(0.0, -0.1, 0.0)) * _trans;
        _self = new GeometryNode("plane", "shot"+id, 0.1, 1, 1);
        _self->set_transform(_trans);
    
    }

    void advance(){
        _trans = glm::translate(glm::vec3(0.0, 0.0, 0.5)) * _trans;
        //_self->set_transform(player->get_transform() * _trans);
    }

};