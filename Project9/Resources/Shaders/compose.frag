#version 440


layout(binding = 0) uniform sampler2D image_0; // g buffer 1
layout(binding = 1) uniform sampler2D image_1; // g buffer 2
layout(binding = 2) uniform sampler2D image_2; // g buffer 3
layout(binding = 3) uniform sampler2D image_3; // lighting 1
layout(binding = 4) uniform sampler2D image_4; // sky
layout(binding = 5) uniform sampler2D image_5; // bloom


layout(location = 0) out vec3 FragColor;
layout(location = 1) in vec2 uv;


layout(location = 1) uniform int TextureChoice;



void main()
{    
    vec3 col0 = texture(image_0, uv).rgb;
	vec3 col1 = texture(image_1, uv).rgb;
	vec3 col2 = texture(image_2, uv).rgb;
	vec3 col3 = texture(image_3, uv).rgb;
	vec3 col4 = texture(image_4, uv).rgb;
	vec3 col5 = texture(image_5, uv).rgb;
	
	vec3 out1 = vec3(0);


	// lit + bloom 
	out1 = col3 + col5;
	out1 = mix(col3,col5, 0.14);


	if (col1 == vec3(0))
	{
		// no geometry!
		out1 = col4;
	}	

	// choose output - normally this will always be lighting and bloom. however for debug, i can change it
	if (TextureChoice == 0) out1 = col0;
	if (TextureChoice == 1) out1 = col1;
	if (TextureChoice == 2) out1 = col2;
    if (TextureChoice == 3) out1 = col3;
    if (TextureChoice == 4) out1 = col4;
    if (TextureChoice == 5) out1 = col5;
   
   

    // tone mapping
	float exposure = 1.0;
    out1 = vec3(1.0) - exp(-out1 * exposure);
    // also gamma correct while we're at it
    const float gamma = 2.2;
    out1 = pow(out1, vec3(1.0 / gamma));

   
   FragColor = col2;
}