#version 330 core

//code from http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/

// Ouput data
out float fragmentdepth;

void main(){
    // Not really needed, OpenGL does it anyway
    fragmentdepth = gl_FragCoord.z;
}