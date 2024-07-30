#version 440

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedo;


layout(location = 0) out vec3 FragColor;
layout(location = 1) out vec3 BloomColor;

layout(location = 1) in vec2 uv;


struct Light
{
	vec4 position_radius;
	vec4 color_intensity;
};


layout (std140, binding = 0) uniform Window
{
	vec2 _size;
	vec2 _resolution;
	
	float _runtime;
	float _playtime;
	
	int _random;
}; 
layout (std140, binding = 1) uniform Camera
{
	vec4 _target;
	vec4 _position;
	mat4 _view;
	mat4 _projection;
	float _fov;
};


layout (std140, binding = 2) buffer Lights
{
	vec4 directional_light_position;
	vec4 directional_light_color_intensity;

	Light point_lights[];
};


float calculate_light_falloff(float _intensity, float _dist)
{
	// return _intensity / (0.0001 + (_dist * _dist));
	return _intensity / (0.0001 + (_dist));
}
float calculate_light_reflection(vec3 _light_pos,vec3 _frag_pos, vec3 _frag_normal)
{
	vec3 _light_dir = normalize(_light_pos - _frag_pos);
	
	return max(0, dot(_frag_normal, _light_dir));
}


// this still needs to be multiplied by the object color 
// but all lighting equasions are complete.
vec3 lighting_at(vec3 light_pos, vec4 light_color,vec3 frag_pos, vec3 frag_normal)
{
	float _dist = distance(light_pos, frag_pos);
	float _falloff = calculate_light_falloff(light_color.a, _dist);
	float _reflect = calculate_light_reflection(light_pos, frag_pos, frag_normal);


	return light_color.rgb * _falloff * _reflect;
}



void main()
{
   // TEST ///////////////////////////////////////////////////////////////////////////////
   // vec4 col = vec4(0);
   
   // TEST ///////////////////////////////////////////////////////////////////////////////
   // vec4 col = vec4(uv.x,uv.y,0,1);

   // TEST ///////////////////////////////////////////////////////////////////////////////
	// float A = _runtime / 10000;
   // FragColor = vec4(A, A, A, 1.0f);
   // FragColor = vec4(1.0f);

   // TEST ///////////////////////////////////////////////////////////////////////////////
   // vec4 col = texture2D(gPosition, uv);
   // vec4 col = texture2D(gNormal, uv);
   // vec4 col = texture2D(gAlbedo, uv);

   // SHADER ///////////////////////////////////////////////////////////////////////////////
   
   
   vec3 col1 = texture(gPosition, uv).xyz;
   vec3 col2 = texture(gNormal, uv).xyz;
   vec3 col3 = texture(gAlbedo, uv).xyz;
   vec3 col4 = vec3(0); // lit 
   vec3 col5 = vec3(0); // bloom 

   vec3 out1 = vec3(0); // lit 
   vec3 out2 = vec3(0); // bloom 

   ///////////////////////////////////////////////////////////////////////////////
   // calculate lighting 
	vec3 lighting = vec3(0);
	for (int i = 0; i < point_lights.length(); ++i)
	{
		lighting = lighting + lighting_at(point_lights[i].position_radius.xyz, point_lights[i].color_intensity, col1, col2);
	}
	col4 = col3 * lighting;


	///////////////////////////////////////////////////////////////////////////////
	// calculate bloom 
	float brightness = dot(col4, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
	{
		col5 = col4;
	}
    else
	{
		col5 = vec3(0.0, 0.0, 0.0);
	}


	///////////////////////////////////////////////////////////////////////////////
	// output results 
	out1 = col4;
	out2 = col5;

	FragColor = out1;
	BloomColor = out2;
}
