#include "Primitive.hpp"
#include "polyroots.hpp"

Primitive::~Primitive()
{
}

Sphere::Sphere(){
    _primitive = new NonhierSphere(glm::vec3(0, 0, 0), 1.0);
}

Sphere::~Sphere()
{
}

Intersection Sphere::intersect(Ray* ray){
    return _primitive->intersect(ray);
}

Cube::Cube(){
    _primitive = new NonhierBox(glm::vec3(0,0,0), 1.0);
}

Cube::~Cube()
{
}

Intersection Cube::intersect(Ray* ray){
    return _primitive->intersect(ray);
}


NonhierSphere::~NonhierSphere()
{
}

//(P-c)(P-c) = R^2
//where P = a + t(b-a) = origin + t(direction)
//(a + t(d))(a + t(d)) - R^2 = 0
//d*d*t^2 + d*(a-c)*2*t + (a-c)*(a-c) - R^2 = 0;
//d = ray._dir; a = ray._orig; c = m_pos, R = m_radius
Intersection NonhierSphere::intersect(Ray* ray){
    Intersection intersection;

    //from notes
    glm::vec3 a_c = ray->_orig - m_pos;
    double B = 2*glm::dot(ray->_dir, a_c);
    double A = glm::dot(ray->_dir, ray->_dir);
    double C = glm::dot(a_c, a_c) - m_radius*m_radius;

    double roots[2];
    int n = quadraticRoots(A, B, C, roots);
    double t = -1;
    
    switch (n) {
        case 2:
            t = std::min(roots[0], roots[1]);
            if (t < 0) 
                t = std::max(roots[0], roots[1]);
            if (t < 0){
                intersection._hit = false;
                return intersection;
            } else {
                intersection._hit = true;
                intersection._t = t;
            }
        break;
        case 1:
            t = roots[0];
            if (t < 0){
                intersection._hit = false;
                return intersection;
            } else {
                intersection._hit = true;
                intersection._t = t;
            }
        break;
        default:
            intersection._hit = false;
            return intersection;
    }

    //t * ray->_dir because you can't multiply double and vec3....
    glm::vec3 t_dir;
    t_dir[0] = t * ray->_dir[0];
    t_dir[1] = t * ray->_dir[1];
    t_dir[2] = t * ray->_dir[2];

    intersection._point = ray->_orig + t_dir;
    intersection._normal = glm::normalize(intersection._point - m_pos);

    return intersection;
}

NonhierBox::NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
    double x = m_pos.x;
    double y = m_pos.y;
    double z = m_pos.z;
    double l = m_size;

    double tempcube[24] = {
        x, y, z + l,
        x, y, z,
        x + l, y, z,
        x + l, y, z + l,
        x, y + l, z + l,
        x, y + l, z,
        x + l, y + l, z,
        x + l, y + l, z + l
    };

    for (int i = 0; i < 24; i++){
        m_cube[i] = tempcube[i];
    }

    m_vertices.emplace_back(5, 6, 2);
    m_vertices.emplace_back(6, 7, 3);
    m_vertices.emplace_back(7, 8, 4);
    m_vertices.emplace_back(5, 1, 8);
    m_vertices.emplace_back(1, 2, 3);
    m_vertices.emplace_back(8, 7, 6);
    m_vertices.emplace_back(1, 5, 2);
    m_vertices.emplace_back(2, 6, 3);
    m_vertices.emplace_back(3, 7, 4);
    m_vertices.emplace_back(8, 1, 4);
    m_vertices.emplace_back(4, 1, 3);
    m_vertices.emplace_back(5, 8, 6);

  }

NonhierBox::~NonhierBox()
{
}

Intersection NonhierBox::intersect(Ray* ray){
    Intersection intersection;

    std::vector<glm::vec3>::iterator it;

    for (it = m_vertices.begin(); it != m_vertices.end(); ++it){
        glm::vec3 triangle = *it;
        int i0 = (triangle[0] - 1)*3;
        int i1 = (triangle[1] - 1)*3;
        int i2 = (triangle[2] - 1)*3;

        glm::dvec3 p_0 = glm::dvec3(m_cube[i0], m_cube[i0 + 1], m_cube[i0 + 2]);
        glm::dvec3 p_1 = glm::dvec3(m_cube[i1], m_cube[i1 + 1], m_cube[i1 + 2]);
        glm::dvec3 p_2 = glm::dvec3(m_cube[i2], m_cube[i2 + 1], m_cube[i2 + 2]);

        glm::dvec3 a = ray->_orig;
		glm::dvec3 b_a = ray->_dir;

        glm::dvec3 R = a - p_0;
		double X1 = p_1[0] - p_0[0];
		double X2 = p_2[0] - p_0[0];
		double X3 = -b_a[0];

		double Y1 = p_1[1] - p_0[1];
		double Y2 = p_2[1] - p_0[1];
		double Y3 = -b_a[1];

		double Z1 = p_1[2] - p_0[2];
		double Z2 = p_2[2] - p_0[2];
		double Z3 = -b_a[2];

		glm::mat3 M = glm::mat3(glm::vec3(X1, Y1, Z1),
								glm::vec3(X2, Y2, Z2),
								glm::vec3(X3, Y3, Z3));
		double D = glm::determinant(M);

		glm::mat3 M1 = glm::mat3(glm::vec3(R[0], R[1], R[2]),
								glm::vec3(X2, Y2, Z2),
								glm::vec3(X3, Y3, Z3));
		double D1 = glm::determinant(M1);

		glm::mat3 M2 = glm::mat3(glm::vec3(X1, Y1, Z1),
								glm::vec3(R[0], R[1], R[2]),
								glm::vec3(X3, Y3, Z3));
		double D2 = glm::determinant(M2);

		glm::mat3 M3 = glm::mat3(glm::vec3(X1, Y1, Z1),
								glm::vec3(X2, Y2, Z2),
								glm::vec3(R[0], R[1], R[2]));
		double D3 = glm::determinant(M3);

		double beta = D1/D;
		double gamma = D2/D;
		double t = D3/D;

		//glm::dvec3 i;
		//my life was saved when i realized this function existed
		//turns out we are not allowed to use it
		//bool inter = glm::intersectRayTriangle(a, b_a, p_0, p_1, p_2, i);

		//std::cout << beta << " " << gamma << " " << t << std::endl;
		if (beta >= 0 && gamma >= 0 && (beta + gamma) <= 1){
		//if (inter){
			//std::cout << "t = " << i.z << std::endl;
			//double t = i.z;
			if (!intersection._hit || (intersection._hit && t < intersection._t)){ // first intersection || closer intersection
				intersection._hit = true;
				intersection._t = t;

                intersection._point = a + t*(b_a);
				intersection._normal = glm::normalize(glm::cross(p_1 - p_0, p_2 - p_0));
			}
		} 
		
    }

    return intersection;
}
