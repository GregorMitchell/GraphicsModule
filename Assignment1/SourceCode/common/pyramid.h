/* 
* "pyramid.h"
* by Gregor Mitchell
* November 2020
* 
* Example class to create a pyramid object
* 
* Adapted from "tetrahedron.h" by Iain Martin, 2018
*/

#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class Pyramid
{
public:
	Pyramid();
	~Pyramid();

	/* function prototypes */
	void makePyramid();
	void drawPyramid(int drawmode);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<GLushort> elements;

	// Define vertex buffer object names (e.g as globals)
	GLuint pyra_buffer_normals;
	GLuint pyra_buffer_vertices;
	GLuint pyra_buffer_colours;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numvertices;
	int drawmode;
};
