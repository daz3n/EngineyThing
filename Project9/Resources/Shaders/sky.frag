#version 450
layout(location = 0) out vec4 outColor;



layout(location = 1) in vec2 uv;
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






// https://codesandbox.io/s/fractal-noise-gpu-283k0?file=/src/noise.ts:25-952
// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
vec3 noise(vec3 position)
{
	float frequency = 1.44;
	float amplitude = 25;
	int octaves = 4;
	float time = _runtime * 0.00025;

    vec3 pos = position;

    float f = frequency;
    float a = amplitude;

    float t = mod(time, 100.0);

    for(int i = 0; i < octaves; i++) {
      pos.z += snoise(uv.xy * f + vec2(t, 0.)) * a;
      f *= 2.0;
      a *= 0.5;
    }

	return pos;
}


void main() 
{
	// translate forward ray by frustum 
	vec4 RayO = _position;
	vec4 RayD = inverse(_projection * _view) * vec4(2 * uv.x - 0.5, 2 * uv.y - 0.5, 1, 1);
	
	
	float Up = max(0, dot(RayD.xyz,vec3(0, 1,0)));
	
	float Dim = 0.25;
	vec3 SkyColor = vec3(Up * Dim, Up*Dim,Up*Dim + 0.5);

	// this is just the cloud density at that position.
	// i think i can make them look a lot better by doing fancy rayracing.
	float Clouds2 = max(0, noise(RayD.xyz).z * 0.05);
	float Clouds = min(1, max(0, Up - 0.5) * Clouds2);
	float CloudDensity = smoothstep(0,1, Clouds);
	vec3 CloudColor = vec3(CloudDensity);


    // outColor = vec4(Up*Dim, Up*Dim, Up*Dim + 0.5, 1.0);
	outColor = vec4(SkyColor, 1);
	outColor += vec4(CloudColor, 1);
}