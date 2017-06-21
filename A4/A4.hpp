#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
);

Intersection intersect(SceneNode *root, Ray *ray);

glm::vec3 rayColor(Ray* r, Intersection &inter, 
	const glm::vec3 & ambient, const std::list<Light *> & lights, SceneNode *root,
	/*const glm::vec3 & bg,*/
	int &maxHits);

void printHier(SceneNode *root);

glm::vec3 getBg(int x, int y, int w, int h);