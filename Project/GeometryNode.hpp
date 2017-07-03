#pragma once

#include "SceneNode.hpp"

//circular hitbox
class Hitbox{
public:
	glm::dvec4 _pos;
	double _width;
	double _height;

	Hitbox() : _pos(glm::dvec4(0, 0, 0, 1)), _width(1.0), _height(1.0){ }
	double centerX(){ return _pos[0] - _width/2.0; }
	double centerZ(){ return _pos[2] - _height/2.0; }

};

class GeometryNode : public SceneNode {
public:
	GeometryNode(
		const std::string & meshId,
		const std::string & name,
		double hitwidth = 1.0,
		double hitheight = 1.0
	);

	void translate(const glm::vec3& amount);

	Material material;

	Hitbox *hitbox;
	// Mesh Identifier. This must correspond to an object name of
	// a loaded .obj file.
	std::string meshId;
};
