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
#include <limits>
#include <cstdlib>

#define RENDER_HITBOX false

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

static const size_t DIM = 8;

//clock_t m_start_time = 0;
//clock_t m_current_time = 0;
//float m_current_time_secs = 0;

//----------------------------------------------------------------------------------------
// Constructor
Project::Project(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0), 
	  m_textureAttrribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vbo_vertexUV(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  //m_vao_particle(0),
	  //m_vbo_particle(0),
	  m_mouseX(0.0),
	  m_mouseY(0.0),
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
	  m_playerNode(nullptr),
	  m_framebuffer(0),
	  m_shadowMap(0),
	  m_shadowView(mat4()),
	  m_doShadowMapping(false),
	  m_drawReflection(false),
	  m_drawTexture(false),
	  m_reflectedView(mat4()),
	  m_texture(0),
	  e(rd()),
	  dis(0,2),
	  shotId(0),
	  lives(3)
	  //particleCount(0),
	  //lastUsedParticle(0)
	  //particles(new Particle[MAX_PARTICLES]),
	  //particle_positions(new float[3*MAX_PARTICLES])
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
	glClearColor(0.1, 0.1, 0.1, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
//	glGenVertexArrays(1, &m_vao_particle);
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

	{
		glGenFramebuffers(1, &m_framebuffer);
		//glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		CHECK_GL_ERRORS;
	}

	{
		glGenTextures(1, &m_shadowMap);
		//glBindTexture(GL_TEXTURE_2D, m_shadowMap);
		CHECK_GL_ERRORS;
	}

	{
		glGenTextures(1, &m_texture);
		//glBindTexture(GL_TEXTURE_2D, m_texture);
		CHECK_GL_ERRORS;
	}

	findPlayerNode((SceneNode*)&*m_rootNode);

	findPlaneNode((SceneNode*)&*m_rootNode);

	findEnemyNodes((SceneNode*)&*m_rootNode);

	findSpecialObjects((SceneNode*)&*m_rootNode);

	Bounds bounds(m_plane->hitbox->_pos, m_plane->hitbox->_maxXYZ);
	m_collisionTree = new CollisionTreeNode(bounds, 0);
	m_collisionTree->construct((SceneNode*)&*m_rootNode);

	m_start_time = clock();

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

	m_shader_shadow.generateProgramObject();
	m_shader_shadow.attachVertexShader( getAssetFilePath("shadow_VertexShader.vs").c_str() );
	m_shader_shadow.attachFragmentShader( getAssetFilePath("shadow_FragmentShader.fs").c_str() );
	m_shader_shadow.link();

	/*m_particle_shader.generateProgramObject();
	m_particle_shader.attachVertexShader( getAssetFilePath("particle_vs.vs").c_str());
	m_particle_shader.attachFragmentShader( getAssetFilePath("particle_fs.fs").c_str() );
	m_particle_shader.link();*/
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


		m_textureAttrribLocation = m_shader.getAttribLocation("uv");
		glEnableVertexAttribArray(m_textureAttrribLocation);

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

	/*{
		glBindVertexArray(m_vao_particle);

		m_particleAttrribLocation = m_particle_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_particleAttrribLocation);
		//bind stuff later
	}*/

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

	// Generate VBO to store all texture UV data
	{
		glGenBuffers(1, &m_vbo_vertexUV);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexUV);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexUVBytes(),
				meshConsolidator.getVertexUVDataPtr(), GL_STATIC_DRAW);

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

	// Generate VBO to store the particles.
	/*{
		glGenBuffers( 1, &m_vbo_particle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_particle );

		glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES*3*sizeof(float), NULL, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}*/
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

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexUV);
	glVertexAttribPointer(m_textureAttrribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

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

	/*glBindVertexArray(m_vao_particle);
	glBindBuffer(GL_ARRAY_BUFFER, m_vao_particle);
	glVertexAttribPointer(m_particleAttrribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;*/

}

//----------------------------------------------------------------------------------------
void Project::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void Project::initViewMatrix() {
	vec3 lookFrom = glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 );
	m_view = glm::lookAt( 
		lookFrom,
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );/*glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));*/
	//vec3 reflectedLF = glm::reflect(, glm::normalize(vec3(0.0, 1.0, 0.0)));
	//vec3 reflectedLF = glm::vec3( 0.0f, -0.5-float(DIM)*2.0*M_SQRT1_2, 5.0-float(DIM)*2.0*M_SQRT1_2 );
}

//----------------------------------------------------------------------------------------
void Project::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
	m_shadowView = glm::lookAt( 
		m_light.position,
		glm::vec3( 0.0f, -0.5f, -5.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );
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
	if (invincibilityTime > 0){
		invincibilityTime--;
		//cout << "invincibilityTime: " << invincibilityTime << endl;
	}
	m_planeDrawn = false;
	m_current_time = clock() - m_start_time;
	m_current_time_secs = ((float)m_current_time)/CLOCKS_PER_SEC;

	if (lmb_down && m_playerNode != nullptr && lives > 0){
		Shot* shot = new Shot(m_playerNode, shotId);
		//cout << "Added shot " << shotId << endl;
		shotId++;
		if (shotId > 10000){
			shotId = 0;
		}
		m_shots.emplace_back(shot);
		m_playerNode->add_child(shot->_self);
	}
	
	for (auto& enemy: m_enemies){
		moveEnemy(enemy);
	}
	//moveEnemy(m_enemy1);
	m_collisionTree->clear();
	m_collisionTree->construct((SceneNode*)&*m_rootNode, m_current_time_secs);

	for (Shot* shot : m_shots){
		shot->advance();
		//cout << "shot " << shot->_self->m_name << " advancing " << endl;
		checkShotCollisions(shot);
	}

	moveParticles();

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

		if( ImGui::Button( "Reset All" ) ) {
			resetAll();
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	ImGui::Begin("Player", &showDebugWindow, ImVec2(100, 300), opacity,
			windowFlags);

		string livesDisplay = "";
		for (int i = 0; i < lives; i++){
			livesDisplay += "*";
		}

		ImGui::Text("Lives: %s", livesDisplay.c_str());


	ImGui::End();

	ImGui::Begin("Options", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);
			

		if( ImGui::Checkbox( "Z-buffer", &m_zbuffer) ) {
			
		}

		if( ImGui::Checkbox( "Backface Culling", &m_backfaceCulling) ) {
			
		}
		
		if( ImGui::Checkbox( "Draw Reflections" , &m_drawReflection) ) {
			
		}

		if( ImGui::Checkbox( "Draw Shadows", &m_doShadowMapping) ) {
			
		}

		if( ImGui::Checkbox( "Apply Texture", &m_drawTexture) ) {
			
		}

	ImGui::End();

	
	
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix, 
		bool shadow, bool gettingShadows,
		bool inReflectionMode = false
) {

	shader.enable();
	{

		GLuint location;
		//-- Set ModelView matrix:
		location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		location = shader.getUniformLocation("curTime");
		glUniform1f(location, 0);
		CHECK_GL_ERRORS;

		location = shader.getUniformLocation("time0");
		glUniform1f(location, 0);
		CHECK_GL_ERRORS;

		GLuint draw_shadows = shader.getUniformLocation("drawShadows");
		glUniform1f(draw_shadows, shadow && !inReflectionMode);
		CHECK_GL_ERRORS;

		if(!gettingShadows){
			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;


			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			vec3 kd = inReflectionMode ? 0.5*node.material.kd : node.material.kd;
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.ks");
			vec3 ks = inReflectionMode ? 0.5*node.material.ks : node.material.ks;
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, node.material.shininess);
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.alpha");
			glUniform1f(location, node.material.alpha);
		
		}
	}

	shader.disable();

}

//----------------------------------------------------------------------------------------
void Project::getShadowMap(SceneNode* root){

	//glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	CHECK_GL_ERRORS;

	//glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	CHECK_GL_ERRORS;	
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//CHECK_GL_ERRORS;
	CHECK_GL_ERRORS;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	CHECK_GL_ERRORS;
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadowMap, 0);
	CHECK_GL_ERRORS;
	glDrawBuffer(GL_NONE);
	CHECK_GL_ERRORS;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	/*if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;*/

	mat4 ortho_mat = glm::ortho<float>(-12, 12, -20, 20, -25, 25); //glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
	//m_shadowView already set
	m_ortho_shadowView = ortho_mat * m_shadowView;// * mat4(1.0);
	//cout << "generating shadows" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glViewport(0,0,1024,1024);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_vao_meshData);
	getNodeShadows(root);
	
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECK_GL_ERRORS;
	//glViewport(0,0,1024,1024);
	glViewport(0,0, m_windowWidth, m_windowHeight);
}

//----------------------------------------------------------------------------------------
void Project::getNodeShadows(SceneNode* root){
	if (root->m_nodeType == NodeType::GeometryNode){
		GeometryNode * geometryNode = static_cast<GeometryNode *>(root);
		if (geometryNode != m_plane)
		{
		//updateShaderUniforms(m_shader_shadow, *geometryNode, m_ortho_shadowView, true);
		m_shader_shadow.enable();
		GLint location = m_shader_shadow.getUniformLocation("ModelView");
		mat4 modelView = m_ortho_shadowView * geometryNode->trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;
		m_shader_shadow.disable();

		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

			//-- Now render the mesh:
		m_shader_shadow.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

		m_shader_shadow.disable();
		}

	}
	for (SceneNode *child : root->children){
		child->set_transform(root->get_transform() * child->get_transform());
		getNodeShadows(child);
		child->set_transform(glm::inverse(root->get_transform()) * child->get_transform());
	}
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

	//getShadowMap((SceneNode *) &*m_rootNode);

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

	if (m_doShadowMapping){
		getShadowMap((SceneNode *) &*m_rootNode);

		//glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
		//CHECK_GL_ERRORS;
		glm::mat4 biasMatrix(
				0.5, 0.0, 0.0, 0.0, 
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0
		);

		m_depthBias = biasMatrix*m_ortho_shadowView;
	}

	if (m_drawReflection){
		drawReflection((SceneNode *) &*m_rootNode);
	}

	renderSceneGraph(*m_rootNode, false);

	glBindVertexArray(m_vao_meshData);
	renderTransparentObjects((SceneNode *) &*m_rootNode);
	glBindVertexArray(0);
	CHECK_GL_ERRORS;

	drawParticles();

	if (m_zbuffer)
		glDisable( GL_DEPTH_TEST );

	if (m_backfaceCulling || m_frontfaceCulling){
		glDisable(GL_CULL_FACE);
	}
}

//----------------------------------------------------------------------------------------
void Project::renderSceneGraph(const SceneNode & root, bool inReflectionMode) {

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

	if (m_drawReflection && inReflectionMode){
		drawPlane();
	} else if (m_drawTexture) {
		applyTexture(m_plane);
	}

	renderNodes((SceneNode *) &root, inReflectionMode);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//cheating hierarchy since plane doesn't have any parent transformations
//----------------------------------------------------------------------------------------
void Project::drawPlane(){


	/*updateShaderUniforms(m_shader, *m_bg, 
					m_view, m_doShadowMapping, false);

	BatchInfo batchInfo = m_batchInfoMap[m_bg->meshId];

	m_shader.enable();
	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

	m_shader.disable();*/

	//cout << glm::to_string(m_bg->trans) << endl;

	if (m_drawReflection){
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glDepthMask(GL_FALSE);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	updateShaderUniforms(m_shader, *m_plane, 
					m_view, m_doShadowMapping, false);

	applyTexture(m_plane);

	if (m_doShadowMapping){
		m_shader.enable();
		GLuint DepthBiasID = m_shader.getUniformLocation("depthBiasMVP");
		GLuint ShadowMapID = m_shader.getUniformLocation("shadowMap");

				//GLuint TextureID = glGetUniformLocation(programID, "textureSampler");

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_shadowMap);
		glUniform1i(ShadowMapID, 1);
		CHECK_GL_ERRORS;

		mat4 depthBias_times_model = m_depthBias*m_plane->trans;
		glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, value_ptr(depthBias_times_model));
		CHECK_GL_ERRORS;
		m_shader.disable();
	}

	BatchInfo batchInfo = m_batchInfoMap[m_plane->meshId];

	//m_plane->texture.loadFile((char*)filename.c_str());

	//if (m_drawTexture)

	//-- Now render the mesh:
	m_shader.enable();
	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS;
	m_shader.disable();

	m_planeDrawn = true;
}


//----------------------------------------------------------------------------------------
void Project::drawShot(Shot* shot){
	m_shader.enable();

	GLuint location = m_shader.getUniformLocation("ModelView");
	mat4 modelView = m_view * shot->_self->trans;
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
	CHECK_GL_ERRORS;

	location = m_shader.getUniformLocation("curTime");
	glUniform1f(location, 0);
	CHECK_GL_ERRORS;

	location = m_shader.getUniformLocation("time0");
	glUniform1f(location, 0);
	CHECK_GL_ERRORS;

	GLuint draw_shadows = m_shader.getUniformLocation("drawShadows");
	glUniform1f(draw_shadows, false);
	CHECK_GL_ERRORS;

	location = m_shader.getUniformLocation("NormalMatrix");
	mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
	glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
	CHECK_GL_ERRORS;

	location = m_shader.getUniformLocation("material.kd");
	vec3 kd(1.0, 1.0, 0.0); // yellow
	glUniform3fv(location, 1, value_ptr(kd));
	CHECK_GL_ERRORS;
	location = m_shader.getUniformLocation("material.ks");
	vec3 ks(1.0, 1.0, 0.0); // yellow
	glUniform3fv(location, 1, value_ptr(ks));
	CHECK_GL_ERRORS;

	location = m_shader.getUniformLocation("material.shininess");
	glUniform1f(location, 100);
	CHECK_GL_ERRORS;
	location = m_shader.getUniformLocation("material.alpha");
	glUniform1f(location, 1.0);

	BatchInfo batchInfo = m_batchInfoMap[shot->_meshId];

	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

	m_shader.disable();

}

void Project::drawParticles(){
	for (auto& p : particles){

	if (p->_life <= 0) continue; 
	//updateShaderUniforms(m_particle_shader, *node, 
	//				m_view, m_doShadowMapping, false);
	glBindVertexArray(m_vao_meshData);

	m_shader.enable();

	GLuint location;
		//-- Set ModelView matrix:
	//cout << "trans: " << glm::to_string(p->_trans) << endl;
	location = m_shader.getUniformLocation("ModelView");
	mat4 modelView = m_view  * p->_trans;
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
	CHECK_GL_ERRORS;

	location = m_shader.getUniformLocation("curTime");
		glUniform1f(location, 0);
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("time0");
		glUniform1f(location, 0);
		CHECK_GL_ERRORS;

			//-- Set NormMatrix:
	location = m_shader.getUniformLocation("NormalMatrix");
	mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
	glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
	CHECK_GL_ERRORS;


		//-- Set Material values:
	location = m_shader.getUniformLocation("material.kd");
	vec3 kd(1.0, 1.0, 0.0);
	glUniform3fv(location, 1, value_ptr(kd));
	CHECK_GL_ERRORS;
	location = m_shader.getUniformLocation("material.ks");
	vec3 ks(1.0, 1.0, 0.0);
	glUniform3fv(location, 1, value_ptr(ks));
	CHECK_GL_ERRORS;
	location = m_shader.getUniformLocation("material.shininess");
	glUniform1f(location, 100);
	CHECK_GL_ERRORS;
	location = m_shader.getUniformLocation("material.alpha");
	glUniform1f(location, 1.0f);
	CHECK_GL_ERRORS;
		

	//glBindBuffer( GL_ARRAY_BUFFER, m_vbo_particle );

	//glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES*3*sizeof(float), NULL, GL_STREAM_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, particleCount*3*sizeof(float), particle_positions);
	CHECK_GL_ERRORS;


	BatchInfo batchInfo = m_batchInfoMap["cube"];

	//m_particle_shader.enable();
	/*for (int i = 0; i < particleCount; i++){
		glDrawArrays(GL_TRIANGLES, 0, 6)
	}*/
	//cout << "particle count: "<< particleCount << endl;
	//glDrawArraysInstanced(GL_TRIANGLES, 0, 3, particleCount);
	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

	//CHECK_GL_ERRORS;

	m_shader.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
	}
}

void Project::moveParticles(){
	for (auto& p : particles){
		//Particle& p = particles[i];
		if (p->_life > 0){
			p->_life--;
			if (p->_life > 0){
				p->_speed += glm::vec3(0.0, -9.81, 0.0) * 0.1;
				//p._pos += p._speed;
				p->move();
			}
		} else {
			auto it = std::find(particles.begin(), particles.end(), p);
			if (it != particles.end()){
				particles.erase(it);
			}
		}
	}

	
}

void Project::generateParticles(GeometryNode* node){
	//cout << "generateParticles at " << glm::to_string(node->trans) << endl;
	for (int i = 0; i < 50; i++){
		float x = dis(e) - 1.0;
		float y = dis(e);
		float z = dis(e) - 1.0;

		float modifier = 1.5;

		vec3 pos(x*modifier, y*modifier, z*modifier);

		Particle* p = new Particle(node->trans,pos);

		particles.emplace_back(p);
	}
}

//----------------------------------------------------------------------------------------
void Project::applyTexture(GeometryNode* node){

	//return;
	m_shader.enable();
	//glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	CHECK_GL_ERRORS;
	
	//cout << "width: " << node->texture._w << ", height: " << node->texture._h << endl;

	/*if(m_doShadowMapping){
		location = m_shader.getUniformLocation("shadowMap");
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_shadowMap);
		glUniform1i(location, 1);			
	}*/

	if (node->texture._data != nullptr && m_drawTexture){
		//cout << "applying textures" << endl;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, node->texture._w, node->texture._h, 0, GL_RGBA, GL_UNSIGNED_BYTE, node->texture._data);
		CHECK_GL_ERRORS;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		CHECK_GL_ERRORS;

	}

	GLuint location = m_shader.getUniformLocation("drawTexture");
	glUniform1f(location, (node->texture._data != nullptr && m_drawTexture));
	CHECK_GL_ERRORS;

	m_shader.disable();

}


//----------------------------------------------------------------------------------------
void Project::renderNodes(SceneNode *root, bool inReflectionMode){

	//cout << "inReflectionMode: " << inReflectionMode << endl;

	if (root->m_nodeType == NodeType::GeometryNode){
		GeometryNode * geometryNode = static_cast<GeometryNode *>(root);

		//geometryNode->updateHitbox(m_current_time_secs);
		//if (!(m_planeDrawn && (geometryNode == m_plane || geometryNode == m_bg)))
		if (!(geometryNode->hasAnimation()))
		{

			if(inReflectionMode && m_drawReflection && geometryNode != m_plane){
				glStencilFunc(GL_EQUAL, 1, 0xFF); //pass if stencil value 1
				glStencilMask(0x00); //don't write to stencil buffer
				glDepthMask(GL_TRUE);
			}

			updateShaderUniforms(m_shader, *geometryNode, 
					m_view, m_doShadowMapping, false, inReflectionMode);

			m_shader.enable();

			GLuint location = m_shader.getUniformLocation("drawTexture");
			glUniform1f(location, (geometryNode->texture._data != nullptr && m_drawTexture));
			CHECK_GL_ERRORS;

			if (m_drawTexture && geometryNode->texture._data != nullptr){
				GLuint location = m_shader.getUniformLocation("textureSampler");
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_texture);
				glUniform1i(location, 0);
				CHECK_GL_ERRORS;
			}

			if (m_doShadowMapping){
				GLuint DepthBiasID = m_shader.getUniformLocation("depthBiasMVP");
				GLuint ShadowMapID = m_shader.getUniformLocation("shadowMap");

				//GLuint TextureID = glGetUniformLocation(programID, "textureSampler");

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_shadowMap);
				glUniform1i(ShadowMapID, 1);
				CHECK_GL_ERRORS;

				mat4 depthBias_times_model = m_depthBias*geometryNode->trans;
				glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, value_ptr(depthBias_times_model));
				CHECK_GL_ERRORS;
			}

			m_shader.disable();


			if (!(!inReflectionMode && geometryNode->isTransparent()) && !(m_drawReflection && geometryNode == m_plane)){
				//if (m_drawTexture)
				//if (geometryNode == m_plane)
				//	applyTexture(geometryNode);

				BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

				//-- Now render the mesh:
				m_shader.enable();
				glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

				m_shader.disable();
			}

			if (RENDER_HITBOX && !inReflectionMode) renderHitbox(geometryNode);
		} else {
			renderAnimatedObject(geometryNode, inReflectionMode);
		}

		if (geometryNode == m_playerNode){
			for (Shot* shot : m_shots){
				drawShot(shot);
			}
			return; //don't draw shots twice
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
				CHECK_GL_ERRORS;

	for (SceneNode *child : root->children){

		if (inReflectionMode && m_drawReflection){
			child->translate(vec3(0, 1, 0));
			child->scale(vec3(1, -1, 1));
		}
		child->set_keyframe_parent_transform(root->get_transform());
		child->set_transform(root->get_transform() * child->get_transform());
		renderNodes(child, inReflectionMode);
		if (inReflectionMode && m_drawReflection){
			child->scale(vec3(1, -1, 1));
			child->translate(vec3(0, -1, 0));
		}
		child->set_transform(glm::inverse(root->get_transform()) * child->get_transform());

	}
}

//----------------------------------------------------------------------------------------
void Project::renderAnimatedObject(GeometryNode *node, bool inReflectionMode){
	if(inReflectionMode && m_drawReflection && node != m_plane){
		glStencilFunc(GL_EQUAL, 1, 0xFF); //pass if stencil value 1
		glStencilMask(0x00); //don't write to stencil buffer
		glDepthMask(GL_TRUE);
	}

	updateShaderUniforms(m_shader, *node, 
		m_view, m_doShadowMapping, false, inReflectionMode);

		m_shader.enable();

		int seconds = (int) m_current_time_secs;

		Keyframe* cur = node->getKeyframeAt(seconds);
		Keyframe* next = node->getNextKeyframe(seconds);

		mat4 modelView0 = m_view * cur->get_total_transform();
		mat4 modelView1 = m_view * next->get_total_transform();
		float time0 = (float)(cur->time);
		float time1 = (float)(next->time);

		if (inReflectionMode){
			modelView0 = m_view * cur->parentTrans * glm::scale(vec3(1, -1, 1)) * translate(vec3(0, 1, 0)) * cur->trans;
			modelView1 = m_view * next->parentTrans * glm::scale(vec3(1, -1, 1)) * translate(vec3(0, 1, 0)) * next->trans;
		}

		GLuint location = m_shader.getUniformLocation("ModelView");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView0));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("nextModelView");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView1));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("time0");
		glUniform1f(location, time0);
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("time1");
		glUniform1f(location, time1);
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("curTime");
		float dec = (float)(m_current_time_secs - seconds);
		glUniform1f(location, (float)(seconds%node->m_animationEnd) + dec);
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("drawTexture");
		glUniform1f(location, (node->texture._data != nullptr && m_drawTexture));
		CHECK_GL_ERRORS;

		if (m_drawTexture && node->texture._data != nullptr){
			location = m_shader.getUniformLocation("textureSampler");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_texture);
			glUniform1i(location, 0);
			CHECK_GL_ERRORS;
		}

		if (m_doShadowMapping){
			GLuint DepthBiasID = m_shader.getUniformLocation("depthBiasMVP");
			GLuint ShadowMapID = m_shader.getUniformLocation("shadowMap");

				//GLuint TextureID = glGetUniformLocation(programID, "textureSampler");

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_shadowMap);
			glUniform1i(ShadowMapID, 1);
			CHECK_GL_ERRORS;

			mat4 depthBias_times_model = m_depthBias*node->trans;
			glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, value_ptr(depthBias_times_model));
			CHECK_GL_ERRORS;
		}

	m_shader.disable();


	if (!(!inReflectionMode && node->isTransparent()) && !(m_drawReflection && node == m_plane)){
				//if (m_drawTexture)
				//if (geometryNode == m_plane)
				//	applyTexture(geometryNode);

		BatchInfo batchInfo = m_batchInfoMap[node->meshId];

				//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

		m_shader.disable();
	}

	if (RENDER_HITBOX && !inReflectionMode) renderHitbox(node);
}

void Project::renderTransparentObjects(SceneNode *root){

	if (root->m_nodeType == NodeType::GeometryNode){
		GeometryNode * geometryNode = static_cast<GeometryNode *>(root);

		if (geometryNode->isTransparent()){
			glEnable(GL_BLEND);

			glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
			glBlendEquation(GL_FUNC_ADD);

			m_shader.enable();
			GLuint location = m_shader.getUniformLocation("drawTexture");
			glUniform1f(location, (geometryNode->texture._data != nullptr && m_drawTexture));
			CHECK_GL_ERRORS;

			if (m_doShadowMapping){
				
				GLuint DepthBiasID = m_shader.getUniformLocation("depthBiasMVP");
				GLuint ShadowMapID = m_shader.getUniformLocation("shadowMap");

				//GLuint TextureID = glGetUniformLocation(programID, "textureSampler");

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_shadowMap);
				glUniform1i(ShadowMapID, 1);
				CHECK_GL_ERRORS;

				mat4 depthBias_times_model = m_depthBias*geometryNode->trans;
				glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, value_ptr(depthBias_times_model));
				CHECK_GL_ERRORS;
			}

			m_shader.disable();

			updateShaderUniforms(m_shader, *geometryNode, m_view, m_doShadowMapping, false);

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
void Project::drawReflection(SceneNode* root){
	if (!m_drawReflection) return;

	glEnable(GL_STENCIL_TEST);

	renderSceneGraph(*root, true);

	glDisable(GL_STENCIL_TEST);
	
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
void Project::findBgNode(SceneNode *root){
	if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "bg"){
		m_bg = static_cast<GeometryNode *>(root);
		return;
	}
	for (SceneNode *child : root->children){
		findBgNode(child);
	}
}


//----------------------------------------------------------------------------------------
void Project::findEnemyNodes(SceneNode *root){
	if (root->m_nodeType == NodeType::GeometryNode){
		GeometryNode* enemy1 = static_cast<GeometryNode *>(root);
		if (enemy1->isEnemy())
			m_enemies.emplace_back(enemy1);
	} /*else if (root->m_nodeType == NodeType::GeometryNode && root->m_name == "e2"){
		m_enemy2 = static_cast<GeometryNode *>(root);
	}*/

	//if (m_enemy1 != nullptr && m_enemy2 != nullptr) return;
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


void Project::resetAll(){
	for (auto child : m_rootNode->children){
		m_rootNode->remove_child(child);
	}

	m_rootNode = nullptr;

	processLuaSceneFile(m_luaSceneFile);

	findPlayerNode((SceneNode*)&*m_rootNode);

	findPlaneNode((SceneNode*)&*m_rootNode);

	findEnemyNodes((SceneNode*)&*m_rootNode);

	findSpecialObjects((SceneNode*)&*m_rootNode);

	Bounds bounds(m_plane->hitbox->_pos, m_plane->hitbox->_maxXYZ);
	//m_collisionTree = new CollisionTreeNode(bounds, 0);
	m_collisionTree->clear();
	m_collisionTree->construct((SceneNode*)&*m_rootNode);

	m_start_time = clock();

	lives = 3;
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


void Project::moveEnemy(GeometryNode* enemy){
	//if (m_current_time%1000 != 0) return;
	//std::uniform_real_distribution<> dis(-1, 1);
	//srand(m_current_time);
	double x =  dis(e) - 1.0;
	double y = 0;//rand() % 2 - 1;
	double z = dis(e) - 1.0;//rand() % 2 - 1;

	double modifier = 0.5;

	enemy->translate(vec3(modifier*x, modifier*y, modifier*z));

	std::vector<GeometryNode*> collisions;
	std::vector<vec3> axis;
	m_collisionTree->collideGeometry(enemy, collisions, axis, false);
	bool adjust(false);

	for (int i = 0; i < collisions.size(); i++){
		GeometryNode* collision = collisions[i];
		//cout << collision->m_name << endl;
		if (collision == m_plane || collision == enemy) {
			continue;
		} else if (collision == m_playerNode){
			if (invincibilityTime <= 0){
				lives--;
				generateParticles(m_playerNode);
				invincibilityTime = 5;
				if (lives <=0){
					removeNode((SceneNode*)&*m_rootNode, m_playerNode);
				}
			}
			//continue;
		}
		adjust = true;
	}

	if (adjust){
		enemy->translate(vec3(-modifier*x, -modifier*y, -modifier*z));
	}
}

void Project::movePlayer(double x, double z, bool adjusting){
	if (m_playerNode == nullptr) return;
	dvec3 transl(x, 0.0, z);
	m_playerNode->translate(transl);
	std::vector<GeometryNode*> collisions;
	std::vector<vec3> axis;
	m_collisionTree->collideGeometry(m_playerNode, collisions, axis, false);
	bool adjust(false);
	//cout << "collisions: " << collisions.size() << endl;
	for (int i = 0; i < collisions.size(); i++){
		GeometryNode* collision = collisions[i];
		//cout << collision->m_name << endl;
		if (collision == m_plane || collision == m_playerNode) continue;
		if (invincibilityTime <= 0 && collision->isEnemy()){//(collision == m_enemy1 || collision == m_enemy2)){
			cout << "collided into " << collision->m_name << endl;
			lives--;
			generateParticles(m_playerNode);
			invincibilityTime = 5;
			if (lives <= 0){
				removeNode((SceneNode*)&*m_rootNode, m_playerNode);
			}
		}
		adjust = true;
	}

	if (adjust){
		//cout << glm::to_string(transl_back) << endl;
		double backx = 0;
		double backz = 0;
		if (abs(x) > std::numeric_limits<double>::epsilon()){
			backx = std::copysign(0.1, adjusting ? x : -x);
		}
		if (abs(z) > std::numeric_limits<double>::epsilon()){
			backz = std::copysign(0.1, adjusting ? z : -z);
		}

		movePlayer(backx, backz, true);
	}
}

void Project::checkShotCollisions(Shot* shot){
	if (lives <= 0) return;
	GeometryNode* node = shot->_self;

	std::vector<GeometryNode*> collisions;
	std::vector<vec3> axis;
	m_collisionTree->collideGeometry(node, collisions, axis, false);
	bool removeSelf = false;

	//cout << "shot collided with " << collisions.size() << "objects" <<endl;

	for (int i = 0; i < collisions.size(); i++){
		GeometryNode* collision = collisions[i];
		//cout << shot->_self->m_name << " collided with " << collision->m_name << endl;
		if (collision == m_plane || collision == m_playerNode){
			continue;
		} else if (collision->m_name.find("shot") != std::string::npos) {
			continue;
		} else if (collision->isEnemy()){// == m_enemy1 || collision == m_enemy2){
			generateParticles(collision);
			removeNode((SceneNode*)&*m_rootNode, collision);
			removeSelf = true;
		} else {
			//generateParticles(collision);
			removeSelf = true;
		}
	}

	if (removeSelf){
		m_playerNode->remove_child(shot->_self);
		auto it = std::find(m_shots.begin(), m_shots.end(), shot);
		if (it != m_shots.end()){
			m_shots.erase(it);
		}
	}
}

void Project::removeNode(SceneNode* root, GeometryNode* target){
	if (root == nullptr) return;
	for (SceneNode* child : root->children){
		if (child->m_nodeType == NodeType::GeometryNode){
			GeometryNode * geometryNode = static_cast<GeometryNode *>(child);
			if (geometryNode == target){
				//cout << "removing "<< target->m_name << endl; 
				root->remove_child(child);
				return;
			}
		}
		removeNode(child, target);
	}
}

void Project::rotateShot(double x){
	if (m_playerNode == nullptr) return;
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

		if (button == GLFW_MOUSE_BUTTON_LEFT){
			lmb_down = true;
			eventHandled = true;
		}
	}

	if (actions == GLFW_RELEASE){
		if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			mmb_down = false;
			eventHandled = true;
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT){
			lmb_down = false;
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
