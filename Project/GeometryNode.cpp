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

	/*std::string file = "Assets/" + meshId + ".obj";//ccheating here, assuming file name is Assets/meshId.obj
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
	*/
}

void GeometryNode::translate(const glm::vec3& amount) {
	set_transform( glm::translate(amount) * trans );
	//hitbox->_pos = glm::translate(amount) * hitbox->_pos;
}

void GeometryNode::scale(const glm::vec3 & amount) {
	set_transform( glm::scale(amount) * trans );
	//hitbox->_maxXYZ = glm::scale(amount) * hitbox->_maxXYZ;
}

void GeometryNode::setTransparency(float alpha){
	material.alpha = alpha;
}

bool GeometryNode::isTransparent(){
	return abs(1.0f - material.alpha) > std::numeric_limits<float>::epsilon();
}