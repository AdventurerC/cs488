#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::cout << "made mesh " << fname << std::endl;
	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	std::cout << "faces: " << m_faces.size() << std::endl;
	std::cout << "vertices: " << m_vertices.size() << std::endl;
}

//a - v1 = beta(v2 - v1) + gamma(v3 - v1) - t(b-a) 
Intersection Mesh::intersect(Ray* ray){
    Intersection intersection;

	for (Triangle face : m_faces ){
		glm::dvec3 p_0 = m_vertices[face.v1];
		glm::dvec3 p_1 = m_vertices[face.v2];
		glm::dvec3 p_2 = m_vertices[face.v3];

		glm::dvec3 a = ray->_orig;
		glm::dvec3 b_a = ray->_dir;

		//solve for beta, gamma, t using Cramer's rule
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

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

