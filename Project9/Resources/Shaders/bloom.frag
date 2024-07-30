#version 440

#if 0
// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
layout(binding = 0) uniform sampler2D srcTexture;

layout(location = 1) in vec2 uv;
layout (location = 0) out vec3 downsample;

uniform vec2 srcResolution;
uniform int mipLevel = 1;



vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v)   { return PowVec3(v, invGamma); }

float sRGBToLuma(vec3 col)
{
    //return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
	return dot(col, vec3(0.299f, 0.587f, 0.114f));
}

float KarisAverage(vec3 col)
{
	// Formula is 1 / (1 + luma)
	float luma = sRGBToLuma(ToSRGB(col)) * 0.25f;
	return 1.0f / (1.0f + luma);
}

// NOTE: This is the readable version of this shader. It will be optimized!
void main()
{
	vec2 srcTexelSize = 1.0 / srcResolution;
	float x = srcTexelSize.x;
	float y = srcTexelSize.y;
	vec2 texCoord = uv;

	// Take 13 samples around current texel:
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = texture(srcTexture, vec2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
	vec3 b = texture(srcTexture, vec2(texCoord.x,       texCoord.y + 2*y)).rgb;
	vec3 c = texture(srcTexture, vec2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;

	vec3 d = texture(srcTexture, vec2(texCoord.x - 2*x, texCoord.y)).rgb;
	vec3 e = texture(srcTexture, vec2(texCoord.x,       texCoord.y)).rgb;
	vec3 f = texture(srcTexture, vec2(texCoord.x + 2*x, texCoord.y)).rgb;

	vec3 g = texture(srcTexture, vec2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
	vec3 h = texture(srcTexture, vec2(texCoord.x,       texCoord.y - 2*y)).rgb;
	vec3 i = texture(srcTexture, vec2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;

	vec3 j = texture(srcTexture, vec2(texCoord.x - x, texCoord.y + y)).rgb;
	vec3 k = texture(srcTexture, vec2(texCoord.x + x, texCoord.y + y)).rgb;
	vec3 l = texture(srcTexture, vec2(texCoord.x - x, texCoord.y - y)).rgb;
	vec3 m = texture(srcTexture, vec2(texCoord.x + x, texCoord.y - y)).rgb;

	// Apply weighted distribution:
	// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
	// a,b,d,e * 0.125
	// b,c,e,f * 0.125
	// d,e,g,h * 0.125
	// e,f,h,i * 0.125
	// j,k,l,m * 0.5
	// This shows 5 square areas that are being sampled. But some of them overlap,
	// so to have an energy preserving downsample we need to make some adjustments.
	// The weights are the distributed, so that the sum of j,k,l,m (e.g.)
	// contribute 0.5 to the final color output. The code below is written
	// to effectively yield this sum. We get:
	// 0.125*5 + 0.03125*4 + 0.0625*4 = 1

	// Check if we need to perform Karis average on each block of 4 samples
	vec3 groups[5];


	switch (mipLevel)
	{
	case 0:
	  // We are writing to mip 0, so we need to apply Karis average to each block
	  // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
	  // artifacts).
	  groups[0] = (a+b+d+e) * (0.125f/4.0f);
	  groups[1] = (b+c+e+f) * (0.125f/4.0f);
	  groups[2] = (d+e+g+h) * (0.125f/4.0f);
	  groups[3] = (e+f+h+i) * (0.125f/4.0f);
	  groups[4] = (j+k+l+m) * (0.5f/4.0f);
	  groups[0] *= KarisAverage(groups[0]);
	  groups[1] *= KarisAverage(groups[1]);
	  groups[2] *= KarisAverage(groups[2]);
	  groups[3] *= KarisAverage(groups[3]);
	  groups[4] *= KarisAverage(groups[4]);
	  downsample = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
	  downsample = max(downsample, 0.0001f);
	  break;
	default:
	  downsample = e*0.125;                // ok
	  downsample += (a+c+g+i)*0.03125;     // ok
	  downsample += (b+d+f+h)*0.0625;      // ok
	  downsample += (j+k+l+m)*0.125;       // ok
	  break;
	}
	downsample /= (mipLevel + 1);
}
#endif 

#if 1
layout(binding = 0) uniform sampler2D image;
layout(location = 0) out vec3 FragColor;
layout(location = 1) in vec2 uv;

uniform vec2 srcResolution;
uniform int mipLevel = 1;
uniform int horizontal;


void main()
{             
	const float d = 1048576;
	const float w0 = float(1) / d;
	const float w1 = float(20) / d;
	const float w2 = float(190) / d;
	const float w3 = float(1140) / d;
	const float w4 = float(4845) / d;
	const float w5 = float(15504) / d;
	const float w6 = float(38760) / d;
	const float w7 = float(77520) / d;
	const float w8 = float(125970) / d;
	const float w9 = float(167960) / d;
	const float w10 = float(184756) / d;


    // float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    float weight[10] = float[] (w9, w8, w7, w6, w5, w4, w3, w2, w1, w0);
    
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    // vec2 tex_offset = 1.0 / srcResolution; // gets size of single texel
     vec3 result = texture(image, uv).rgb * w10; // current fragment's contribution
    
	if (horizontal == 1)
     {
		// horizontal 
		 for(int i = 0; i < weight.length(); ++i)
		 {
			 result += texture(image, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			 result += texture(image, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		 }
     }
	 else 
     {
		 // vertical 
		 for(int i = 0; i < weight.length(); ++i)
		 {
			 result += texture(image, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			 result += texture(image, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		 }
    }

	
    // FragColor = vec3(uv.x,uv.y,0);
    // FragColor = texture(image, uv).rgb;
    FragColor = result;
	// FragColor = max(vec3(0), log(result + vec3(0.5)) + vec3(0.3));
}
#endif 