#version 330

// Model-Space coordinates
in vec3 position;
in vec3 normal;
in vec2 uv;

struct LightSource {
    vec3 position;
    vec3 rgbIntensity;
};
uniform LightSource light;

uniform mat4 ModelView;
uniform mat4 nextModelView;
uniform float time0;
uniform float time1;
uniform float curTime;
uniform mat4 Perspective;

// Remember, this is transpose(inverse(ModelView)).  Normals should be
// transformed using this matrix instead of the ModelView matrix.
uniform mat3 NormalMatrix;

out VsOutFsIn {
	vec3 position_ES; // Eye-space position
	vec3 normal_ES;   // Eye-space normal
	LightSource light;
} vs_out;

out vec4 ShadowCoord;
out vec2 UV;

uniform mat4 depthBiasMVP;

vec4 lerp(float curTime, float time0, vec4 p0, float time1, vec4 p1){
	float epsilon = 0.001 f;

	if (abs(curTime - time0) < epsilon){
		return p0;
	} else if (abs(time1 - curTime) < epsilon){
		return p1;
	} else {
		return p0 + (p1 - p0)*(curTime - time0);
	}
}


void main() {
	vec4 pos4 = vec4(position, 1.0);

	//-- Convert position and normal to Eye-Space:
	vs_out.position_ES = (ModelView * pos4).xyz;
	vs_out.normal_ES = normalize(NormalMatrix * normal);

	vs_out.light = light;

	ShadowCoord =  (depthBiasMVP * pos4);//* vec4(position,1);

	vec4 p0 = ModelView * pos4;
	vec4 p1 = nextModelView * pos4;

	gl_Position = Perspective * ModelView * lerp(curTime, time0, p0, time1, p1);//vec4(position, 1.0);

	UV = uv;
}
