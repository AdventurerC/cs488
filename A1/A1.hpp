#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

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
private:
	void initGrid();
	void drawCube(int x, int z);
	void drawIndicator();
	void highlightCells(int x, int x1, float y, int z, int z1, float r, float g, float b);
	void reset();
	
	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.
	

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	glm::mat4 orig_proj;

	float colour[8][3];
	int current_col;
	
	int _x;
	int _z;

	bool _inCopyMode;
	bool _inRotateMode;

	double _mouseX;
	double _mouseY;

	double _rotateX;
	double _rotateY;

	int _zoom;
	double _zoomMultiplier;

	
	Grid _grid;

};
