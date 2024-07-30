#version 440
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


uniform mat4 model;

layout (std140, binding = 1) uniform Camera
{
	vec4 _target;
	vec4 _position;
	mat4 _view;
	mat4 _projection;
	float _fov;
};
void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);

    gl_Position = _projection * _view * worldPos;
}