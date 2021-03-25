/*
* "assignment1.cpp"
* by Gregor Mitchell
* November 2020
*
* Creates and displays a moving train
* User can alter movement of train, position of light, camera position
*
* Adapted from "poslight.cpp" by Iain Martin, 2018
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cylinder.h"
#include "cube.h"
#include "pyramid.h"

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program;
GLuint vao;

GLuint colourmode;
GLuint emitmode;

//position and view globals
GLfloat vx, vy, vz, cam_y;
GLfloat angle_y, angle_inc_y;
GLuint drawmode;
GLuint numlats, numlongs;
GLfloat speed;

GLfloat bar_speed;
GLfloat bar_x, bar_x2, bar_y, bar_y2;
GLfloat bar_x_inc, bar_x_inc2, bar_y_inc, bar_y_inc2;

GLfloat wheel_speed, wheel_speed_inc;

GLfloat light_x, light_y, light_z;

GLuint ani_start;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID;
GLuint colourmodeID, emitmodeID;

GLfloat aspect_ratio;
GLuint numspherevertices;

/* Global instances of our objects */
Sphere aSphere;
Cube aCube;
Pyramid aPyramid;
Cylinder bodyCylinder;
Cylinder wheelCylinder;

using namespace std;
using namespace glm;

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	speed = 0.1f;
	bar_speed = 0.01f;

	vx = 0; 
	vy = 0;
	vz = 12;
	cam_y = 6;

	light_x = 0; 
	light_y = 3; 
	light_z = 3;

	angle_y = 0;
	angle_inc_y = 0;

	aspect_ratio = 1.3333f;
	colourmode = 0; 
	emitmode = 0;
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere

	bar_x = 0;
	bar_x2 = 0;
	bar_y = 0;
	bar_y2 = 0;
	bar_x_inc = 0;
	bar_x_inc2 = 0;
	bar_y_inc = 0;
	bar_y_inc2 = 0;

	wheel_speed = 0;
	wheel_speed_inc = -2;

	ani_start = 1;

	//colours passed in to cylinder
	bodyCylinder = Cylinder(vec3(0.3f, 0.f, 0.f));
	wheelCylinder = Cylinder(vec3(0.2f, 0.2f, 0.2f));

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("assignment1.vert", "assignment1.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	lightposID = glGetUniformLocation(program, "lightpos");
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");

	/* create our sphere and cube objects */
	aSphere.makeSphere(numlats, numlongs);
	bodyCylinder.makeCylinder();
	wheelCylinder.makeCylinder();
	aCube.makeCube();
	aPyramid.makePyramid();
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.f, 0.f, 0.f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, cam_y, vz), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Apply rotations to the view position. This wil get appleid to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis

	// Define the light position and transform by the view matrix
	vec4 lightpos = view *  vec4(light_x, light_y, light_z, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		glUniform1ui(colourmodeID, 0);
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitmodeID, emitmode);
		aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID, emitmode);
	}
	model.pop();

	glUniform1ui(colourmodeID, colourmode);

	// Define the global model transformations (rotate and scale). Note, we're not modifying the light source position
	model.top() = rotate(model.top(), -radians(angle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis



	// CABIN RECT
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = scale(model.top(), vec3(3.f, 3.f, 3.f));
		model.top() = translate(model.top(), vec3(0.25f, 0, 0));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	//ROOF PYRAMID
	model.push(model.top());
	{
		model.top() = scale(model.top(), vec3(1.75f, 1.5f, 1.75f));
		model.top() = translate(model.top(), vec3(0.425f, 0.5f, 0));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our pyramid*/
		aPyramid.drawPyramid(drawmode);
	}
	model.pop();

	// BASE RECT
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(-0.75f, -0.75f, 0));
		model.top() = scale(model.top(), vec3(9.f, 0.25f, 2.25f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	// UNDERCARRIAGE RECT
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(-0.625f, -1.25f, 0));
		model.top() = scale(model.top(), vec3(8.5f, 1.75f, 2.f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	//LONG CYLINDER
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-1.5f, -0.25f, 0));
		model.top() = scale(model.top(), vec3(3.f, 0.5f, 0.5f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(0, 1, 0));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));

		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bodyCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	//CHIMNEY CYLINDER
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-2.5f, 0.49f, 0));
		model.top() = scale(model.top(), vec3(0.125f, 0.5f, 0.125f));

		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		bodyCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	//BIG WHEEL 1
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0.7f, -1.725f, 0.5625f));
		model.top() = rotate(model.top(), -radians(wheel_speed), glm::vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.675f, 0.675f, 0.125f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));


		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		wheelCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	//BIG WHEEL 2
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(0.7f, -1.725f, -0.5625f));
		model.top() = rotate(model.top(), -radians(wheel_speed), glm::vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.675f, 0.675f, 0.125f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));


		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		wheelCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	//MEDIUM WHEEL 1
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-0.85f, -1.725f, 0.5625f));
		model.top() = rotate(model.top(), -radians(wheel_speed), glm::vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.675f, 0.675f, 0.125f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));


		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		wheelCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	//MEDIUM WHEEL 2
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-0.85f, -1.725f, -0.5625f));
		model.top() = rotate(model.top(), -radians(wheel_speed), glm::vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.675f, 0.675f, 0.125f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));


		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		wheelCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	//SMALL WHEEL 1
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-2.15f, -1.85f, 0.5625f));
		model.top() = rotate(model.top(), -radians(wheel_speed), glm::vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.5f, 0.5f, 0.125f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));


		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		wheelCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	//SMALL WHEEL 2
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(-2.15f, -1.85f, -0.5625f));
		model.top() = rotate(model.top(), -radians(wheel_speed), glm::vec3(0, 0, 1));
		model.top() = scale(model.top(), vec3(0.5f, 0.5f, 0.125f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(1, 0, 0));


		// Recalculate the normal matrix and send the model and normal matrices to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		wheelCylinder.drawCylinder(drawmode); // Draw our cylinder
	}
	model.pop();

	// PISTON RECT 1
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(-0.6f, -1.715f, 0.6875f));
		model.top() = translate(model.top(), vec3(bar_x, bar_y, 0));
		model.top() = scale(model.top(), vec3(3.125f, 0.5f, 0.25f));

			if (bar_x <= 0)
			{
				bar_x_inc += bar_speed;
			}
			else if (bar_x >= 1)
			{
				bar_x_inc -= bar_speed;
			}

			if (bar_y == 0)
			{
				bar_y_inc -= bar_speed;
			}
			else if (bar_y <= -0.5)
			{
				bar_y_inc += bar_speed;
			}
			else if (bar_y >= 0.5)
			{
				bar_y_inc -= bar_speed;
			}

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	// PISTON RECT 2
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(-0.6f, -1.715f, -0.6875f));
		model.top() = translate(model.top(), vec3(bar_x2, bar_y2, 0));
		model.top() = scale(model.top(), vec3(3.125f, 0.5f, 0.25f));

		if (bar_x2 <= 0)
		{
			bar_x_inc2 += bar_speed;
		}
		else if (bar_x2 >= 1)
		{
			bar_x_inc2 -= bar_speed;
		}

		if (bar_y2 == 0)
		{
			bar_y_inc2 -= bar_speed;
		}
		else if (bar_y2 <= -0.5)
		{
			bar_y_inc2 += bar_speed;
		}
		else if (bar_y2 >= 0.5)
		{
			bar_y_inc2 -= bar_speed;
		}

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
	}
	model.pop();

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_y += angle_inc_y;
	bar_x += bar_x_inc * ani_start;
	bar_x2 += bar_x_inc2 * ani_start;
	bar_y += bar_y_inc * ani_start;
	bar_y2 += bar_y_inc2 * ani_start;
	wheel_speed += wheel_speed_inc;
}



/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//rotate whole model
	if (key == 'Q') angle_inc_y += speed;
	if (key == 'W') angle_inc_y -= speed;

	//starts/stops animation
	if (key == 'E' && action != GLFW_PRESS)
	{
		if (ani_start == 1) {
			ani_start = 0;
			wheel_speed_inc = 0;
		}
		else if (ani_start == 0) 
		{
			ani_start = 1;
			wheel_speed_inc = -2;
		}
	};

	//change light position
	if (key == 'A') light_x -= speed;
	if (key == 'S') light_x += speed;
	if (key == 'D') light_y -= speed;
	if (key == 'F') light_y += speed;
	if (key == 'G') light_z -= speed;
	if (key == 'H') light_z += speed;

	//camera controls
	if (key == 'I' && vx > -18) vx -= 1.f;
	if (key == 'K' && vx < 84) vx += 1.f;
	if (key == 'J') vy -= 1.f;
	if (key == 'L') vy += 1.f;
	if (key == 'O' && vz > 5) vz -= 0.4, cam_y -= 0.2, cout << "vz = " << vz << endl;
	if (key == 'U') vz += 0.4, cam_y += 0.2, cout << "vz = " << vz << endl;


	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'R' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Position light example");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

