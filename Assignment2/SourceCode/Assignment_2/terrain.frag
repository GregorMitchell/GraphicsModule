/* 
* "terrain.frag"
* by Gregor Mitchell
* November 2020
* 
* Fragment shader demonstarting positional lighting
* 
* Adapted from "poslight.frag" and "poslight.vert" by Iain Martin, 2018
*/

#version 420 core

//variables in from vertex shader
in vec4 fcolour;
in vec4 P;
in vec3 N;
in vec3 L;

in vec3 V;
in vec3 R;

in vec3 emissive;

in float distanceToLight;

vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
int  shininess = 8;

out vec4 outputColor;

void main()
{
	//diffuse colour
	vec4 diffuse_albedo = fcolour;

	//ambient lighting calculated
	vec3 ambient = diffuse_albedo.xyz *0.2;

	//difuse lighting calculated
	vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo.xyz;

	//specular lighting calculated
	vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo;

	//attenuation lighting calculated
	float attenuation_k1 = 0.05;
	float attenuation_k2 = 0.05;
	float attenuation_k3 = 0.05;
	float attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + 
								   attenuation_k3 * pow(distanceToLight, 2));

	//calculate all lighting together							   
	outputColor = vec4(ambient + (( diffuse + specular) + emissive), 1.0);
}