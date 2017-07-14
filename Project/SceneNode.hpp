#pragma once

#include "Material.hpp"
#include "Texture.hpp"

#include <glm/glm.hpp>

#include <list>
#include <string>
#include <iostream>

enum class NodeType {
	SceneNode,
	GeometryNode,
	JointNode
};

class Ray {
public:
	glm::vec3 _dir;
	glm::vec3 _orig;
	Ray(glm::vec3 orig, glm::vec3 dir) : _orig(orig), _dir(dir) { }
	Ray() : _orig(glm::vec3()), _dir(glm::vec3()){ }
};

class Intersection {
public:
	glm::vec3 _point;
	glm::vec3 _normal;
	Material *_material;
	double _t;
	bool _hit;
	Intersection() : _hit(false), _t(std::numeric_limits<double>::infinity()){ }

};

class SceneNode {
public:
    SceneNode(const std::string & name);

	SceneNode(const SceneNode & other);

    virtual ~SceneNode();
    
	int totalSceneNodes() const;
    
    const glm::mat4& get_transform() const;
    const glm::mat4& get_inverse() const;
    
    void set_transform(const glm::mat4& m);
    
    void add_child(SceneNode* child);
    
    void remove_child(SceneNode* child);

	void set_keyframe_parent_transform(const glm::mat4& parentTrans);

	//-- Transformations:
    void rotate(char axis, float angle);
    void scale(const glm::vec3& amount);
    void translate(const glm::vec3& amount);

	void start_undo();
    glm::mat4& end_undo();

    void selectChild();

	friend std::ostream & operator << (std::ostream & os, const SceneNode & node);

	bool isSelected;

    // Transformations
    glm::mat4 trans;
    glm::mat4 invtrans;
	glm::mat4 undo_rot;
    
    std::list<SceneNode*> children;

	NodeType m_nodeType;
	std::string m_name;
	unsigned int m_nodeId;

private:
	// The number of SceneNode instances.
	static unsigned int nodeInstanceCount;
};
