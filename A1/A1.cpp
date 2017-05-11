#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <vector>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
const float PI = 3.14159265f;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ),
	_grid(DIM),
	_x(0),
	_z(0),
	_inCopyMode(false),
	_inRotateMode(false),
	_mouseX(0),
	_mouseY(0),
	_rotateX(0),
	_rotateY(0),
	_zoom( 0 ),
	_zoomMultiplier(float(DIM)*2.0*M_SQRT1_2)
{
	colour[0][0] = 0.5f;
	colour[0][1] = 0.5f;
	colour[0][2] = 0.5f;

	colour[1][0] = 0.1f;
	colour[1][1] = 0.5f;
	colour[1][2] = 0.5f;

	colour[2][0] = 0.5f;
	colour[2][1] = 0.1f;
	colour[2][2] = 0.5f;

	colour[3][0] = 0.5f;
	colour[3][1] = 0.5f;
	colour[3][2] = 0.1f;

	colour[4][0] = 0.1f;
	colour[4][1] = 0.1f;
	colour[4][2] = 0.5f;

	colour[5][0] = 0.5f;
	colour[5][1] = 0.1f;
	colour[5][2] = 0.1f;

	colour[6][0] = 0.1f;
	colour[6][1] = 0.5f;
	colour[6][2] = 0.1f;

	colour[7][0] = 0.18f;
	colour[7][1] = 0.31f;
	colour[7][2] = 0.31f;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );

	orig_proj = proj;

}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}


void A1::drawCube(int x, int z){
	//std::vector<GLfloat*> cube_vertices;
	int y = _grid.getHeight(x,z);
	int x1 = x+1;
	int z1 = z+1;
	if (y <= 0) return;

	GLfloat cube_vertices[] = {
		x, 0, z,
   		x, 0, z1,
    	x, y, z1,	
    	x1, y, z,
    	x, 0, z,
    	x, y, z,
    	x1, 0, z1,
    	x, 0, z,
    	x1, 0, z,
    	x1, y, z,
    	x1, 0, z,
    	x, 0, z,
    	x, 0, z,
    	x, y, z1,
    	x, y, z,
    	x1, 0, z1,
    	x, 0, z1,
    	x, 0, z,
    	x, y, z1,
    	x, 0, z1,
    	x1, 0, z1,
    	x1, y, z1,
    	x1, 0, z,
    	x1, y, z,
    	x1, 0, z,
    	x1, y, z1,
    	x1, 0, z1,
    	x1, y, z1,
    	x1, y, z,
    	x, y, z,
    	x1, y, z1,
    	x, y, z,
    	x, y, z1,
    	x1, y, z1,
    	x, y, z1,
    	x1, 0, z1
	};

		GLuint vbo;
		GLuint vao;
		glGenVertexArrays( 1, &vao );
		glBindVertexArray( vao );

		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof(cube_vertices),
			cube_vertices, GL_STATIC_DRAW );

		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		int index = _grid.getColour(x, z);

		glUniform3f(col_uni, colour[index][0],  colour[index][1],
				 colour[index][2]);

		glDrawArrays( GL_TRIANGLES, 0, 36);

		if (x == _x && z == _z && y > 0){
			glUniform3f(col_uni, 0.0f, 0.0f, 0.0f);
			glDrawArrays( GL_LINES, 0, 36);
		}

		CHECK_GL_ERRORS;
}

void A1::highlightCells(int x, int x1, float y, int z, int z1, float r, float g, float b){
	glUniform3f(col_uni, r, g, b);
	GLfloat vertext_buf[] = {
		x, y, z,
		x, y, z1,
		x1, y, z1,
		x, y, z,
		x1, y, z1,
		x1, y, z
	};

	GLuint vbo;
	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertext_buf),
			vertext_buf, GL_STATIC_DRAW );

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays( GL_TRIANGLES, 0, 12);
	CHECK_GL_ERRORS;
	
}

void A1::drawIndicator(){
	int y = _grid.getHeight(_x, _z);
	
	float indY = y + 0.1;

	highlightCells(_x, _x + 1, indY, _z, _z+1, 0.1, 0.1, 0.1);

	highlightCells(-1, _x, -0.1, _z, _z+1, 0.9, 0.9, 0.9);
	highlightCells(_x+1, DIM+1, -0.1, _z, _z+1, 0.9, 0.9, 0.9);
	highlightCells(_x, _x+1, -0.1, -1, _z, 0.9, 0.9, 0.9);	
	highlightCells(_x, _x+1, -0.1, _z+1, DIM+1, 0.9, 0.9, 0.9);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset" ) ) {
			reset();
		}
		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		for (int i = 0; i < 8; i++) {
			ImGui::PushID( i );
			ImGui::ColorEdit3( "##Colour", colour[i] );
			ImGui::SameLine();
			if( ImGui::RadioButton( "##Col", &current_col, i ) ) {
				// Select this colour.
				_grid.setColour(_x, _z, current_col);
			}
			ImGui::PopID();
		}


/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	float theta = glm::radians(_rotateX);
	float phi = glm::radians(_rotateY);
	view = glm::lookAt(glm::vec3(_zoomMultiplier*sin(theta), _zoomMultiplier*cos(phi), _zoomMultiplier*cos(theta)),
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0));

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		//grid.setHeight(1, 1, 2);
		//height[1][1] = 2;
		for (int i = 0; i < DIM; i++)
			for (int j = 0; j < DIM; j++)
				drawCube(i, j);

		// Highlight the active square.
		drawIndicator();
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.

		if (_inRotateMode) {
			double deltaX = (xPos - _mouseX)*0.2;
			double deltaY = (yPos - _mouseY)*0.1;
			_rotateX += deltaX;
			_rotateY += deltaY;

			float theta = glm::radians(_rotateX);
			float phi = glm::radians(_rotateY);
			
			eventHandled = true;
		}

		_mouseX = xPos;
		_mouseY = yPos;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (actions == GLFW_PRESS){
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				_inRotateMode = true;
				eventHandled = true;
			}
		}
	}

	if (actions == GLFW_RELEASE){
		 if (button == GLFW_MOUSE_BUTTON_LEFT) {
			_inRotateMode = false;
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	_zoom += yOffSet > 0? -1 : 1;


	if (_zoom >= 0 && _zoom <= 40){
		float theta = glm::radians(_rotateX);
		float phi = glm::radians(_rotateY);

		_zoomMultiplier += yOffSet*0.5;

		//cout << "_zoom = " << _zoom << endl;
		eventHandled = true;
	} else {
		_zoom = std::min(_zoom, 40);
		_zoom = std::max(_zoom, 0);
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);
	int prevX = _x, prevZ = _z;

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.

		if (key == GLFW_KEY_R){
			reset();
			eventHandled = true;
		} else if (key == GLFW_KEY_SPACE){
			_grid.setHeight(_x,_z, _grid.getHeight(_x,_z) + 1);
			_grid.setColour(_x, _z, current_col);
			eventHandled = true;
		} else if (key == GLFW_KEY_BACKSPACE) {
			_grid.setHeight(_x,_z, std::max(0,_grid.getHeight(_x,_z) - 1));
			eventHandled = true;
		} else if (key == GLFW_KEY_RIGHT) {
			_x = (++_x) % DIM;
			if (_inCopyMode){
				_grid.setColour(_x,_z, _grid.getColour(prevX, _z));
				_grid.setHeight(_x,_z, _grid.getHeight(prevX,_z));
			}
			eventHandled = true;
		} else if (key == GLFW_KEY_LEFT ){
			_x = (--_x) % DIM;
			if (_inCopyMode){
				_grid.setColour(_x,_z, _grid.getColour(prevX, _z));
				_grid.setHeight(_x,_z, _grid.getHeight(prevX,_z));
			}
			eventHandled = true;
		} else if (key == GLFW_KEY_UP ){
			_z = (--_z) % DIM;
			if (_inCopyMode){
				_grid.setColour(_x,_z, _grid.getColour(_x, prevZ));
				_grid.setHeight(_x,_z, _grid.getHeight(_x,prevZ));
			}
			eventHandled = true;
		} else if (key == GLFW_KEY_DOWN ){
			_z = (++_z) % DIM;
			if (_inCopyMode){
				_grid.setColour(_x,_z, _grid.getColour(_x, prevZ));
				_grid.setHeight(_x,_z, _grid.getHeight(_x,prevZ));
			}
			eventHandled = true;
		} else if (key == GLFW_KEY_LEFT_SHIFT ){
			_inCopyMode = true;
			eventHandled = true;
		}
	}

	if( action == GLFW_RELEASE ) {
		if (key == GLFW_KEY_LEFT_SHIFT) {
			_inCopyMode = false;
			eventHandled = true;
		}
	}

	return eventHandled;
}

void A1::reset(){

	current_col = 0;
	_rotateX = 0;
	_rotateY = 0;
	_zoom = 0;
	_mouseX = 0;
	_mouseY = 0;
	_inCopyMode = false;
	_inRotateMode = false;
	_x = 0;
	_z = 0;
	_grid.reset();
	_zoomMultiplier = float(DIM)*2.0*M_SQRT1_2;

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = orig_proj;

}
