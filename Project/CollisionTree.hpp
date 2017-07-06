#pragma once

#include "GeometryNode.hpp"
#include <glm/glm.hpp>
#include <vector>

struct Bounds{
    glm::dvec3 _origin; //origin of area
    glm::dvec3 _maxXYZ; //max (x,y,z) in area (bottom right front corner)    

    Bounds(glm::dvec3 origin, glm::dvec3 maxXYZ) : _origin(origin), _maxXYZ(maxXYZ) {}
    Bounds() : _origin(glm::dvec3(0.0f)), _maxXYZ(glm::dvec3(0.0f)) {}
};

class CollisionTreeNode {
public:
    CollisionTreeNode(Bounds bounds);
    int level;
    std::vector<GeometryNode*> _geometryList;
    Bounds _bounds;
    // q2 | q1
    // -------
    // q3 | q4
    std::vector<CollisionTreeNode* > _children; //size 4
    //CollisionTreeNode* q1;
    //CollisionTreeNode* q2;
    //CollisionTreeNode* q3;
    //CollisionTreeNode* q4;

    bool intersect(CollisionTreeNode* other);
    bool intersectGeometry(GeometryNode* other);
    int findQuadrant(Bounds bounds);
    void construct(SceneNode* root);
    void insert(GeometryNode* node);
    void makeChildren();
};