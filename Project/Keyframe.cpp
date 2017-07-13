#include "Keyframe.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <glm/gtx/io.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

Keyframe::Keyframe(GeometryNode* node, int t) : 
	time(t), 
	position(vec4(0, 0, 0, 1)), 
	trans(mat4()),
	nextKeyframe(nullptr)
{
	if (node != nullptr){
		nodeTrans = node->trans;
		position = vec4(vec3(node->hitbox->_pos), 1);
	}
}


void Keyframe::rotate(char axis, float angle){
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
	trans = rot_matrix * trans;
}

void Keyframe::scale(const glm::vec3 & amount) {
	trans = glm::scale(amount) * trans;
}

void Keyframe::translate(const glm::vec3& amount) {
	trans = glm::translate(amount) * trans;
}

void Keyframe::setNextKeyframe(Keyframe* next){
	nextKeyframe = next;
}