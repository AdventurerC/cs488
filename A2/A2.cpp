#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)),
	 model(mat4()),
	 scaler(mat4()),
	 view(mat4()),
	 proj(mat4()),
	 look(mat4()),
	 m_movingX(false),
	 m_movingY(false),
	 m_movingZ(false),
	 m_activeMode(ROTATE),
	 m_near(1.5),
	 m_far(5.0),
	 m_activeCoord(MODEL),
	 aspect(1.0),
	 m_fov(60),
	 m_mouseX(0.0),
	 m_mouseY(0.0),
	 m_scaleFactor(1.0)
{
	//back
	m_cube3D[0] = vec3(-1, 1, -1);
	m_cube3D[1] = vec3(1,1,-1);
	m_cube3D[2] = vec3(1, -1, -1);
	m_cube3D[3] = vec3(-1,-1,-1);

	//front
	m_cube3D[4] = vec3(-1,1,1);
	m_cube3D[7] = vec3(-1,-1,1);
	m_cube3D[6] = vec3(1,-1,1);
	m_cube3D[5] = vec3(1,1,1);

	m_gnomon3D[0] = vec3(0);
	m_gnomon3D[1] = vec3(0.4, 0, 0); //x
	m_gnomon3D[2] = vec3(0, 0.4, 0); //y
	m_gnomon3D[3] = vec3(0, 0, 0.4); //z	

	m_screen[0] = vec2(-0.75,-0.75);
	m_screen[1] = vec2(0.75, -0.75);
	m_screen[2] = vec2(0.75, 0.75);
	m_screen[3] = vec2(-0.75, 0.75);

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}


glm::vec2 A2::toGLCoord(vec2 point){

	GLfloat x = point[0];
	GLfloat y = point[1];
	GLfloat w = m_windowWidth == 0 ? 768 : m_windowWidth;
	GLfloat h = m_windowHeight == 0 ? 768 : m_windowHeight;
	return vec2((2.0f * x) / w - 1.0f,
			1.0f - ( (2.0f * y) / h));
}

glm::vec2 A2::toGLCoord(GLfloat x, GLfloat y){
	GLfloat w = m_windowWidth == 0 ? 768 : m_windowWidth;
	GLfloat h = m_windowHeight == 0 ? 768 : m_windowHeight;
	return vec2((2.0f * x) / m_windowWidth - 1.0f,
			1.0f - ( (2.0f * y) / m_windowHeight));
}

void A2::reset(){
	model = mat4();
	scaler = mat4();
	view = mat4();
	proj = mat4();
	m_movingX = false;
	m_movingY = false;
	m_movingZ = false;
	m_activeMode = ROTATE;
	m_activeCoord = MODEL;
	m_near = 1.5,
	m_far = 5.0,
	//aspect = 1.0,
	m_fov = 60,
	m_scaleFactor= 1.0;
	endDrag = false;
	beginDrag = false;
	temp = 0;

	m_screen[0] = vec2(-0.75,-0.75);
	m_screen[1] = vec2(0.75, -0.75);
	m_screen[2] = vec2(0.75, 0.75);
	m_screen[3] = vec2(-0.75, 0.75);

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::drawCube(){
	perspective();
	//transform each cube vertex into 2D
	for (int i = 0; i < 8; i++){
		vec4 temp = view * look * scaler * model * vec4(m_cube3D[i],1);
		bool draw = true;
		//float z = temp[2];
		temp = proj * temp;

		m_cube2D[i] = normalize(temp);
	}

	// Draw outer square:
	setLineColour(vec3(1.0f, 0.7f, 0.8f));
	checkDraw(m_cube2D[4],m_cube2D[5]);
	checkDraw(m_cube2D[5], m_cube2D[6]);
	checkDraw(m_cube2D[6], m_cube2D[7]);
	checkDraw(m_cube2D[7],m_cube2D[4]);

	//sides
	checkDraw(m_cube2D[4],m_cube2D[0]);
	checkDraw(m_cube2D[5], m_cube2D[1]);
	checkDraw(m_cube2D[6], m_cube2D[2]);
	checkDraw(m_cube2D[7],m_cube2D[3]);

	// Draw inner square:
	setLineColour(vec3(0.2f, 1.0f, 1.0f));
	checkDraw(m_cube2D[0],m_cube2D[1]);
	checkDraw(m_cube2D[1], m_cube2D[2]);
	checkDraw(m_cube2D[2], m_cube2D[3]);
	checkDraw(m_cube2D[3], m_cube2D[0]);

}

glm::vec2 A2::normalize(glm::vec4 &point){
	float w = point[3];
	float z = point[2];
	float x = point[0]/z;//*z/w;
	float y = point[1]/z;//*z/w;

	return vec2(x,y);	
}

/*void A2::lookAt(const glm::vec3 &lookAt, const glm::vec3 &lookFrom, const glm::vec3 &up) {
	vec3 vz = glm::normalize(lookAt - lookFrom);
	vec3 vx  = glm::normalize(glm::cross(up, vz));
	vec3 vy = glm::cross(vz, vx);

	mat4 temp;
	look[0] = vec4(vz, 0);
	look[1] = vec4(vy, 0);
	look[2] = vec4(vz, 0);
	look[3] = vec4(0, 0, 0, 1);
}*/


void A2::perspective(){
	float cot = 1.0/tan(glm::radians(m_fov)/2);
	
	proj[0] = vec4(cot/aspect, 0, 0, 0);
	proj[1] = vec4(0, cot, 0, 0);
	proj[2] = vec4(0, 0, -1*(m_far + m_near)/(m_far - m_near), -1);//-2*m_far*m_near/(m_far - m_near));
	proj[3] = vec4(0, 0, -2*m_far*m_near/(m_far-m_near),0);

}

void A2::rotate(float amount){
	float theta = radians(amount);
	
	mat4 rotateX;
	mat4 rotateY;
	mat4 rotateZ;
	if (m_movingX){
		rotateX[0] = vec4(1, 0, 0, 0);
		rotateX[1] = vec4(0, cos(theta), sin(theta), 0);
		rotateX[2] = vec4(0, -sin(theta), cos(theta), 0);
		rotateX[3] = vec4(0, 0, 0, 1);
		if (m_activeCoord == MODEL){
			model = rotateX*model;
		} else if (m_activeCoord == VIEW){
			view = rotateX*view;
		}
	}

	if (m_movingY){
		rotateY[0] = vec4(cos(theta), 0, -sin(theta), 0);
		rotateY[1] = vec4(0, 1, 0, 0);
		rotateY[2] = vec4(sin(theta), 0, cos(theta), 0);
		rotateY[3] = vec4(0, 0, 0, 1);
		if (m_activeCoord == MODEL){
			model = rotateY*model;
		} else if (m_activeCoord == VIEW){
			view = rotateY*view;
		}
	}	

	if (m_movingZ){
		rotateZ[0] = vec4(cos(theta), sin(theta), 0, 0);
		rotateZ[1] = vec4(-sin(theta), cos(theta), 0, 0);
		rotateZ[2] = vec4(0, 0, 1, 0);
		rotateZ[3] = vec4(0, 0, 0, 1);
		if (m_activeCoord == MODEL){
			model = rotateZ*model;
		} else if (m_activeCoord == VIEW){
			view = rotateZ*view;
		}
	}


}

void A2::translate(float amount){
	float x = 0;
	float y = 0;
	float z = 0;

	mat4 translate;
	if (m_movingX) x = amount;
	if (m_movingY) y = amount;
	if (m_movingZ) z = amount;

	//cout << amount << endl;

	translate[0] = vec4(1, 0, 0, 0);
	translate[1] = vec4(0, 1, 0, 0);
	translate[2] = vec4(0, 0, 1, 0);
	translate[3] = vec4(x, y, z, 1);

	if (m_activeCoord == MODEL){
		model = translate*model;
	} else if (m_activeCoord == VIEW){
		view = translate*view;
	} 
}

void A2::scale(float amount){

	if (amount > 0){
		amount = 1.1;
	} else if (amount < 0) {
		amount = 0.9;
	} else {
		amount = 1;
	}
	
	float x = 1;
	float y = 1;
	float z = 1;

	if (m_movingX) x = amount;
	if (m_movingY) y = amount;
	if (m_movingZ) z = amount;

	mat4 temp;
	temp[0] = vec4(x, 0, 0, 0);
	temp[1] = vec4(0, y, 0, 0);
	temp[2] = vec4(0, 0, z, 0);
	temp[3] = vec4(0, 0, 0, 1);

	scaler = temp*scaler;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}


bool A2::clipPlane (
		glm::vec3 & v0,
		glm::vec3 & v1,
		glm::vec3  p,
		glm::vec3  n
){

	float wecA = glm::dot(v0 - p, n);
	float wecB = glm::dot(v1 - p, n);
	//cout << "v0 = " << v0 << " v1 = " << v1 <<endl;
	//cout << "p = " << p << " n = " << n <<endl;;
	//cout << "wecA = " << wecA << ", wecB = " << wecB << endl;

	if (wecA < 0 && wecB < 0 ) return false; //reject
	if (wecA >= 0 && wecB >= 0) return true; //trivial accept
	float t = wecA/(wecA - wecB);

	if (wecA < 0){
		v0 = v0 + t*(v1 - v0);
		//cout << v0 << endl;
	} else {
		v1 = v0 + t*(v1 - v0);
		//cout << v1 << endl;
	}

	return true;
}

void A2::checkDraw (
		glm::vec2 v0,
		glm::vec2 v1
) {
	bool draw = true;

	if (refreshViewport){
	vec3 A = vec3(v0, 0);
	vec3 B = vec3(v1, 0);

	//cout << "v0 = " << v0 << " v1 = " << v1 <<endl;
	//cout << "screen 1 clipping" <<endl;
	draw &= clipPlane(A, B, vec3(m_screen[0],0), vec3(0, 1, 0)); //top
	draw &= clipPlane(A, B, vec3(m_screen[1],0), vec3(-1, 0, 0)); //right
	draw &= clipPlane(A, B, vec3(m_screen[2],0), vec3(0, -1, 0)); //bottom
	draw &= clipPlane(A, B, vec3(m_screen[3],0), vec3(1, 0, 0)); //left

	v0[0] = A[0];
	v0[1] = A[1];
	v1[0] = B[0];
	v1[1] = B[1];
	}

	if (!draw) return;

	//cout << " drawing (" << v0 << ") to (" << v1 <<")"<<endl;
	drawLine(v0, v1);
}
//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	drawCube();

	//DRAW GNOMONS

	//model
	for (int i = 0; i < 4; i++){
		vec4 temp = proj * view * model * vec4(m_gnomon3D[i], 1);
		m_gnomon2D[i] = normalize(temp);
	}

	setLineColour(vec3(1.0f,0,0));
	checkDraw(m_gnomon2D[0], m_gnomon2D[1]);
	setLineColour(vec3(0, 0, 1.0f));
	checkDraw(m_gnomon2D[0], m_gnomon2D[2]);
	setLineColour(vec3(0, 1.0f, 0));
	checkDraw(m_gnomon2D[0], m_gnomon2D[3]);

	for (int i = 0; i < 4; i++){
		vec4 temp = proj * view * vec4(m_gnomon3D[i], 1);
		m_gnomon2D[i] = normalize(temp);
	}

	//world
	setLineColour(vec3(0.5f,0,0));
	checkDraw(m_gnomon2D[0], m_gnomon2D[1]);
	setLineColour(vec3(0, 0, 0.5f));
	checkDraw(m_gnomon2D[0], m_gnomon2D[2]);
	setLineColour(vec3(0, 0.5f, 0));
	checkDraw(m_gnomon2D[0], m_gnomon2D[3]);

	//DRAW VIEWPORT
	setLineColour(vec3(0.0));
	drawLine((m_screen[0]), (m_screen[1]));
	drawLine((m_screen[1]), (m_screen[2]));
	drawLine((m_screen[2]), (m_screen[3]));
	drawLine((m_screen[3]), (m_screen[0]));

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		
		if( ImGui::Button( "Reset" ) ) {
			reset();
		}

		if( ImGui::RadioButton( "Rotate Model", &temp, 0 ) ) {
			m_activeMode = ROTATE;
			m_activeCoord = MODEL;
		}

		if( ImGui::RadioButton( "Translate Model", &temp, 1 ) ) {
			m_activeMode = TRANSLATE;
			m_activeCoord = MODEL;
		}

		if( ImGui::RadioButton( "Scale Model", &temp, 2 ) ) {
			m_activeMode = SCALE;
			m_activeCoord = MODEL;
		}

		if( ImGui::RadioButton( "Rotate View", &temp, 3 ) ) {
			m_activeMode = ROTATE;
			m_activeCoord = VIEW;
		}

		if( ImGui::RadioButton( "Translate View", &temp, 4 ) ) {
			m_activeMode = TRANSLATE;
			m_activeCoord = VIEW;
		}

		if( ImGui::RadioButton( "Change FOV", &temp, 5 ) ) {
			m_activeMode = FOV;
			m_activeCoord = PERSP;
		}

		if( ImGui::RadioButton( "Viewport", &temp, 6 ) ) {
			m_activeMode = VIEWPORT;
			m_activeCoord = SCREEN;
		}

		ImGui::SliderFloat("Near Plane", &m_near, 1.0f, std::min(10.0f,m_far - 1.0f));
		ImGui::SliderFloat("Far Plane", &m_far, std::max(m_near + 1.0f, 2.0f), 20.0f);


		std::string near = "";
		std::string far = "";

		for (int i = 0; i < m_near; i++){
			near = near + " ";
		}

		for (int i = 0; i < m_far - m_near; i++){
			far = far + "_";
		}

		ImGui::Text( "<) %s|%s|", near.c_str(), far.c_str() );

		ImGui::Text( "FOV: %.1f DEG", m_fov);

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	float delta = (xPos-m_mouseX);
	refreshViewport = true;

	if (m_activeMode == ROTATE){
		rotate(0.1*delta);
		eventHandled = true;
	} else if (m_activeMode == TRANSLATE){
		translate(0.01*delta);
		eventHandled = true;
	} else if (m_activeMode == SCALE){
		scale(delta);
	} else if (m_activeMode == FOV && m_movingX){
		m_fov += delta;

		m_fov = std::max(1.0f, m_fov);
		m_fov = std::min(360.0f, m_fov);
		
	} else if (m_activeMode == VIEWPORT && (m_movingX || endDrag)){
		refreshViewport = false;
		if (beginDrag){
			topLeft = vec2(xPos, yPos);
			beginDrag = false;
		}
		bottomRight = vec2(xPos, yPos);
		//endDrag = false;
		if (endDrag){
			if (bottomRight[0] < topLeft[0] || bottomRight[1] < topLeft[1]){
				//cout << "reversed" << endl;
				vec2 oldTL = topLeft;
				vec2 oldBR = bottomRight;
				topLeft = vec2(std::min(oldTL[0], oldBR[0]),
						std::min(oldTL[1], oldBR[1]));
				bottomRight = vec2(std::max(oldTL[0], oldBR[0]),
						std::max(oldTL[1], oldBR[1]));
			}
			endDrag = false;
			refreshViewport = true;
		}
		//cout << "TL = " << topLeft << " BR = " << bottomRight << endl;
		m_screen[3] = toGLCoord(topLeft[0], topLeft[1]);		
		m_screen[2] = toGLCoord(bottomRight[0], topLeft[1]);
		m_screen[1] = toGLCoord(bottomRight[0], bottomRight[1]);
		m_screen[0] = toGLCoord(topLeft[0], bottomRight[1]);
	}

	m_mouseX = xPos;
	m_mouseY = yPos;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (actions == GLFW_PRESS){
		//temp
		//m_translating = true;
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (m_activeMode == VIEWPORT){
				beginDrag = true;
				endDrag = false;
			}
			m_movingX = true;
			eventHandled = true;
		} else if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			m_movingY = true;
			eventHandled = true;
		} else if (button == GLFW_MOUSE_BUTTON_RIGHT){
			m_movingZ = true;
			eventHandled = true;
		}
	}

	if (actions == GLFW_RELEASE){
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (m_activeMode == VIEWPORT){
				endDrag = true;
				beginDrag = false;
			}
			m_movingX = false;
			eventHandled = true;
		} else if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			m_movingY = false;
		} else if (button == GLFW_MOUSE_BUTTON_RIGHT){
			m_movingZ = false;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	aspect = (float)width/(float)height;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if ( action == GLFW_PRESS){

		if( key == GLFW_KEY_Q ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}else if( key == GLFW_KEY_A) {
			reset();
		}else if (key == GLFW_KEY_O){
			m_activeMode = ROTATE;
			m_activeCoord = VIEW;
			temp = 3;
		} else if (key == GLFW_KEY_N) {
			m_activeMode = TRANSLATE;
			m_activeCoord = VIEW;
			temp = 4;
		} else if (key == GLFW_KEY_P){
			m_activeCoord = PERSP;
			m_activeMode = FOV;
			temp = 5;
		} else if (key == GLFW_KEY_R){
			m_activeMode = ROTATE;
			m_activeCoord = MODEL;
			temp = 0;
		} else if (key == GLFW_KEY_T){
			m_activeMode = TRANSLATE;
			m_activeCoord = MODEL;
			temp = 1;
		} else if (key == GLFW_KEY_S){
			m_activeMode = SCALE;
			m_activeCoord = MODEL;
			temp = 2;
		} else if (key == GLFW_KEY_V){
			temp = 6;
			m_activeMode = VIEWPORT;
			m_activeCoord = SCREEN;
		}

	}

	return eventHandled;
}
