#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	void drawCube();

	glm::vec2 normalize(glm::vec4 &point);
	void lookAt(glm::vec3 &lookAt, glm::vec3 &lookFrom, glm::vec3 &up);
	void perspective();
	void rotate(float amount);
	void translate(float amount);
	void scale(float amount);

	glm::vec2 toGLCoord(GLfloat x, GLfloat y);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

	glm::mat4 model;
	glm::mat4 scaler;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 screen;

	GLfloat aspect; //= w/h

	bool m_movingX;
	bool m_movingY;
	bool m_movingZ;

	//bool m_translating;
	//bool m_rotating;
	//bool m_scaling;

	float m_mouseX;
	float m_mouseY;

	float m_scaleFactor;

	glm::vec3 m_cube3D[8];
	glm::vec2 m_cube2D[8];

	glm::vec3 m_gnomon3D[4];
	glm::vec2 m_gnomon2D[4];

	glm::vec2 m_screen[4];
	glm::vec2 topLeft;
	glm::vec2 bottomRight;
	bool beginDrag;
	bool endDrag;

	GLfloat m_near;
	GLfloat m_far;
	GLfloat m_fov;

	int temp;

	enum Mode {
		ROTATE,
		TRANSLATE,
		SCALE,
		FOV,
		VIEWPORT
	};

	Mode m_activeMode;

	enum Coordinate {
		MODEL,
		VIEW,
		PERSP,
		SCREEN
	};

	Coordinate m_activeCoord;
};
