#include "Project.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"

#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stack>
#include <algorithm>

#define RENDER_HITBOX false

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

static const size_t DIM = 8;


//----------------------------------------------------------------------------------------
// Constructor
Project::Project(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  m_mouseX(0.0),
	  m_mouseY(0.0),
	  tempMode(0),
	  m_mode(Mode::POSITION),
	  m_zbuffer(true),
	  m_backfaceCulling(false),
	  m_frontfaceCulling(false),
	  m_useAlpha(false),
	  lmb_down(false),
	  mmb_down(false),
	  rmb_down(false),
	  m_translation(mat4()),
	  m_rotation(mat4()),
	  m_shotX(0),
	  m_shotY(0),
	  m_playerX(0),
	  m_playerY(0),
	  m_playerNode(nullptr)
{

}

//----------------------------------------------------------------------------------------
// Destructor
Project::~Project()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void Project::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj"),
			getAssetFilePath("plane.obj"),
			getAssetFilePath("player.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	findPlayerNode((SceneNode*)&*m_rootNode);

	if (m_playerNode != nullptr){
		cout << "Player found" << endl;
	}

	findPlaneNode((SceneNode*)&*m_rootNode);

	if (m_plane != nullptr){
		cout << "Plane found" << endl;
	}

	findEnemyNodes((SceneNode*)&*m_rootNode);

	findSpecialObjects((SceneNode*)&*m_rootNode);

	Bounds bounds(m_plane->hitbox->_pos, m_plane->hitbox->_maxXYZ);
	m_collisionTree = new CollisionTreeNode(bounds, 0);
	m_collisionTree->construct((SceneNode*)&*m_rootNode);
	/*if (m_playerNode != nullptr){
		cout << "Player found" << endl;
	}*/
	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void Project::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void Project::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void Project::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void Project::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void Project::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void Project::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void Project::initViewMatrix() {
	m_view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );/*glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));*/
}

//----------------------------------------------------------------------------------------
void Project::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void Project::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i( location, m_picking ? 1 : 0 );
		CHECK_GL_ERRORS;

		if (!m_picking){
			//-- Set LightSource uniform for the scene:
			{
				location = m_shader.getUniformLocation("light.position");
				glUniform3fv(location, 1, value_ptr(m_light.position));
				location = m_shader.getUniformLocation("light.rgbIntensity");
				glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
				CHECK_GL_ERRORS;
			}

			//-- Set background light ambient intensity
			{
				location = m_shader.getUniformLocation("ambientIntensity");
				vec3 ambientIntensity(0.05f);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void Project::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void Project::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Application", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset Position" ) ) {
			resetPosition();
		}

		if( ImGui::Button( "Reset Orientation" ) ) {
			resetOrientation();
		}

		if( ImGui::Button( "Reset Joints" ) ) {
			resetJoints();
		}

		if( ImGui::Button( "Reset All" ) ) {
			resetAll();
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	ImGui::Begin("Edit", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);
		if( ImGui::Button( "Undo" ) ) {
			undo();
		}

		if( ImGui::Button( "Redo" ) ) {
			redo();
		}

	ImGui::End();

	ImGui::Begin("Options", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);
			

		if( ImGui::Checkbox( "Z-buffer", &m_zbuffer) ) {
			
		}

		if( ImGui::Checkbox( "Backface Culling", &m_backfaceCulling) ) {
			
		}
		
		if( ImGui::Checkbox( "Frontface Culling", &m_frontfaceCulling) ) {
			
		}

	ImGui::End();

	
	
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix, bool picking
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;
			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;


			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			vec3 kd = node.isSelected ? glm::vec3(1.0f) : node.material.kd;
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.ks");
			vec3 ks = node.isSelected ? glm::vec3(1.0f) : node.material.ks;
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, node.isSelected ? 0.0f :node.material.shininess);
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.alpha");
			glUniform1f(location, node.material.alpha);
		}

	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void Project::draw() {

	m_view = m_translation * m_rotation * glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	if (m_zbuffer)
		glEnable( GL_DEPTH_TEST );

	if (m_backfaceCulling || m_frontfaceCulling){
		glEnable(GL_CULL_FACE);
		if (m_backfaceCulling && m_frontfaceCulling){
			glCullFace(GL_FRONT_AND_BACK);
		} else if (m_backfaceCulling){
			glCullFace(GL_BACK);
		} else if (m_frontfaceCulling){
			glCullFace(GL_FRONT);
		}
	}

	renderSceneGraph(*m_rootNode);

	glBindVertexArray(m_vao_meshData);
	renderTransparentObjects((SceneNode *) &*m_rootNode);
	glBindVertexArray(0);
	CHECK_GL_ERRORS;

	if (m_zbuffer)
		glDisable( GL_DEPTH_TEST );

	if (m_backfaceCulling || m_frontfaceCulling){
		glDisable(GL_CULL_FACE);
	}
}

//----------------------------------------------------------------------------------------
void Project::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	renderNodes((SceneNode *) &root);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

void Project::renderNodes(SceneNode *root, bool picking){

	if (root->m_nodeType == NodeType::GeometryNode){
		GeometryNode * geometryNode = static_cast<GeometryNode *>(root);


		if (!geometryNode->isTransparent()){
			updateShaderUniforms(m_shader, *geometryNode, m_view, m_picking);

			BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

			//-- Now render the mesh:
			m_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

			m_shader.disable();

			if (RENDER_HITBOX) renderHitbox(geometryNode);
		} 

		/*if (geometryNode->m_name == "player") {
			cout << "player: " << geometryNode->_hitbox->pos << endl;
		}*/
	}
	for (SceneNode *child : root->children){
		child->set_transform(root->get_transform() * child->get_transform());
		renderNodes(child);
		child->set_transform(glm::inverse(root->get_transform()) * child->get_transform());
	}
}

void Project::renderTransparentObjects(SceneNode *root){

	if (root->m_nodeType == NodeType::GeometryNode){
		GeometryNode * geometryNode = static_cast<GeometryNode *>(root);

		if (geometryNode->isTransparent()){
			glEnable(GL_BLEND);

			glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
			glBlendEquation(GL_FUNC_ADD);

			updateShaderUniforms(m_shader, *geometryNode, m_view, m_picking);

			BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

			//-- Now render the mesh:
			m_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

			m_shader.disable();

			glDisable(GL_BLEND);

			if (RENDER_HITBOX) renderHitbox(geometryNode);
		}

	}
	for (SceneNode *child : root->children){
		child->set_transform(root->get_transform() * child->get_transform());
		renderTransparentObjects(child);
		child->set_transform(glm::inverse(root->get_transform()) * child->get_transform());
	}
}

void Project::renderHitbox(GeometryNode *node){
	m_shader.enable();
	GLint location = m_shader.getUniformLocation("ModelView");
	mat4 scale_mat = glm::scale(mat4(), vec3(node->hitbox->_maxXYZ)); //* mat4();
	mat4 modelView = m_view * node->trans * scale_mat; 
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
	CHECK_GL_ERRORS;

		//-- Set NormMatrix:
	location = m_shader.getUniformLocation("NormalMatrix");
	mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
	glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
	CHECK_GL_ERRORS;

	//-- Set Material values:
	location = m_shader.getUniformLocation("material.kd");
	vec3 kd = glm::vec3(0.6f, 1.0f, 0.8f);
	glUniform3fv(location, 1, value_ptr(kd));
	CHECK_GL_ERRORS;
	
	location = m_shader.getUniformLocation("material.ks");
	vec3 ks = glm::vec3(0.6f, 1.0f, 0.8f);
	glUniform3fv(location, 1, value_ptr(ks));
	CHECK_GL_ERRORS;
	location = m_shader.getUniformLocation("material.shininess");
	glUniform1f(location, 0.0f);
	CHECK_GL_ERRORS;
	
	m_shader.disable();

	BatchInfo hitbox = m_batchInfoMap["cube"];

	m_shader.enable();
	glDrawArrays(GL_LINES, hitbox.startIndex, hitbox.numIndices);
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
void Project::findPlayerNode(SceneNode *root){
	if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "player"){
		m_playerNode = static_cast<GeometryNode *>(root);
		return;
	}
	for (SceneNode *child : root->children){
		findPlayerNode(child);
	}
}

//----------------------------------------------------------------------------------------
void Project::findPlaneNode(SceneNode *root){
	if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "plane"){
		m_plane = static_cast<GeometryNode *>(root);
		return;
	}
	for (SceneNode *child : root->children){
		findPlaneNode(child);
	}
}

//----------------------------------------------------------------------------------------
void Project::findEnemyNodes(SceneNode *root){
	if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "e1"){
		m_enemy1 = static_cast<GeometryNode *>(root);
	} else if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "e2"){
		m_enemy2 = static_cast<GeometryNode *>(root);
	}

	if (m_enemy1 != nullptr && m_enemy2 != nullptr) return;
	for (SceneNode *child : root->children){
		findEnemyNodes(child);
	}
}

//----------------------------------------------------------------------------------------
void Project::findSpecialObjects(SceneNode *root){
	if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "t1"){
		m_transparentNode = static_cast<GeometryNode *>(root);
		m_transparentNode->setTransparency(0.3f);
	} else if (root->m_nodeType == NodeType::GeometryNode && root->m_name.size() == 2 && root->m_name[0] == 't'){
		GeometryNode* node = static_cast<GeometryNode *>(root);
		node->setTransparency(0.3f);
	} else if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "r1"){
		m_reflectNode = static_cast<GeometryNode *>(root);
	}

	if (m_transparentNode != nullptr && m_reflectNode != nullptr) return;
	for (SceneNode *child : root->children){
		findSpecialObjects(child);
	}
}


void Project::resetOrientation(){
	m_rotation = mat4();
}

void Project::resetPosition(){
	m_translation = mat4();
}

//reset& deselects joints, have to manually clear selection vector
void Project::resetJoints(){	
	while(!m_undoStack.empty()){
		Command* cmd = m_undoStack.back();
		m_undoStack.pop_back();
		cmd->execute(-1);
	}
	std::vector<SceneNode*>::iterator it = m_selectedJoints.begin();
	for (it; it != m_selectedJoints.end(); ++it){
		JointNode * jointNode = static_cast<JointNode *>(*it);
		jointNode->resetJoint();
	}
	deselectJoints((SceneNode*)&*m_rootNode);
	m_selectedJoints.clear();
	m_undoStack.clear();
	m_redoStack.clear();
}

void Project::deselectJoints(SceneNode *root){	
	root->isSelected = false;
	for (SceneNode *child : root->children){
		deselectJoints(child);
	}
}

void Project::resetAll(){
	resetOrientation();
	resetPosition();
	resetJoints();
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void Project::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool Project::cursorEnterWindowEvent (
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
bool Project::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	if (ImGui::IsMouseHoveringAnyWindow()) return eventHandled;

	float deltaX = -(xPos - m_mouseX);
	float deltaY = yPos - m_mouseY;
	float modifier = 0.5;

	if (mmb_down){
		m_shotX += deltaX;
		m_shotY += deltaY;
		
		rotateShot(modifier*deltaX);
	}

	m_mouseX = xPos;
	m_mouseY = yPos;
	return eventHandled;
}

void Project::moveJoints(SceneNode *root, float x, float y){
	if (mmb_down){
		std::vector<SceneNode*>::iterator it = m_selectedJoints.begin();
		for (it; it != m_selectedJoints.end(); ++it){
			JointNode * jointNode = static_cast<JointNode *>(*it);
			if((*it)->m_name == "leftElbow-hand" 
				|| (*it)->m_name == "rightElbow-hand"
				|| (*it)->m_name ==  "leftArm-elbow"
				|| (*it)->m_name ==  "rightArm-elbow"){
				
				jointNode->rotate('y', x);
			} else {
				jointNode->rotate('x', x);
			}
			
		}
	}

	if (rmb_down){
		//m_curCmd->_neckY += y;
		std::vector<SceneNode*>::iterator it = m_selectedJoints.begin();
		for (it; it != m_selectedJoints.end(); ++it){
			if ((*it)->m_name == "neckJoint"){
				JointNode * jointNode = static_cast<JointNode *>(*it);
				jointNode->rotate('y', y);
			}
		}
	}
}

void Project::select(SceneNode *node){
	if (node->isSelected) {
		cout << "selected " << *node << endl;
		m_selectedJoints.emplace_back(node);
	} else {
		auto it = std::find(m_selectedJoints.begin(), m_selectedJoints.end(), node);
		if (it != m_selectedJoints.end()){
			cout << "deselected " << *node << endl;
			m_selectedJoints.erase(it);
		}
	}

}

void Project::undo(){
	if (m_undoStack.empty()) return;

	Command* cmd = m_undoStack.back();
	m_undoStack.pop_back();
	cmd->execute(-1);
	m_redoStack.emplace_back(cmd);
}

void Project::redo(){
	if (m_redoStack.empty()) return;

	Command* cmd = m_redoStack.back();
	m_redoStack.pop_back();
	cmd->execute(1);
	m_undoStack.emplace_back(cmd);

}


void Project::movePlayer(double x, double z){
	dvec3 transl(x, 0.0, z);
	m_playerNode->translate(transl);
	std::vector<GeometryNode*> collisions;
	std::vector<vec3> axis;
	m_collisionTree->collideGeometry(m_playerNode, collisions, axis, false);
	bool adjust(false);
	dvec3 transl_back(0.0);
	cout << "collisions: " << collisions.size() << endl;
	for (int i = 0; i < collisions.size(); i++){
		GeometryNode* collision = collisions[i];
//		cout << collision->m_name << endl;
		if (collision == m_plane || collision == m_playerNode) continue;
		adjust = true;
		transl_back.x += axis[i].x * x;
		transl_back.z += axis[i].z * z;
	}

	if (adjust){
		//cout << glm::to_string(transl_back) << endl;
		m_playerNode->translate(dvec3(-x, 0.0, -z));
	}
}

void Project::rotateShot(double x){
	mat4 temp = m_playerNode->get_transform();
	m_playerNode->set_transform(mat4());
	m_playerNode->rotate('y', x);
	//m_playerNode->rotate('z', z);
	m_playerNode->set_transform(temp * m_playerNode->get_transform());
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool Project::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (ImGui::IsMouseHoveringAnyWindow()) return eventHandled;
	// Fill in with event handling code...
	if (actions == GLFW_PRESS){
		if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			mmb_down = true;
			eventHandled = true;
		}
	}

	if (actions == GLFW_RELEASE){
		if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			mmb_down = false;
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool Project::mouseScrollEvent (
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
bool Project::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool Project::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	double moveX = 0;
	double moveZ = 0;

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		} else if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		} else if (key == GLFW_KEY_Z){
			m_zbuffer = !m_zbuffer;
		} else if (key == GLFW_KEY_B){
			m_backfaceCulling = !m_backfaceCulling;
		} else if (key == GLFW_KEY_F){
			m_frontfaceCulling = !m_frontfaceCulling;
		} 

		if (key == GLFW_KEY_W){
			moveZ -= 0.3;
		} else if (key == GLFW_KEY_S){
			moveZ += 0.3;
		} 
		if (key == GLFW_KEY_A){
			moveX -= 0.3;
		} else if (key == GLFW_KEY_D){
			moveX += 0.3;
		}

		if (moveX != 0 || moveZ != 0){
			movePlayer(moveX, moveZ);
		}
	}

	else if (action == GLFW_REPEAT){
		if (key == GLFW_KEY_W){
			moveZ -= 0.3;
		} else if (key == GLFW_KEY_S){
			moveZ += 0.3;
		} 
		if (key == GLFW_KEY_A){
			moveX -= 0.3;
		} else if (key == GLFW_KEY_D){
			moveX += 0.3;
		}

		if (moveX != 0 || moveZ != 0){
			movePlayer(moveX, moveZ);
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
