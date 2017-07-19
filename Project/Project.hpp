#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "CollisionTree.hpp"
#include "Texture.hpp"
#include "Shot.hpp"
#include "Particle.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <cmath>
#include <ctime>
#include <random>

#define MAX_PARTICLES 500

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};


class Project : public CS488Window {
public:
	Project(const std::string & luaSceneFile);
	virtual ~Project();
	bool m_picking;

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();
	void findPlayerNode(SceneNode *root);
	void findPlaneNode(SceneNode *root);
	void findBgNode(SceneNode *root);
	void findEnemyNodes(SceneNode *root);
	void findSpecialObjects(SceneNode *root);

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node, bool inReflectionMode = false);
	void renderNodes(SceneNode *root, bool inReflectionMode = false);
	void renderHitbox(GeometryNode *node);
	void renderTransparentObjects(SceneNode *root);
	void getShadowMap(SceneNode* root);
	void getNodeShadows(SceneNode* root);
	void drawReflection(SceneNode* root);
	void drawPlane();
	void applyTexture(GeometryNode* node);
	void renderAnimatedObject(GeometryNode* node, bool inReflectionMode = false);
	void drawShot(Shot* shot);

	void drawParticles();
	void generateParticles(GeometryNode* node);
	void moveParticles();

	void jointPickerGui(SceneNode *node);

	void resetOrientation();
	void resetPosition();
	void resetAll();

	void movePlayer(double x, double z, bool adjusting = false);
	void moveEnemy(GeometryNode* enemy);
	void checkShotCollisions(Shot* shot, bool enemy = false);
	void rotateShot(double x);
	void removeNode(SceneNode* root, GeometryNode* target);

	clock_t m_start_time;
	clock_t m_current_time;

	float m_current_time_secs;

	glm::mat4 m_perpsective;
	glm::mat4 m_view;
	glm::mat4 m_shadowView;
	glm::mat4 m_reflectedView;
	glm::mat4 m_ortho_shadowView;
	glm::mat4 m_translation;
	glm::mat4 m_rotation;
	glm::mat4 m_depthBias;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLuint m_vbo_vertexUV;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	GLint m_textureAttrribLocation;
	GLint m_shadow_positionAttribLocation;
	ShaderProgram m_shader;
	GLuint m_framebuffer;
	GLuint m_texture;
	GLuint m_shadowMap;
	ShaderProgram m_shader_shadow;

	std::vector<Particle*> particles;

	bool m_doShadowMapping;
	bool m_drawReflection;
	bool m_drawTexture;
	bool m_planeDrawn;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;

	int lives;
	int invincibilityTime; 
	GeometryNode* m_playerNode;
	GeometryNode* m_plane;
	GeometryNode* m_bg;
	GeometryNode* m_enemy1;
	GeometryNode* m_enemy2;
	GeometryNode* m_transparentNode;
	GeometryNode* m_reflectNode;
	CollisionTreeNode* m_collisionTree;
	std::vector<Shot*> m_shots;
	std::vector<Shot*> m_enemyShots;
	int shotId;
	std::vector<GeometryNode*> m_enemies;

	enum Mode {
		POSITION,
		JOINT
	};

	bool m_zbuffer;
	bool m_backfaceCulling;
	bool m_frontfaceCulling;
	bool m_useAlpha;

	bool m_particles_on_all_collisions;
	bool danmaku;
	bool moving_enemies;

	float m_mouseX;
	float m_mouseY;
	float m_playerX;
	float m_playerY;
	float m_shotX;
	float m_shotY;

	float m_jointRotateX;
	float m_jointRotateY;

	bool lmb_down;
	bool mmb_down;
	bool rmb_down;
	bool up_key;
	bool down_key;
	bool left_key;
	bool right_key;

	std::random_device rd;
	std::mt19937 e;
	std::uniform_real_distribution<> dis;
};
