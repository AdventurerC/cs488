#version 330 core

//code from http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/

// Input vertex data, different for all executions of this shader.
//layout(location = 0) in vec3 vertexPosition_modelspace;

in vec3 position;

// Values that stay constant for the whole mesh.
uniform mat4 ModelView;

void main(){
 gl_Position =  ModelView * vec4(position,1.0);
}