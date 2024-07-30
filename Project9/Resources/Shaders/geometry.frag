#version 440
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D diffuse;
uniform sampler2D specular;


// uniforms
uniform vec3 albedo;

// uniform float metallic;
// uniform float roughness;
// uniform float ao;

// uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;

// raytrace a blade of grass:
// solve the equasion for the ray hitting the blade...
// this is the same as solving for the sphere but with a different equasion!

// todo: solve this for each ray (this is my grass!)
// z = (1 - 500(y^2 + 0.5x^2))

// 
void main()
{    
    // test
   // gPosition = vec4(1);
   // gPosition =  vec4(normalize(Normal), 1);
   gPosition = FragPos;

    
   // gNormal = vec4(1);
   // gNormal = vec4(1,0,0,1);
   gNormal = normalize(Normal);

    // test
   gAlbedoSpec = texture(diffuse, TexCoords).rgb;
   if (gAlbedoSpec == vec3(0)) gAlbedoSpec = albedo;


    // and the diffuse per-fragment color
    // gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    // gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}  