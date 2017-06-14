#include "JointNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/io.hpp>

using namespace glm;

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {
	
}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
	cur_x = init;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;
	cur_y = init;
}

//---------------------------------------------------------------------------------------
void JointNode::rotate(char axis, float angle) {
	vec3 rot_axis;
	double prev_cur_x = cur_x;
	double prev_cur_y = cur_y;
	switch (axis) {
		case 'x':
			cur_x += angle;
			cur_x = std::min(std::max(cur_x, m_joint_x.min), m_joint_x.max);
			angle = cur_x - prev_cur_x;
			rot_axis = vec3(1,0,0);
			break;
		case 'y':
			cur_y += angle;
			cur_y = std::min(std::max(cur_y, m_joint_y.min), m_joint_y.max);
			angle = cur_y - prev_cur_y;
			rot_axis = vec3(0,1,0);
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
	        break;
		default:
			break;
	}
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	rot = rot_matrix * rot;
	undo_rot = rot_matrix * undo_rot;
	mat4 tempscale = scale_mat;
	trans = rot_matrix * trans;

}

void JointNode::rotate_to(char axis, float angle) {
	double delta = angle - cur_x;
	rotate(axis, delta);
}

void JointNode::resetJoint(){
	cur_y = m_joint_y.init;
	cur_x = m_joint_x.init;

}
