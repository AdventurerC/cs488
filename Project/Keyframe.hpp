#pragma once

#include "GeometryNode.hpp"
#include <glm/glm.hpp>

class GeometryNode;

class Keyframe {
public:
    Keyframe(GeometryNode* node, int t);

    int time;

    glm::mat4 parentTrans;

    glm::vec4 position;
    glm::mat4 trans;

    //Keyframe* nextKeyframe;

    void rotate(char axis, float angle);
    void scale(const glm::vec3& amount);
    void translate(const glm::vec3& amount);

    void set_parent_transform(const glm::mat4& pTrans);
    glm::mat4 get_total_transform();

    //void setNextKeyframe(Keyframe* next);
};