/*
* "pyramid.cpp"
* by Gregor Mitchell
* November 2020
*
* Create a pyramid object
*
* Adapted from "tetrahedron.cpp" by Iain Martin, 2018
*/


// Link to static libraries, could define these as linker inputs in the project settings instead if you prefer
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

// Include the header to the GLFW wrapper class which also includes the OpenGL extension initialisation
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

// Include GLM core and matrix extensions
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cylinder.h"
#include "cube.h"
#include "pyramid.h"
#include <iostream>

using namespace std;

// Define the vertex attributes for vertex positions and normals for the pyramid
Pyramid::Pyramid()
{
	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;
	numvertices = 18;
}


Pyramid::~Pyramid()
{
}


void Pyramid::makePyramid()
{
	glm::vec3 pyra_normals[18];	// Array for normals for flat shaded pyramid

	// Define vertices as glm:vec3 type to make it easier to calculate normals
	glm::vec3 pyra_vertices[] = {
		glm::vec3(0, 0.5f, 0), glm::vec3(-0.5f, 0, 0.5f), glm::vec3(0.5f, 0, 0.5f),
		glm::vec3(0, 0.5f, 0), glm::vec3(0.5f, 0, 0.5f), glm::vec3(0.5, 0, -0.5f),
		glm::vec3(0, 0.5f, 0), glm::vec3(-0.5f, 0, -0.5f), glm::vec3(-0.5f, 0, 0.5f),
		glm::vec3(0, 0.5f, 0), glm::vec3(0.5f, 0, -0.5f), glm::vec3(-0.5f, 0, -0.5f),
		glm::vec3(0.5f, 0, -0.5f), glm::vec3(0.5f, 0, 0.5f), glm::vec3(-0.5, 0, 0.5f),
		glm::vec3(-0.5f, 0, 0.5f), glm::vec3(-0.5f, 0, -0.5f), glm::vec3(0.5, 0, -0.5f),

	};

	// Specify the vertex buffer
	glGenBuffers(1, &pyra_buffer_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, pyra_buffer_vertices);
	glBufferData(GL_ARRAY_BUFFER, numvertices * sizeof(glm::vec3), &pyra_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//define colours for each face of the pyramid
	GLfloat pyra_colours[] = {
		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,

		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,

		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,

		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,

		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,

		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,
		0.3f, 0.0f, 0.0f, 1.0f,
	};

	/* Specify the colour buffer */
	glGenBuffers(1, &pyra_buffer_colours);
	glBindBuffer(GL_ARRAY_BUFFER, pyra_buffer_colours);
	glBufferData(GL_ARRAY_BUFFER, numvertices * sizeof(GLfloat) * 4, &pyra_colours[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Calculate the normals for each triangle, then set each set of three normals to be the same
	for (int v = 0; v < numvertices; v+=3)
	{
		glm::vec3 normal = glm::cross(pyra_vertices[v + 1] - pyra_vertices[v],
									  pyra_vertices[v + 2] - pyra_vertices[v]);
		pyra_normals[v] = pyra_normals[v + 1] = pyra_normals[v + 2] = normal;
	}
	

	/* Define a buffer of the vertex normals */
	glGenBuffers(1, &pyra_buffer_normals);
	glBindBuffer(GL_ARRAY_BUFFER, pyra_buffer_normals);
	glBufferData(GL_ARRAY_BUFFER, numvertices * sizeof(glm::vec3), &pyra_normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Pyramid::drawPyramid(int drawmode)
{
	/* Bind the vertices */
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, pyra_buffer_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind the colours */
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, pyra_buffer_colours);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind the normals */
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, pyra_buffer_normals);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (drawmode == 2)
	{
		// Draw the vertices
		glPointSize(3.f);  // Set the point size when drawing vertices
		glDrawArrays(GL_POINTS, 0, numvertices);
	}
	else
	{
		// Draw the triangles
		glDrawArrays(GL_TRIANGLES, 0, numvertices);
	}
}
