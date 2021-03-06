#pragma once

#include "SceneNode.hpp"
#include <vector>
#include <map>
#include "Keyframe.hpp"

class Keyframe;

//rectangular hitbox
class Hitbox{
public:
	glm::dvec3 _pos;
	glm::dvec3 _basePos;
	glm::dvec3 _maxXYZ;
	//double _width;
	//double _height;
	//double _depth;

	Hitbox() : _basePos(glm::dvec3(0.0f)),
		_pos(glm::dvec3(0.0f)), 
		_maxXYZ(glm::dvec3(1.0f)){ }
	double x(){ return _pos[0] - _maxXYZ.x/2.0; }
	double z(){ return _pos[2] + _maxXYZ.z/2.0; }
	double y(){ return _pos[1] - _maxXYZ.y/2.0; }

	double x1(){ return _pos[0] + _maxXYZ.x/2.0; }
	double z1(){ return _pos[2] - _maxXYZ.z/2.0; }
	double y1(){ return _pos[1] + _maxXYZ.y/2.0; }

};

class GeometryNode : public SceneNode {
public:
	GeometryNode(
		const std::string & meshId,
		const std::string & name,
		double hitwidth = 1.0,
		double hitheight = 1.0,
		double hitdepth = 1.0
	);

	void translate(const glm::vec3& amount);
	void scale(const glm::vec3& amount);
	void setTransparency(float alpha);
	bool isTransparent();
	void kill();
	bool isEnemy();

	bool hasAnimation();
	void set_keyframe_parent_transform(const glm::mat4& parentTrans);
	void updateHitbox(float curtime);


	Keyframe* getKeyframeAt(int curtime);
	Keyframe* getNextKeyframe(int curtime);
	Keyframe* getPreviousKeyframe(int curtime);
	Keyframe* getLastKeyframe();
	Keyframe* getFirstKeyframe();

	void setKeyframe(int time);

	void setAnimationLength(int time);

	Material material;
	Texture texture;

	Hitbox *hitbox;
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_faces;
	std::vector<glm::vec3> m_normals;
	// Mesh Identifier. This must correspond to an object name of
	// a loaded .obj file.
	std::string meshId;

	std::map<int, Keyframe*> m_keyframes;

	int m_animationEnd;

	int loop;

	//bool draw;
};
