#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <cmath>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

//since all you do to joints is rotate, extremely lazy implementation
class Command {
public:
	std::vector<SceneNode*> _nodes;
	std::vector<glm::mat4> _mats;
	float _rotateX;
	float _rotateY;
	float _neckY;

	Command() : _rotateX(0.0), _rotateY(0.0), _neckY(0.0) { }

	void execute(int direction){
		direction = std::copysign(1, direction);
		//idk how to undo rotate on 2 DOF on the head so have to work with entire matrix
		for (int i = 0; i < _nodes.size(); i++){
			if (direction < 0){
				_nodes[i]->set_transform( glm::inverse(_mats[i]) * _nodes[i]->get_transform());
			} else {
				_nodes[i]->set_transform( _mats[i] * _nodes[i]->get_transform());
			}
		}
		
	}
};

class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();
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

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderNodes(SceneNode *root, bool picking = false);
	void renderArcCircle();

	void jointPickerGui(SceneNode *node);

	void resetOrientation();
	void resetPosition();
	void resetJoints(SceneNode *root);
	void resetAll();

	void moveJoints(SceneNode *root, float x, float y);
	void pick(SceneNode *node, unsigned int id);
	void select(SceneNode *node);

	void undo();
	void redo();


	glm::mat4 m_perpsective;
	glm::mat4 m_view;
	glm::mat4 m_translation;
	glm::mat4 m_rotation;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

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

	enum Mode {
		POSITION,
		JOINT
	};

	int tempMode;
	Mode m_mode;
	bool m_drawCircle;
	bool m_zbuffer;
	bool m_backfaceCulling;
	bool m_frontfaceCulling;

	float m_mouseX;
	float m_mouseY;
	float m_rotateX;
	float m_rotateY;

	float m_jointRotateX;
	float m_jointRotateY;

	Command* m_curCmd;
	Command* m_neckCmd;
	bool cmd_started;

	bool lmb_down;
	bool mmb_down;
	bool rmb_down;

	std::vector<SceneNode*> m_selectedJoints;
	std::vector<Command*> m_undoStack;
	std::vector<Command*> m_redoStack;
};
