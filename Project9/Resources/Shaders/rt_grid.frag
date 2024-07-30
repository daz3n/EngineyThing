#version 450

layout(location = 0) out vec3 outColor;


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

layout(location = 1) in vec2 uv;

float time_of_day()
{
	return sin(_runtime * 0.0001);
}
float clamped_time_of_day()
{
	return max(0, time_of_day());
}


vec3 sky_color(float angle)
{
	vec3 DarkNight = vec3(0.04 * angle, 0.02 * angle,0.05 * angle);
	vec3 LightNight = min(vec3(1), vec3(0.2 * angle,0.2 * angle,0.35 * angle) * 5);
	vec3 MixNight = mix(LightNight, DarkNight, clamped_time_of_day());



	// vec3 NightSky = vec3(0.2 * angle,0.2 * angle,0.5 * angle) * 5;
	// vec3 DaySky = vec3(0.7 * angle, 0.55 * angle, 0.9 * angle) * 5;


	vec3 DarkDay = vec3(0.44 * angle, 0.32 * angle,0.55 * angle);
	vec3 LightDay = min(vec3(1), vec3(0.9 * angle,0.7 * angle,0.7 * angle) * 5);
	vec3 MixDay = mix(LightDay, DarkDay, clamped_time_of_day());


	float StretchDaytime = min(1, clamped_time_of_day() * 2);
	vec3 SkyCol =  mix(MixNight, MixDay, StretchDaytime);
	
	return SkyCol;
}

vec3 sky(vec3 ro, vec3 rd, out float hit)
{
	// plane intersection 
	float d = dot(rd,vec3(0,1,0));

    
	if (d < 0)
	{
		// will intersect the ground!
		return vec3(0.0);
	}
	// sky!
	vec3 sun_position = normalize(vec3(-cos(time_of_day()),sin(time_of_day()), 0));

	// do i hit the sun?
	float Dot = dot(rd, sun_position);

	if (Dot > 0.9995)
	{
		hit = 1;
		return vec3(mix(sky_color(d), vec3(1,0.9,0.85), smoothstep(0.9995, 1.0, Dot + 0.002)));
	}
	
	hit = 1;
	return sky_color(d);
}







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
float noise(vec3 position)
{
	float frequency = 1.44;
	float amplitude = 25;
	int octaves = 4;
	float time = clamped_time_of_day();

    vec3 pos = position;

    float f = frequency;
    float a = amplitude;

    float t = mod(time, 100.0);

    for(int i = 0; i < octaves; i++) {
      pos.z += snoise(uv.xy * f + vec2(t, 0.)) * a;
      f *= 2.0;
      a *= 0.5;
    }

	return pos.z;
}

vec3 clouds(vec3 ro, vec3 rd, out float hit)
{
	// cast against a noise function to generate clouds.
	// each cloud absorbs the sky light behind it, shifting the wavelength.

	// the density of the cloud between camera and the sun determines how much light gets through.
	// higher density absorbs light.

	// instead of having a density cutoff, instead use the density as a divisor.
	// values of 0 must result in no change.
	// values between 0 and 1 must result in a brighter light.
	// values over 1 must dim the light.

	// this looks a bit like a sin wave.
	// steps:
	// 1. generate noise for my cloud density
	// 2. remap the density to between 0 and 1.
	// 3. light modifier = 0.8 + sin(1.4 * x + 0.2)
	// 
	// float modifier = noise(rd) * 0.005;
	
	float modifier = noise(rd);
	float mod2 = max(0,dot(rd, vec3(0,1,0)));

	
	return vec3(modifier * mod2);
}




vec3 ground(vec3 ro, vec3 rd, out float hit)
{
	// Define the normal of the ground plane (assuming it's y=0)
    vec3 planeNormal = vec3(0, 1, 0);
    float planeD = 0.0; // The ground plane is at y=0

    // Calculate the dot product between the ray direction and the plane normal
    float denom = dot(rd, planeNormal);
    bool will_hit_ground = denom < 0;

	 if (will_hit_ground)
    {
        // Calculate the intersection point t using the plane equation
        float t = (planeD - dot(planeNormal, ro)) / denom;

        if (t >= 0.0) // Ensure the intersection is in front of the ray origin
        {
            hit = t;
            vec3 pos = ro + (rd * t);

			vec3 ret = vec3(0);

            // Optional: highlight grid lines at integer boundaries
            if (mod(abs(pos.x), 1) < 0.01) ret.x = 50;
            if (mod(abs(pos.z), 1) < 0.01) ret.z = 50;
			if (distance(pos,vec3(0)) <= 1.5) 
			{
				vec3 MaxCol = vec3(50,45,20);
				
				if (ret.x == 50 || ret.z == 50)
				{
					float dst = distance(pos,vec3(0));
					ret.y = MaxCol.y / ((1 + 0.5 * dst));
				}
			}

			ret /= 0.75 * (1 + distance(pos, _position.xyz));

			return abs(ret) / 50.0;
        }
    }

    // No intersection
    hit = 1000.0;
    return vec3(0.0);
}

vec3 checkerboard(vec3 ro, vec3 rd, out float hit)
{
	vec3 checker_color_1 = vec3(1);
	vec3 checker_color_2 = vec3(0);

	// Define the normal of the ground plane (assuming it's y=0)
    vec3 planeNormal = vec3(0, 1, 0);
    float planeD = 0.0; // The ground plane is at y=0

    // Calculate the dot product between the ray direction and the plane normal
    float denom = dot(rd, planeNormal);
    bool will_hit_ground = denom < 0;

	 if (will_hit_ground)
    {
        // Calculate the intersection point t using the plane equation
        float t = (planeD - dot(planeNormal, ro)) / denom;

        if (t >= 0.0) // Ensure the intersection is in front of the ray origin
        {
            hit = t;
            vec3 pos = ro + (rd * t);

			vec3 ret = vec3(0);

            
			// any coordinate needs to be even or odd.
			// how to do that?
			vec2 coords = pos.xz;
			vec2 zeros_or_ones = mod(coords,vec2(2));
			
			float tileid = 0;
			if (zeros_or_ones.x > 1) tileid += 1;
			if (zeros_or_ones.y > 1) tileid += 1;

			if (tileid == 1) return checker_color_1;


			return checker_color_2;
			
        }
    }

    // No intersection
    hit = 1000.0;
    return vec3(0.0);

}


void main() 
{
	// intersect any grass on the 0,0 xy plane
	vec3 fwd = normalize(_target.xyz - _position.xyz); // 
	vec3 right = normalize(cross(fwd, vec3(0,1,0)));
	vec3 up = normalize(cross(right, fwd));


	// remap the uv coordinates to -1, 1
	vec2 correct_uv = (uv - vec2(0.5)) * 2;
	float aspect = _resolution.x / _resolution.y;
	correct_uv.y /= aspect;
	// 
	

	// create the ray to cast into the scene
	vec3 ro = _position.xyz;
	vec3 rd = normalize(fwd + (right * correct_uv.x) + (up * correct_uv.y));
	// 


	// test the ray against all objects in the scene
	float hit = 1000;
	outColor = vec3(0);
	outColor = ground(ro,rd,hit);
	

	outColor += sky(ro, rd, hit);
	// outColor += clouds(ro, rd, hit);
	outColor += checkerboard(ro,rd,hit) * 0.05;
}



