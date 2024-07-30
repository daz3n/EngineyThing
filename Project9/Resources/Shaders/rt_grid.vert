#version 440

layout(location = 1) out vec2 uv;

void main()
{
	vec3 vertices[4] = vec3[4](
		vec3( 1.0f,  1.0f, 0.0f),  // top right
		vec3( 1.0f, -1.0f, 0.0f),  // bottom right
		vec3(-1.0f, -1.0f, 0.0f),  // bottom left
		vec3(-1.0f,  1.0f, 0.0f)   // top left 
	);
	int indices[6] = int[6](  // note that we start from 0!
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	);

	vec2 uv_coords[4] = vec2[4](
		vec2(1,1),
		vec2(1,0),
		vec2(0,0),
		vec2(0,1)
	);

	int INDEX = indices[gl_VertexID];

   gl_Position = vec4(vertices[INDEX], 1.0);
   uv = uv_coords[INDEX];

}
