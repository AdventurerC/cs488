#pragma once

#include "GeometryNode.hpp"
#include <glm/glm.hpp>
#include <vector>

struct Bounds{
    glm::dvec3 _origin; //origin of area
    glm::dvec3 _maxXYZ; //max (x,y,z) in area (bottom right front corner)    

    Bounds(glm::dvec3 origin, glm::dvec3 maxXYZ) : _origin(origin), _maxXYZ(maxXYZ) {}
    Bounds() : _origin(glm::dvec3(0.0f)), _maxXYZ(glm::dvec3(0.0f)) {}

    double x(){ return _origin.x - _maxXYZ.x/2.0; }
	double z(){ return _origin.z - _maxXYZ.z/2.0; }
	double y(){ return _origin.y - _maxXYZ.y/2.0; }

	double x1(){ return _origin.x + _maxXYZ.x/2.0; }
	double z1(){ return _origin.z + _maxXYZ.z/2.0; }
	double y1(){ return _origin.y + _maxXYZ.y/2.0; }
};

class CollisionTreeNode {
public:
    CollisionTreeNode(Bounds bounds, int curDepth);
    int level;
    Bounds _bounds;
    // q2 | q1
    // -------
    // q3 | q4
     //size 4
    //CollisionTreeNode* q1;
    //CollisionTreeNode* q2;
    //CollisionTreeNode* q3;
    //CollisionTreeNode* q4;

    bool intersect(CollisionTreeNode* other);
    bool intersectGeometry(GeometryNode* other, bool checkY = false);
    void collideGeometry(GeometryNode* other, std::vector<GeometryNode*> &collisions, std::vector<glm::vec3> &axis, bool checkY = false);
    int findQuadrant(Bounds other);
    void construct(SceneNode* root);
    void insert(GeometryNode* node);
    void makeChildren();
    bool collide2Geo(GeometryNode* node, GeometryNode* other, glm::vec3 &axis, bool checkY = false);
private:
    int _depth;
    std::vector<CollisionTreeNode* > _children;
    std::vector<GeometryNode*> _geometryList;
};
