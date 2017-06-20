#include "PhongMaterial.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess )
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
{}

glm::vec3 PhongMaterial::kd() const{
	return m_kd;
}

glm::vec3 PhongMaterial::ks() const{
	return m_ks;
}

double PhongMaterial::shininess() const{
	return m_shininess;
}

PhongMaterial::~PhongMaterial()
{}	
