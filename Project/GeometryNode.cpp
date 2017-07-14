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
	  m_faces(),
	  loop(0),
	  m_animationEnd(0)
{
	m_nodeType = NodeType::GeometryNode;
	hitbox->_pos = dvec3(0.0);
	hitbox->_maxXYZ = dvec3(hitwidth, hitheight, hitdepth);

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

void GeometryNode::setKeyframe(int time){
	Keyframe* key = new Keyframe(this, time);

	m_keyframes[time] = key;

	cout << "Created keyframe for " << m_name << " at " << time << endl;

	setAnimationLength(m_animationEnd);
}

/*void GeometryNode::setNextKeyframe(Keyframe* cur, Keyframe* next){
	cur->setNextKeyframe(next);
}*/

Keyframe* GeometryNode::getFirstKeyframe(){
	if (m_keyframes.size() == 0) return nullptr;
	return (m_keyframes.begin())->second;
}

Keyframe* GeometryNode::getLastKeyframe(){
	if (m_keyframes.size() == 0) return nullptr;
	return (m_keyframes.rbegin())->second;
}

Keyframe* GeometryNode::getKeyframeAt(int curtime){

	loop = curtime/m_animationEnd;

	curtime = curtime%m_animationEnd;

	if (m_keyframes.size() == 0) return nullptr;

	auto it = m_keyframes.find(curtime);
	if (it != m_keyframes.end()){
		return it->second;
	}

	Keyframe* ret = nullptr;

	it = m_keyframes.upper_bound(curtime);
	if (it == m_keyframes.end()){ //after last keyframe
		return getLastKeyframe();
	} else if (it == m_keyframes.begin()){ //before first keyframe
		if (loop > 1){
			return getLastKeyframe();
		} else {
			return getFirstKeyframe();
		}

	}

	return (--it)->second;

}

Keyframe* GeometryNode::getNextKeyframe(int curtime){

	//std::cout << "curtime = " << curtime << endl;

	loop = curtime/m_animationEnd;

	curtime = curtime%m_animationEnd;

	if (m_keyframes.size() == 0) return nullptr;

	/*auto it = m_keyframes.find(++curtime);
	if (it != m_keyframes.end()){
		return it->second;
	}*/


	auto it = m_keyframes.upper_bound(curtime);
	if (it == m_keyframes.end() ){
	//	std::cout << m_name << " curtime " << curtime <<" next key at " << m_keyframes.begin()->first << std::endl;
		return getFirstKeyframe();
	}
	//std::cout << m_name << " curtime " << curtime << " next key at " << (it)->first << std::endl;
	return (it)->second;
}

Keyframe* GeometryNode::getPreviousKeyframe(int curtime){

	loop = curtime/m_animationEnd;

	curtime = curtime%m_animationEnd;

	if (m_keyframes.size() == 0) return nullptr;
	auto it = m_keyframes.lower_bound(curtime);
	if (it == m_keyframes.end()){
		return getLastKeyframe();
	}
	return (--it)->second; 
}

void GeometryNode::setAnimationLength(int time){
	m_animationEnd = std::max(time, (m_keyframes.rbegin())->first);
}

bool GeometryNode::hasAnimation(){
	return m_keyframes.size() > 0;
}

//in -- tranformation of all parent nodes up to this one
void GeometryNode::set_keyframe_parent_transform(const glm::mat4& parentTrans){
	for(const auto& keyframe : m_keyframes){
		keyframe.second->set_parent_transform(parentTrans);
	}
}