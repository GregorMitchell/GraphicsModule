/*
* "assignment2.cpp"
* by Gregor Mitchell
* December 2020
*
* Loads, creates and displays a movable rocket with particle effect and terrain generation
*
* Adapted from "Assignment1.cpp" by Gregor Mitchell, 2020
* 
* Camera Controls adapted from https://learnopengl.com/Getting-started/Camera
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
#include "terrain_object.h"
#include "tiny_loader_texture.h"

/* Include the image loader */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "points2.h"

/* Define buffer object indices */
GLuint elementbuffer;
GLuint quad_vbo, quad_normals, quad_colours, quad_tex_coords;

GLuint program;
GLuint program2;
GLuint vao;

GLuint colourmode;
GLuint emitmode;

//position and view globals
glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp;
glm::mat4 view;
GLfloat cameraSpeed;
GLfloat lastX = 400, lastY = 300;
GLboolean firstMouse;
GLfloat camyaw, campitch;

GLuint drawmode;
GLuint numlats, numlongs;

GLfloat shipmove;
GLfloat shipcanmove;
GLfloat shipspeed;

GLfloat light_x, light_y, light_z;

/* Point sprite object and adjustable parameters */
points2* point_anim;
GLfloat speed;
GLfloat maxdist;
GLfloat point_size;

GLuint textureID1, textureID2, textureID3, textureID4, textureID5, textureID6;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID;
GLuint colourmodeID, emitmodeID;

GLuint modelID2, viewID2, projectionID2, lightposID2, normalmatrixID2, point_sizeID2;
GLuint colourmodeID2, emitmodeID2;

GLfloat aspect_ratio;
GLuint numspherevertices;

/* Define textureID*/
GLuint texID;

/* Global instances of our objects */
Sphere aSphere;

terrain_object* heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLuint land_resolution;

TinyObjLoader nose;
TinyObjLoader body;
TinyObjLoader engine;
TinyObjLoader fins;

using namespace std;
using namespace glm;

/* Function to load textures into the program */
bool load_texture(char* filename, GLuint& texID, bool bGenMipmaps);

bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps)
{
	glGenTextures(1, &texID);
	// local image parameters
	int width, height, nrChannels;

	/* load an image file using stb_image */
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data)
	{
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
			// texID[i] will now be the identifier for this specific texture
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
	else
	{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{
	/* Set the object transformation controls to their initial values */

	// Camera matrix
	cameraPos = glm::vec3(-30, 2, -10);
	cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	firstMouse = true;
	camyaw = 0.0f;
	campitch = 0.0f;

	//light position
	light_x = 60;
	light_y = 20;
	light_z = -20;

	//ship animation variables
	shipmove = 0;
	shipcanmove = 0;
	shipspeed = 0;

	aspect_ratio = 1.3333f;
	colourmode = 0;
	emitmode = 0;
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("terrain.vert", "terrain.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	try
	{
		program2 = glw->LoadShader("object.vert", "object.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* load an image file using stb_image */
	const char* filename1 = "\images\\nose.png";
	const char* filename2 = "\images\\body.png";
	const char* filename3 = "\images\\engine.png";
	const char* filename4 = "\images\\fins.png";
	const char* filename5 = "\images\\flame.png";
	const char* filename6 = "\images\\smoke.png";

	//load image files

	// This will flip the image so that the texture coordinates defined in
	// the sphere, match the image orientation as loaded by stb_image
	stbi_set_flip_vertically_on_load(true);
	if (!load_texture(filename1, textureID1, true))
	{
		cout << "Fatal error loading texture: " << filename1 << endl;
		exit(0);
	}

	if (!load_texture(filename2, textureID2, false))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	if (!load_texture(filename3, textureID3, false))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	if (!load_texture(filename4, textureID4, false))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	if (!load_texture(filename5, textureID5, false))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
		exit(0);
	}

	if (!load_texture(filename6, textureID6, false))
	{
		cout << "Fatal error loading texture: " << filename2 << endl;
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

	/* Define uniforms to send to vertex shader */
	modelID2 = glGetUniformLocation(program2, "model");
	colourmodeID2 = glGetUniformLocation(program2, "colourmode");
	emitmodeID2 = glGetUniformLocation(program2, "emitmode");
	viewID2 = glGetUniformLocation(program2, "view");
	projectionID2 = glGetUniformLocation(program2, "projection");
	lightposID2 = glGetUniformLocation(program2, "lightpos");
	normalmatrixID2 = glGetUniformLocation(program2, "normalmatrix");
	point_sizeID2 = glGetUniformLocation(program2, "size");

	/* Load and create our objects*/
	nose.load_obj("\obj\\nose.obj");
	body.load_obj("\obj\\body.obj");
	engine.load_obj("\obj\\engine.obj");
	fins.load_obj("\obj\\fins.obj");

	/* Create the heightfield object */
	octaves = 10;
	perlin_scale = 10.f;
	perlin_frequency = 2.f;
	land_size = 100.f;
	land_resolution = 200;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(land_resolution, land_resolution, land_size, land_size);
	heightfield->setColourBasedOnHeight();
	heightfield->createObject();

	/* create our sphere object */
	aSphere.makeSphere(numlats, numlongs);

	// Create our quad and texture 
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	/* Define the texture behaviour parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	// This is the location of the texture object (TEXTURE0), i.e. tex1 will be the name
	// of the sampler in the fragment shader
	int loc = glGetUniformLocation(program, "tex1");
	if (loc >= 0) glUniform1i(loc, 0);

	//variables for particle animation
	speed = 0.005f;
	maxdist = 0.6f;
	point_anim = new points2(10000, maxdist, speed);
	point_anim->create();
	point_size = 4;

	// Enable gl_PointSize
	glEnable(GL_PROGRAM_POINT_SIZE);
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

	/* Enable Blending for the analytic point sprite */
	glEnable(GL_BLEND);

	/* Make the compiled shader program current */
	glUseProgram(program);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 1000.0f);

	//define camera matrix
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	cameraSpeed = 0.5f;

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		glUniform1ui(colourmodeID, 0);
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.75f, 0.75f, 0.75f)); // make a small sphere
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

	//TERRAIN
	model.push(model.top());
	{
		// Send the model uniform to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		/* Draw our heightfield */
		heightfield->drawObject(drawmode);
	}
	model.pop();

	/* Change current shader */
	glUseProgram(program2);

	//NOSE
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(4, 0, -10));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f));

		model.top() = translate(model.top(), vec3(0, shipmove, 0));

		// Send the model uniform to the currently bound shader,
		glUniformMatrix4fv(modelID2, 1, GL_FALSE, &(model.top()[0][0]));
		glUniform1ui(colourmodeID2, colourmode);
		glUniformMatrix4fv(viewID2, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID2, 1, GL_FALSE, &projection[0][0]);
		glUniform4fv(lightposID2, 1, value_ptr(lightpos));

		glBindTexture(GL_TEXTURE_2D, textureID1);
		nose.drawObject(drawmode);
	}
	model.pop();

	//BODY
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(4, 0, -10));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f));

		model.top() = translate(model.top(), vec3(0, shipmove, 0));

		// Send the model uniform to the currently bound shader,
		glUniformMatrix4fv(modelID2, 1, GL_FALSE, &(model.top()[0][0]));
		glUniform1ui(colourmodeID2, colourmode);
		glUniformMatrix4fv(viewID2, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID2, 1, GL_FALSE, &projection[0][0]);
		glUniform4fv(lightposID2, 1, value_ptr(lightpos));

		glBindTexture(GL_TEXTURE_2D, textureID2);
		body.drawObject(drawmode);
	}
	model.pop();

	//ENGINE
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(4, 0, -10));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f));

		model.top() = translate(model.top(), vec3(0, shipmove, 0));

		// Send the model uniform to the currently bound shader,
		glUniformMatrix4fv(modelID2, 1, GL_FALSE, &(model.top()[0][0]));
		glUniform1ui(colourmodeID2, colourmode);
		glUniformMatrix4fv(viewID2, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID2, 1, GL_FALSE, &projection[0][0]);
		glUniform4fv(lightposID2, 1, value_ptr(lightpos));

		glBindTexture(GL_TEXTURE_2D, textureID3);
		engine.drawObject(drawmode);
	}
	model.pop();

	//FINS
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(4, 0, -10));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f));

		model.top() = translate(model.top(), vec3(0, shipmove, 0));

		// Send the model uniform to the currently bound shader,
		glUniformMatrix4fv(modelID2, 1, GL_FALSE, &(model.top()[0][0]));
		glUniform1ui(colourmodeID2, colourmode);
		glUniformMatrix4fv(viewID2, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID2, 1, GL_FALSE, &projection[0][0]);
		glUniform4fv(lightposID2, 1, value_ptr(lightpos));

		glBindTexture(GL_TEXTURE_2D, textureID4);
		fins.drawObject(drawmode);
	}
	model.pop();

	//PARTICLES
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(4, 5, -10));
		model.top() = rotate(model.top(), -radians(180.0f), glm::vec3(1, 0, 0));
		model.top() = rotate(model.top(), -radians(90.0f), glm::vec3(0, 1, 0));
		model.top() = scale(model.top(), vec3(150.f, 50.f, 150.f));


		model.top() = translate(model.top(), vec3(0, -(shipmove / 500), 0));

		// Send our uniforms variables to the currently bound shader,
		glUniformMatrix4fv(modelID2, 1, GL_FALSE, &model.top()[0][0]);
		glUniform1ui(colourmodeID2, colourmode);
		glUniform1f(point_sizeID2, point_size);
		glUniformMatrix4fv(viewID2, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID2, 1, GL_FALSE, &projection[0][0]);

		//change texture depending on if the rocket is moving
		if (shipcanmove == 1) {
			glBindTexture(GL_TEXTURE_2D, textureID5);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, textureID6);
		}

		point_anim->draw();
		point_anim->animate();
	}
	model.pop();

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	//ship animation
	if (shipcanmove == 1 && shipspeed < 10) {
		shipspeed += 0.1;
	}
	shipmove += shipspeed;

	if (cameraPos.x > -8.5) {
		cameraPos.x = -8.5;
	}
	if (cameraPos.x < -45) {
		cameraPos.x = -45;
	}
	if (cameraPos.z > -3.5) {
		cameraPos.z = -3.5;
	}
	if (cameraPos.z < -32.5) {
		cameraPos.z = -32.5;
	}
}



/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);


	//camera controls
	if (key == 'W') {
		if (cameraPos.x >= -45 && cameraPos.x <= -8.5) {
			if (cameraPos.z >= -32.5 && cameraPos.z <= -3.5) {
				cameraPos += cameraSpeed * cameraFront * vec3(1.0f, 0.0f, 1.0f);
			}
		}
	}
	if (key == 'S') {
		if (cameraPos.x >= -45 && cameraPos.x <= -8.5) {
			if (cameraPos.z >= -32.5 && cameraPos.z <= -3.5) {
				cameraPos -= cameraSpeed * cameraFront * vec3(1.0f, 0.0f, 1.0f);
			}
		}
	}
	if (key == 'A') {
		if (cameraPos.x >= -45 && cameraPos.x <= -8.5) {
			if (cameraPos.z >= -32.5 && cameraPos.z <= -3.5) {
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * vec3(1.0f, 0.0f, 1.0f), cout << "z = " << cameraPos.z << endl;
			}
		}
	}
	if (key == 'D') {
		if (cameraPos.x >= -45 && cameraPos.x <= -8.5) {
			if (cameraPos.z >= -32.5 && cameraPos.z <= -3.5) {
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * vec3(1.0f, 0.0f, 1.0f), cout << "z = " << cameraPos.z << endl;
			}
		}
	}

	//start and reset the animation
	if (key == 'T') shipcanmove = 1;
	if (key == 'G') shipmove = 0, shipspeed = 0, shipcanmove = 0;

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}
}

//use mouse input to determine where the camera is looking
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	camyaw += xoffset;
	campitch += yoffset;

	if (campitch > 89.0f)
		campitch = 89.0f;
	if (campitch < -89.0f)
		campitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(camyaw)) * cos(glm::radians(campitch));
	direction.y = sin(glm::radians(campitch));
	direction.z = sin(glm::radians(camyaw)) * cos(glm::radians(campitch));
	cameraFront = glm::normalize(direction);
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1024, 768, "Gregor Mitchell - Assignment 2");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setCursorPosCallback(mouseCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

