#include "CollisionTree.hpp"

#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

#define MAX_LEAF 3

CollisionTreeNode::CollisionTreeNode(Bounds bounds) 
    : _bounds(bounds) 
{
}

void CollisionTreeNode::construct(SceneNode* root){
    if (root->m_nodeType == NodeType::GeometryNode){
        GeometryNode * geometryNode = static_cast<GeometryNode *>(root);
        insert(geometryNode);
    }

    for (SceneNode *child : root->children){
		child->set_transform(root->get_transform() * child->get_transform());
		construct(child);
		child->set_transform(glm::inverse(root->get_transform()) * child->get_transform());
	}
}

void CollisionTreeNode::insert(GeometryNode* node){
    Bounds nodeBounds;
    nodeBounds._origin = glm::dvec3(node->trans * glm::vec4(glm::vec3(node->hitbox->_pos), 1)); 
    nodeBounds._maxXYZ = glm::dvec3(node->trans * glm::scale(mat4(), vec3(node->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    if (_children.size() > 0){
        int q = findQuadrant(nodeBounds);
        if (q != -1){
            _children[q]->insert(node);
            return;
        }
    }

    _geometryList.push_back(node);

    if (_geometryList.size() >= MAX_LEAF){
            makeChildren();
            //std::vector<GeometryNode*> tempList = _geometryList;
            for (int i = 0; i < _geometryList.size(); i++){
                insert(_geometryList[i]);
            }
            _geometryList.clear();
    }
    
}

int CollisionTreeNode::findQuadrant(Bounds other){
    dvec3 otherXYZ = other._maxXYZ;
    dvec3 otherOrigin = other._origin;
    dvec3 origin = _bounds._origin;
    dvec3 maxXYZ = _bounds._maxXYZ;

    double otherX = otherOrigin.x - otherXYZ.x/2;
    double otherZ = otherOrigin.z - otherXYZ.z/2;
    double minZ = origin.z - maxXYZ.z/2;
    double minX = origin.x - maxXYZ.x/2;

    //q1
    if (origin.x <= otherX && origin.z >= otherXYZ.z && maxXYZ.x >= otherXYZ.x && minZ <= otherZ){
        return 0;
    }

    //q2
    if (origin.x >= otherXYZ.x && origin.z >= otherXYZ.z && minX <= otherX &&  minZ <= otherZ){
        return 1;
    }

    //q3
    if (origin.x >= otherXYZ.x && origin.z <= otherZ  && minX <= otherX && maxXYZ.z >= otherXYZ.z){
        return 2;
    }

    //q4
    if (origin.x <= otherX && origin.z <= otherZ && maxXYZ.x >= otherXYZ.x && maxXYZ.z >= otherXYZ.z){
        return 3;
    }

    return -1;

}

void CollisionTreeNode::makeChildren(){
    dvec3 origin = _bounds._origin;
    dvec3 maxXYZ = _bounds._maxXYZ;
    dvec3 q1o(origin.x + maxXYZ.x/4.0,
                    origin.y, origin.z - maxXYZ.z/4);
    dvec3 q2o(origin.x - maxXYZ.x/4.0,
                    origin.y, origin.z - maxXYZ.z/4);
    dvec3 q3o(origin.x - maxXYZ.x/4.0,
                    origin.y, origin.z + maxXYZ.z/4);
    dvec3 q4o(origin.x - maxXYZ.x/4.0,
                    origin.y, origin.z - maxXYZ.z/4);
    dvec3 max(maxXYZ.x/2, maxXYZ.y, maxXYZ.z/2);    
    CollisionTreeNode* q1 = new CollisionTreeNode(Bounds(q1o, max));
    CollisionTreeNode* q2 = new CollisionTreeNode(Bounds(q2o,max));
    CollisionTreeNode* q3 = new CollisionTreeNode(Bounds(q3o,max));
    CollisionTreeNode* q4 = new CollisionTreeNode(Bounds(q4o,max));

    _children.push_back(q1);
    _children.push_back(q2);
    _children.push_back(q3);
    _children.push_back(q4);
}

bool CollisionTreeNode::intersectGeometry(GeometryNode* other, bool checkY){
    Bounds nodeBounds;
    //nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    //nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;
    nodeBounds._origin = glm::dvec3(other->trans * glm::vec4(glm::vec3(other->hitbox->_pos), 1)); 
    nodeBounds._maxXYZ = glm::dvec3(other->trans * glm::scale(mat4(), vec3(other->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    std::cout << "origin = " << glm::to_string(nodeBounds._origin) << "maxXYZ = " << glm::to_string(nodeBounds._maxXYZ) << std::endl;

    bool hit(false);

	if (nodeBounds.x() <= _bounds.x1()){
		return true;
	} 

	if (nodeBounds.x1() >= _bounds.x()){
		return true;
	}

	if (nodeBounds.z1() >= _bounds.z()){
		return true;
	}

	if (nodeBounds.z() <= _bounds.z1()){
		return true;
	}

    if (checkY){

        if (nodeBounds.y() <= _bounds.y1()){
            return true;
        } 

        if (nodeBounds.y1() <= _bounds.y()){
            return true;
        } 
    }

    return false;
    
}