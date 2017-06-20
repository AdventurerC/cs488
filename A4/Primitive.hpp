#pragma once

#include <glm/glm.hpp>
#include "A4.hpp"
#include <vector>

class Primitive {
public:
  virtual ~Primitive();
  virtual Intersection intersect(Ray* ray) = 0;
};

class Sphere : public Primitive {
public:
  Sphere();
  virtual ~Sphere();
  virtual Intersection intersect(Ray* ray);
private:
  Primitive* _primitive;
};

class Cube : public Primitive {
public:
  Cube();
  virtual ~Cube();
  virtual Intersection intersect(Ray* ray);
private:
  Primitive* _primitive;
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  virtual Intersection intersect(Ray* ray);


private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size);
  
  virtual ~NonhierBox();
  virtual Intersection intersect(Ray* ray);

private:
  glm::vec3 m_pos;
  double m_cube [24];
  std::vector<glm::vec3> m_vertices;
  double m_size;
};
