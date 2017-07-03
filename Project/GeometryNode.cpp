#include "GeometryNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
		const std::string & meshId,
		const std::string & name,
		double hitwidth,
		double hitheight
)
	: SceneNode(name),
	  meshId(meshId),
	  hitbox(new Hitbox())
{
	m_nodeType = NodeType::GeometryNode;
	hitbox->_width = hitwidth;
	hitbox->_height = hitheight;
}

void GeometryNode::translate(const glm::vec3& amount) {
	set_transform( glm::translate(amount) * trans );
	hitbox->_pos = glm::translate(amount) * hitbox->_pos;
}