#include "CollisionTree.hpp"

#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

#define MAX_LEAF 3
#define MAX_DEPTH 20

CollisionTreeNode::CollisionTreeNode(Bounds bounds, int curDepth) 
    : _bounds(bounds), _depth(curDepth)
{
}

void CollisionTreeNode::construct(SceneNode* root, float curtime){
    if (root->m_nodeType == NodeType::GeometryNode){
        GeometryNode * geometryNode = static_cast<GeometryNode *>(root);
        insert(geometryNode, curtime);
    }

    for (SceneNode *child : root->children){
		child->set_transform(root->get_transform() * child->get_transform());
		construct(child, curtime);
		child->set_transform(glm::inverse(root->get_transform()) * child->get_transform());
	}
}


void CollisionTreeNode::insert(GeometryNode* node, float curtime){
    Bounds nodeBounds;
    //nodeBounds._origin = glm::dvec3(node->trans * glm::vec4(glm::vec3(node->hitbox->_pos), 1)); 
    
    if (node->hasAnimation()){
        int t = (int)curtime;
        Keyframe* cur = node->getKeyframeAt(t);
        Keyframe* next = node->getNextKeyframe(t);
        glm::vec4 p0 = cur->get_total_transform() * glm::vec4(node->hitbox->_pos, 1);
        glm::vec4 p1 = next->get_total_transform() * glm::vec4(node->hitbox->_pos, 1);
        glm::vec4 lerp = p0 + (curtime - t)*(p1-p0);
        nodeBounds._origin = glm::vec3(lerp);
    } else {
    //nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    //nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;
        nodeBounds._origin = glm::dvec3(node->trans * glm::vec4(glm::vec3(node->hitbox->_pos), 1)); 
    }
    nodeBounds._maxXYZ = glm::dvec3(node->hitbox->_maxXYZ);//glm::dvec3(other->trans * glm::scale(mat4(), vec3(other->hitbox->_maxXYZ)) * glm::vec4(1.0f));
    
    //nodeBounds._maxXYZ = glm::dvec3(node->trans * glm::scale(mat4(), vec3(node->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    if (_children.size() > 0){
        int q = findQuadrant(nodeBounds);
        if (q != -1){
            std::cout << "inserting " << node->m_name << "into q" << q+1 << std::endl;
            _children[q]->insert(node, curtime);
            return;
        }
    }

    _geometryList.push_back(node);

    if (_geometryList.size() >= MAX_LEAF && _depth < MAX_DEPTH){
            makeChildren();
            std::vector<GeometryNode*> tempList = _geometryList;
            _geometryList.clear();
            for (int i = 0; i < tempList.size(); i++){
                insert(tempList[i], curtime);
            }
    }
    
}

void CollisionTreeNode::clear(){
    if (_children.size() > 0){
        for (CollisionTreeNode* child : _children){
            child->clear();
        }
    }

    _children.clear();
    _geometryList.clear();
}

int CollisionTreeNode::findQuadrant(Bounds other){
    dvec3 otherXYZ = other._origin + 0.5*other._maxXYZ;
    dvec3 otherOrigin = other._origin;
    dvec3 origin = _bounds._origin;
    dvec3 maxXYZ = origin + 0.5*_bounds._maxXYZ;

    double otherX = otherOrigin.x - otherXYZ.x/2;
    double otherZ = otherOrigin.z - otherXYZ.z/2;
    double minZ = origin.z - maxXYZ.z/2;
    double minX = origin.x - maxXYZ.x/2;

    //std::cout << "origin min x " << origin.x << "other min x" << otherX 
    //<< "origin min z " << origin.z << "other max z" << otherXYZ.z << maxXYZ.x << otherXYZ.x << minZ << otherZ << std::endl;

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
    //std::cout << "making children" << std::endl;
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
    CollisionTreeNode* q1 = new CollisionTreeNode(Bounds(q1o, max), _depth++);
    CollisionTreeNode* q2 = new CollisionTreeNode(Bounds(q2o,max), _depth++);
    CollisionTreeNode* q3 = new CollisionTreeNode(Bounds(q3o,max), _depth++);
    CollisionTreeNode* q4 = new CollisionTreeNode(Bounds(q4o,max),_depth++);

    _children.push_back(q1);
    _children.push_back(q2);
    _children.push_back(q3);
    _children.push_back(q4);
}

/*bool CollisionTreeNode::intersectGeometry(GeometryNode* other, bool checkY){
    Bounds nodeBounds;
    //nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    //nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;
    nodeBounds._origin = glm::dvec3(other->trans * glm::vec4(glm::vec3(other->hitbox->_pos), 1)); 
    nodeBounds._maxXYZ = glm::dvec3(other->hitbox->_maxXYZ);//glm::dvec3(other->trans * glm::scale(mat4(), vec3(other->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    //std::cout << "origin = " << glm::to_string(nodeBounds._origin) << "maxXYZ = " << glm::to_string(nodeBounds._maxXYZ) << std::endl;

    bool hit(false);

    bool yHit = false;

    if (checkY){
        yHit = _bounds.y() < nodeBounds.y1() && _bounds.y1() > nodeBounds.y();
    }

    if (_bounds.x() < nodeBounds.x1() && _bounds.x1() > nodeBounds.x() &&
        _bounds.z() < nodeBounds.z1() && _bounds.z1() > nodeBounds.z()){
        return checkY ? yHit*true : true;
    }


    return false;
    
}*/

bool CollisionTreeNode::intersectGeometry(GeometryNode* other, float curtime, bool checkY){
    Bounds nodeBounds;
    if (other->hasAnimation()){
        int t = (int)curtime;
        Keyframe* cur = other->getKeyframeAt(t);
        Keyframe* next = other->getNextKeyframe(t);
        glm::vec4 p0 = cur->get_total_transform() * glm::vec4(other->hitbox->_pos, 1);
        glm::vec4 p1 = next->get_total_transform() * glm::vec4(other->hitbox->_pos, 1);
        glm::vec4 lerp = p0 + (curtime - t)*(p1-p0);
        nodeBounds._origin = glm::vec3(lerp);
    } else {
    //nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    //nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;
        nodeBounds._origin = glm::dvec3(other->trans * glm::vec4(glm::vec3(other->hitbox->_pos), 1)); 
    }
    nodeBounds._maxXYZ = glm::dvec3(other->hitbox->_maxXYZ);//glm::dvec3(other->trans * glm::scale(mat4(), vec3(other->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    //std::cout << "origin = " << glm::to_string(nodeBounds._origin) << "maxXYZ = " << glm::to_string(nodeBounds._maxXYZ) << std::endl;

    bool hit(false);

    bool yHit = false;

    if (checkY){
        yHit = _bounds.y() < nodeBounds.y1() && _bounds.y1() > nodeBounds.y();
    }

    if (_bounds.x() < nodeBounds.x1() && _bounds.x1() > nodeBounds.x() &&
        _bounds.z() < nodeBounds.z1() && _bounds.z1() > nodeBounds.z()){
        return checkY ? yHit*true : true;
    }


    return false;
    
}

//in: other
//out: collisions
void CollisionTreeNode::collideGeometry(GeometryNode* other, std::vector<GeometryNode*> &collisions, std::vector<vec3> &axis, float curtime, bool checkY){
    //Bounds nodeBounds;
    //nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    //nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;
    //nodeBounds._origin = glm::dvec3(other->trans * glm::vec4(glm::vec3(other->hitbox->_pos), 1)); 
    //nodeBounds._maxXYZ = glm::dvec3(other->hitbox->_maxXYZ);//glm::dvec3(other->trans * glm::scale(mat4(), vec3(other->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    for (auto node : _geometryList){
        vec3 geoAxis;
        if (collide2Geo(node, other, geoAxis, curtime, checkY) || collide2Geo(other, node, geoAxis, curtime, checkY) ){
            collisions.push_back(node);
            axis.push_back(geoAxis);
        }
    }

    for (auto child : _children){
        if (child->intersectGeometry(other, curtime, checkY)){
            child->collideGeometry(other, collisions, axis, curtime, checkY);
        }
    }
}

bool CollisionTreeNode::collide2Geo(GeometryNode* node, GeometryNode* other, glm::vec3 &axis, float curtime, bool checkY){
    if (node == other) return false;

    Bounds nodeBounds;

    //nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    //nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;
    if (node->hasAnimation()){
        int t = (int)curtime;
        Keyframe* cur = node->getKeyframeAt(t);
        Keyframe* next = node->getNextKeyframe(t);
        glm::vec4 p0 = cur->get_total_transform() * glm::vec4(node->hitbox->_pos, 1);
        glm::vec4 p1 = next->get_total_transform() * glm::vec4(node->hitbox->_pos, 1);
        glm::vec4 lerp = p0 + (curtime - t)*(p1-p0);
        nodeBounds._origin = glm::vec3(lerp);
    } else {
        nodeBounds._origin = glm::dvec3(node->trans * glm::vec4(glm::vec3(node->hitbox->_pos), 1)); 
    }    

    //nodeBounds._origin = glm::dvec3(node->trans * glm::vec4(glm::vec3(node->hitbox->_pos), 1)); 
    nodeBounds._maxXYZ = glm::dvec3(node->hitbox->_maxXYZ);//glm::dvec3(node->trans * glm::scale(mat4(), vec3(node->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    Bounds otherBounds;
    if (other->hasAnimation()){
        int t = (int)curtime;
        Keyframe* cur = other->getKeyframeAt(t);
        Keyframe* next = other->getNextKeyframe(t);
        glm::vec4 p0 = cur->get_total_transform() * glm::vec4(other->hitbox->_pos, 1);
        glm::vec4 p1 = next->get_total_transform() * glm::vec4(other->hitbox->_pos, 1);
        glm::vec4 lerp = p0 + (curtime - t)*(p1-p0);
        otherBounds._origin = glm::vec3(lerp);
    } else {
        otherBounds._origin = glm::dvec3(other->trans * glm::vec4(glm::vec3(other->hitbox->_pos), 1)); 
    }    

    //nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    //nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;
    //otherBounds._origin = glm::dvec3(other->trans * glm::vec4(glm::vec3(other->hitbox->_pos), 1)); 
    otherBounds._maxXYZ = glm::dvec3(other->hitbox->_maxXYZ);//glm::dvec3(other->trans * glm::scale(mat4(), vec3(other->hitbox->_maxXYZ)) * glm::vec4(1.0f));

    bool hit(false);

    bool yHit = false;

    if (checkY){
        yHit = nodeBounds.y() < otherBounds.y1() && nodeBounds.y1() > otherBounds.y();
    }


    if (nodeBounds.x() < otherBounds.x1() && nodeBounds.x1() > otherBounds.x() &&
        nodeBounds.z() < otherBounds.z1() && nodeBounds.z1() > otherBounds.z()){
        //std::cout << nodeBounds.x() << "<" << otherBounds.x1() << "&&" << nodeBounds.x1() << ">" << otherBounds.x() << "&&" <<
        //nodeBounds.z() << "<" << otherBounds.z1() << "&&" << nodeBounds.z1() << ">" << otherBounds.z() << std::endl;
        hit = checkY ? yHit*true : true;
    }

    if(!hit) return false;

    return hit;

}