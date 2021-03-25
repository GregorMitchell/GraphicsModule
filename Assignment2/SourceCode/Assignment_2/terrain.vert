/* 
* "terrain.vert"
* by Gregor Mitchell
* December 2020
* 
* Vertex shader demonstarting positional lighting
* 
* Adapted from "poslight.vert" by Iain Martin, 2018
*/

// Specify minimum OpenGL version
#version 420 core

// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

//variables passed onto the fragment shader
out vec4 fcolour;

out vec4 P;
out vec3 N;
out vec3 L;

out vec3 V;
out vec3 R;

out vec3 emissive;
out float distanceToLight;

//uniforms from the application
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode, emitmode;
uniform vec4 lightpos;


//global constants
vec3 specular_albedo = vec3(1.0, 0.8, 0.6);

void main()
{
	emissive = vec3(0);

	vec4 position_h = vec4(position, 1.0);
	vec4 diffuse_albedo;
	vec3 light_pos3 = lightpos.xyz;			

		diffuse_albedo = colour;

	vec3 ambient = diffuse_albedo.xyz *0.2;

	//defining and calculating variables to pass onto the fragment shader
	mat4 mv_matrix = view * model;
	P = mv_matrix * position_h;
	N = normalize(normalmatrix * normal);
	L = light_pos3 - P.xyz;
	distanceToLight = length(L);
	L = normalize(L);
	V = normalize(-P.xyz);	
	R = reflect(-L, N);

	//if the object should emit light, emissive lighting is calculated
	if (emitmode == 1) emissive = vec3(1.0, 1.0, 0.8); 

	fcolour = colour;

	gl_Position = (projection * view * model) * position_h;
}


