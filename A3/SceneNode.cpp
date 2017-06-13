#include "SceneNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/io.hpp>

using namespace glm;


// Static class variable
unsigned int SceneNode::nodeInstanceCount = 0;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
	m_nodeType(NodeType::SceneNode),
	trans(mat4()),
	rot(mat4()),
	transl(mat4()),
	undo_rot(mat4()),
	isSelected(false),
	scale_mat(mat4()),
	m_nodeId(nodeInstanceCount++)
{

}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
	: m_nodeType(other.m_nodeType),
	  m_name(other.m_name),
	  trans(other.trans),
	  invtrans(other.invtrans)
{
	for(SceneNode * child : other.children) {
		this->children.push_front(new SceneNode(*child));
	}
}

//---------------------------------------------------------------------------------------
SceneNode::~SceneNode() {
	for(SceneNode * child : children) {
		delete child;
	}
}

//---------------------------------------------------------------------------------------
void SceneNode::set_transform(const glm::mat4& m) {
	trans = m;
	invtrans = m;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_transform() const {
	return trans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_inverse() const {
	return invtrans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_translation() const {
	return transl;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_rotation() const {
	return rot;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_scale() const {
	return scale_mat;
}

//---------------------------------------------------------------------------------------
void SceneNode::add_child(SceneNode* child) {
	children.push_back(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::remove_child(SceneNode* child) {
	children.remove(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			break;
		case 'y':
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
	//cout << rot_matrix << endl;
	trans = rot_matrix * trans;

	//cout << trans << endl;
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
	scale_mat = glm::scale(amount) * scale_mat;
	trans = glm::scale(amount) * trans;
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
	transl = glm::translate(amount) * transl;
	trans = glm::translate(amount) * trans;
}

void SceneNode::start_undo(){
	undo_rot = mat4();
}

mat4& SceneNode::end_undo(){
	return undo_rot;
}

void SceneNode::selectChild(){
	for(SceneNode * child : children) {
		if (child->m_nodeType == NodeType::GeometryNode){
			child->isSelected = this->isSelected;
		}
	}
}

//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() const {
	return nodeInstanceCount;
}

//---------------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const SceneNode & node) {

	//os << "SceneNode:[NodeType: ___, name: ____, id: ____, isSelected: ____, transform: ____"
	switch (node.m_nodeType) {
		case NodeType::SceneNode:
			os << "SceneNode";
			break;
		case NodeType::GeometryNode:
			os << "GeometryNode";
			break;
		case NodeType::JointNode:
			os << "JointNode";
			break;
	}
	os << ":[";

	os << "name:" << node.m_name << ", ";
	os << "id:" << node.m_nodeId;
	os << "]";

	return os;
}
