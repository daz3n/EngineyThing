#version 440
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec2 aTexCoords;

uniform mat4 model;

// out
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

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

    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * aNormal;

    gl_Position = _projection * _view * worldPos;

}