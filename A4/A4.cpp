#include <glm/ext.hpp>

#include "A4.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"

#define EPSILON 0.0001

#define RENDER_BOUNDING false

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
) {

  // Fill in raytracing code here...

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	//std::cout << std::endl;
	//std::cout << "Hierarchy: " << std::endl;
	//printHier(root);

	size_t h = image.height();
	size_t w = image.width();

	glm::vec4 p_k;

	glm::vec3 _eye = eye;
	glm::vec3 _view = view;

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {

			//convert (x,y) to world coordinates
			p_k[0] = x; //x_k
			p_k[1] = y; //y_k
			p_k[2] = 0; //z_k
			p_k[3] = 1;

			double d = glm::length(_view);
			double h_fov = 2*d*tan(glm::radians(fovy)/2);

			glm::mat4 T1 = glm::translate(glm::vec3(-(double)w/2.0, -(double)h/2.0, d));
			glm::mat4 S2  = glm::scale(glm::vec3(-h_fov/(double)h, -h_fov/(double)h, 1.0));
			
			glm::vec3 u, v, w_vec;
			w_vec = glm::normalize(_view);
			u = glm::normalize(glm::cross(up, w_vec));
			v = glm::cross(w_vec, u);

			glm::mat4 R3 = glm::mat4( glm::vec4(u, 0),
									glm::vec4(v, 0),
									glm::vec4(w_vec, 0),
									glm::vec4(glm::vec3(0.0), 1));
			
			glm::mat4 T4 = glm::mat4(glm::vec4(1, 0, 0, 0),
									glm::vec4(0, 1, 0, 0),
									glm::vec4(0, 0, 1, 0),
									glm::vec4(eye[0], eye[1], eye[2], 1));

			//glm::vec4 p_world4 = to_world*p_k;
			glm::mat4 to_world = T4 * R3 * S2 * T1;
			glm::vec3 p_world = glm::vec3(to_world * p_k);//glm::vec3(p_world4[0], p_world4[1], p_world4[2]);

			Ray r(_eye, p_world - _eye);
			Intersection inter = intersect(root, &r);

			// Red: increasing from top to bottom
			// Green: increasing from left to right
			// Blue: in lower-left and upper-right corners
			/*glm::vec3 color((double)y / h, (double)x / w, ((y < h/2 && x < w/2)
							|| (y >= h/2 && x >= w/2)) ? 1.0 : 0.0);*/
			glm::vec3 color = getBg(x, y, w, h);

			if (inter._hit){
				const PhongMaterial * phong_m = static_cast<const PhongMaterial *>(inter._material);
				int maxHits = 3;
				color = rayColor (&r, inter, ambient, lights, root, maxHits);
			} 

			// Red: increasing from top to bottom
			image(x, y, 0) = color[0];
			// Green: increasing from left to right
			image(x, y, 1) = color[1];
			// Blue: in lower-left and upper-right corners
			image(x, y, 2) = color[2];
		}
	}
	//image.savePng("test.png");

}

Intersection intersect(SceneNode *root, Ray *ray) {
	Intersection intersection;
	Material *material = nullptr;

	Ray r(glm::vec3(root->get_inverse() * glm::vec4(ray->_orig, 1)), glm::vec3(root->get_inverse() * glm::vec4(ray->_dir,0)));
	//Ray r(glm::mat3(root->get_transform()) * ray->_orig, glm::mat3(root->get_transform()) * ray->_dir);

	if (root->m_nodeType == NodeType::GeometryNode){
		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(root);
		//test intersection with actual primitive
		intersection = (RENDER_BOUNDING ? geometryNode->m_primitive->intersect_bounding(&r) 
									: geometryNode->m_primitive->intersect(&r));
		material = geometryNode->m_material;
	}

	for (SceneNode *child : root->children){

		Intersection i = intersect(child, &r);

		if (i._t < EPSILON) continue;

		if (i._hit){
			if (!intersection._hit || i._t < intersection._t) {
				intersection = i;
				intersection._t = i._t;
				intersection._hit = true;
				//intersection._point = i._point;
				//intersection._normal = i._normal;
				if (i._material != nullptr){
					intersection._material = i._material;
				}
			}
		}
	}

	//transform intersection point by original transformation
	
	intersection._point = glm::vec3(root->get_transform() * glm::vec4(intersection._point,1));
	intersection._normal = glm::normalize(glm::transpose(glm::mat3(root->get_inverse())) * intersection._normal);
	if (intersection._hit){
		if (material != nullptr){
			intersection._material = material;
		}
	}

	return intersection;
}

//maxHits not even used yet
glm::vec3 rayColor(Ray* r, Intersection &inter, 
	const glm::vec3 & ambient, const std::list<Light *> & lights, SceneNode *root,
	/*const glm::vec3 & bg,*/
	int &maxHits) 
{
	const PhongMaterial * phong_m = static_cast<const PhongMaterial *>(inter._material);
	Light _light;
	glm::dvec3 p = inter._point;
	glm::dvec3 raydir = glm::normalize(r->_orig - inter._point); //intersection to eye
	glm::dvec3 light_dir;
	//glm::vec3 total_col = bg;
	glm::dvec3 col = phong_m->kd() * ambient;
	glm::dvec3 normal = glm::normalize(glm::dvec3(inter._normal));

	//implementation based off of A3 fragment shader
	for (Light *light : lights){
		glm::dvec3 diffuse(0.0f);
		glm::dvec3 specular(0.0f);

		//from intersection point to light
		light_dir = light->position - inter._point;
		glm::dvec3 shadow_dir = light_dir;
		light_dir = glm::normalize(light_dir);
		Ray shadow(glm::dvec3(inter._point) + EPSILON * shadow_dir, shadow_dir);

		//check shadow ray intersection
		Intersection shadow_inter = intersect(root, &shadow);

		//if shadow ray hits something, don't do anything
		if (shadow_inter._hit && glm::length(shadow_inter._t * shadow._dir) < glm::length(shadow_dir)) continue;
		
		//L - 2N(L*N)
		glm::dvec3 reflected_ray = light_dir - 2*glm::dot(light_dir, normal)*normal; 
		double l_n = glm::dot(normal, light_dir); //L*N
		if (l_n < 0) l_n = 0.0;
		if (glm::length(phong_m->kd()) != 0){
			diffuse = l_n*phong_m->kd();// * light->colour;
		}
		
		//Ray reflected(glm::dvec3(inter._point) + 0.001*reflected_ray, reflected_ray);
		//reflected_ray = glm::normalize(reflected_ray);

		//double r_v = abs(glm::dot(reflected_ray, raydir));//

		if (glm::length(phong_m->ks()) > 0 && maxHits > 0){
		//if (l_n > 0.0) {
			glm::dvec3 v = glm::normalize(-inter._point);
			glm::dvec3 l = shadow_dir;
			double n_h = std::max(glm::dot(normal, glm::normalize(v + l)), 0.0);
			
			specular = phong_m->ks() * pow(n_h, phong_m->shininess());
			/*if (l_n != 0)
				specular = (pow(r_v,phong_m->shininess()))*phong_m->ks();// * light->colour;
			/*specular = pow(ks,phong_m->shininess() )*phong_m->ks() *
				rayColor(&reflected, inter, ambient, lights, root, --maxHits);*/
		}
		col = col + glm::dvec3(light->colour) * (diffuse + specular /( light->falloff[0] + 
				light->falloff[1] * glm::length(reflected_ray) +
				light->falloff[2] * pow(glm::length(reflected_ray), 2)));
	}
	
	return col;
}

void printHier(SceneNode *root){
	std::cout << *root;// << std::endl;
	std::cout << " transform: ";
	std::cout << glm::to_string(root->get_transform()) << std::endl;
	std::cout << std::endl;

	for (SceneNode *child : root->children){
		printHier(child);
	}
}

//generates a black - blue gradient night sky with random stars
glm::vec3 getBg(int x, int y, int w, int h){
	//default from starter code
	/*return glm::vec3((double)y / h, (double)x / w, ((y < h/2 && x < w/2)
							|| (y >= h/2 && x >= w/2)) ? 1.0 : 0.0);*/
	
	//dark at top, light at bottom;
	double r = (double)y/(double)h;
	double g = (double)y/(double)h;
	double b = (double)y/(double)h;

	r *= 0.1;
	g *= 0.1;

	int rand = std::rand() % 100;

	if (rand == 0) {
		rand = std::rand() % 20 + 1;
		glm::dvec3 col(r, g, b);
		return col + glm::dvec3(1.0)/(double)rand;
	}

	return glm::vec3(r, g, b);
}