#include "CollisionTree.hpp"

#define MAX_LEAF 3

CollisionTreeNode(Bounds bounds) 
    : _bounds(bounds) 
{
}

void CollisionTreeNode::construct(SceneNode* root){

}

void CollisionTreeNode::insert(GeometryNode* node){
    Bounds nodeBounds;
    nodeBounds._origin = dvec3(node->trans * node->hitbox->_pos); 
    nodeBounds._maxXYZ = node->trans * node->hitbox->_maxXYZ;

    if (_children.size() == 0 && _geometryList.size() < MAX_LEAF){
        _geometryList.push_back(node);
        //split
        if (_geometryList.size() >= MAX_LEAF){
            makeChildren();
            //std::vector<GeometryNode*> tempList = _geometryList;
            for (int i = 0; i < _geometryList.size(); i++){
                insert(_geometryList[i]);
            }
            _geometryList.clear();
        }
    } else { // _children.size.() == 4 && _geometryList.size() == 0; only store GeometryNode at leaf
        int q = findQuadrant(nodeBounds);
        if (q){

        }
    }

    
}