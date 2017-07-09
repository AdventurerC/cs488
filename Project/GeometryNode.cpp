#include "GeometryNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>
#include "cs488-framework/ObjFileDecoder.hpp"

using namespace glm;

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
		const std::string & meshId,
		const std::string & name,
		double hitwidth,
		double hitheight,
		double hitdepth
)
	: SceneNode(name),
	  meshId(meshId),
	  hitbox(new Hitbox()),
	  m_vertices(),
	  m_faces()
{
	m_nodeType = NodeType::GeometryNode;
	hitbox->_pos = dvec3(0.0);
	hitbox->_maxXYZ = dvec3(hitwidth, hitheight, hitdepth);
	//hitbox->_width = hitwidth;
	//hitbox->_height = hitheight;
	//hitbox->_depth = hitdepth;

	std::string file = "Assets/" + meshId + ".obj";//ccheating here, assuming file name is Assets/meshId.obj
	std::ifstream in(file.c_str());
	std::string code;
	std::string objectName;
	double vx, vy, vz;
	size_t s1, s2, s3;

	string currentLine;
	//copypasted part of ObjFileDecoder::decode since that doesn't return vertices 
	while (!in.eof()) {
        try {
            getline(in, currentLine);
        } catch (const ifstream::failure &e) {
            in.close();
            stringstream errorMessage;
            errorMessage << "Error calling getline() -- " << e.what() << endl;
        }
	    if (currentLine.substr(0, 2) == "o ") {
		    // Get entire line excluding first 2 chars.
		    istringstream s(currentLine.substr(2));
		    s >> objectName;


	    } else if (currentLine.substr(0, 2) == "v ") {
            // Vertex data on this line.
            // Get entire line excluding first 2 chars.
            istringstream s(currentLine.substr(2));
            glm::vec3 vertex;
            s >> vertex.x;
            s >> vertex.y;
            s >> vertex.z;
            m_vertices.push_back(vertex);

        }
    }

    std::vector<glm::vec2> uv;
    ObjFileDecoder::decode(file.c_str(), objectName, m_faces, m_normals, uv); 

	//cout << name << endl;
}

void GeometryNode::translate(const glm::vec3& amount) {
	set_transform( glm::translate(amount) * trans );
	//hitbox->_pos = glm::translate(amount) * hitbox->_pos;
}

void GeometryNode::scale(const glm::vec3 & amount) {
	set_transform( glm::scale(amount) * trans );
	//hitbox->_maxXYZ = glm::scale(amount) * hitbox->_maxXYZ;
}


//kill these later, implement collision in CollisionTree.cpp
bool GeometryNode::collide3D(GeometryNode* other, glm::vec3 &axis) {
	Hitbox* otherHitbox = other->hitbox;
	bool hit(false);

	//Left-Right
	if (hitbox->x() <= otherHitbox->x1()){
		axis.x = 1;
		hit = true;
	} 

	//Right-Left
	if (hitbox->x1() >= otherHitbox->x()){
		axis.x = -1;
		hit = true;
	}

	//Top-Bottom
	if (hitbox->z1() <= otherHitbox->z()){
		axis.z = 1;
		hit = true;
	}

	//bottom-Top
	if (hitbox->z() >= otherHitbox->z1()){
		axis.z = -1;
		hit = true;
	}

	if (hitbox->y() <= otherHitbox->y1()){
		axis.y = 1;
		hit = true;
	} 

	if (hitbox->y1() <= otherHitbox->y()){
		axis.y = -1;
		hit = true;
	} 


	return false;
}

bool GeometryNode::collide2D(GeometryNode* other, glm::vec3 &axis) {
	Hitbox* otherHitbox = other->hitbox;
	bool hit(false);

	//Left-Right
	if (hitbox->x() <= otherHitbox->x1()){
		axis.x = 1;
		hit = true;
	} 

	//Right-Left
	if (hitbox->x1() >= otherHitbox->x()){
		axis.x = -1;
		hit = true;
	}

	//Top-Bottom
	if (hitbox->z1() <= otherHitbox->z()){
		axis.z = 1;
		hit = true;
	}

	//bottom-Top
	if (hitbox->z() >= otherHitbox->z1()){
		axis.z = -1;
		hit = true;
	}

	return hit;
}