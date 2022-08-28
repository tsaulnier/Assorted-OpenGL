#version 400
uniform float currentAngle;
uniform mat4 project;
uniform mat4 view;
uniform mat4 model;
uniform float windDir;

layout(location=0) in vec3 vertex;
layout(location=1) in vec3 vColor;
layout(location=2) in vec2 vTexCoord;
out vec3 color;
out vec2 texCoord;
void main() {
    color = vColor;
    texCoord = vTexCoord;

    // Adjust the position of the current vertex
    vec4 v = vec4(vertex,1.);
    float zdisp = sin(v.z+currentAngle)*0.05*v.x;
    v.z += zdisp;
    v.y += sin(v.x*11.3+v.z*sin(currentAngle)+currentAngle)*0.11;
   gl_Position = project*view*model*v;
}
