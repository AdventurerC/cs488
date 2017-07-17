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
    glm::mat4 _playerTrans;
    double _speed;
    Shot(GeometryNode* player, int id) : 
        _player(player), 
        _meshId("plane"),
        _trans(glm::mat4()),
        _speed(1.0) 
    {
        _trans = glm::scale(glm::vec3(0.05, 1.0, 0.2)) * glm::translate(glm::vec3(0.0, -0.1, 0.0)) * _trans;
        _self = new GeometryNode("plane", "shot"+std::to_string(id), 0.05, 1, 0.2);
        _self->set_transform(_trans);
        _playerTrans = player->trans;
        
    }

    void advance(){
        _trans = glm::translate(glm::vec3(0.0, 0.0, -_speed)) * _trans;
        _self->set_transform(_playerTrans *_trans);
    }

};