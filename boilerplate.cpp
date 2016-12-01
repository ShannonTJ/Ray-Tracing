// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Modifications by: Shannon TJ 10101385

// Date:    Fall 2016
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include "ImageBuffer.h"
#include <math.h>

// Specify that we want the OpenGL core profile before including GLFW headers
#ifndef LAB_LINUX
	#include <glad/glad.h>
#else
	#define GLFW_INCLUDE_GLCOREARB
	#define GL_GLEXT_PROTOTYPES
#endif
#include <GLFW/glfw3.h>


float x = 0.f;
float y = 0.f;
float z = 0.f; 

using namespace std;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
	// OpenGL names for vertex and fragment shaders, shader program
	GLuint  vertex;
	GLuint  fragment;
	GLuint  program;

	// initialize shader and program names to zero (OpenGL reserved value)
	MyShader() : vertex(0), fragment(0), program(0)
	{}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
	// load shader source from files
	string vertexSource = LoadSource("vertex.glsl");
	
	string fragmentSource = LoadSource("fragment.glsl");	
			
	if (vertexSource.empty() || fragmentSource.empty()) return false;

	// compile shader source into shader objects
	shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	// link shader program
	shader->program = LinkProgram(shader->vertex, shader->fragment);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
	// unbind any shader programs and destroy shader objects
	glUseProgram(0);
	glDeleteProgram(shader->program);
	glDeleteShader(shader->vertex);
	glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint  colourBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	MyGeometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
	{}
};

// create buffers and fill with geometry data, returning true if successful
bool InitializeGeometry(MyGeometry *geometry)
{
	// vertex positions of a line
	const GLfloat vertices[][2] = {
		{ -1.0f, -1.0f},
		{ 1.0f, -1.0f},
	    { -1.0f, 1.0f},
	    
		{ 1.0f, -1.0f},
	    { -1.0f, 1.0f},
	    { 1.f, 1.f}
	};
	
	const GLfloat colours[][3] = {
		{ 0.0f, 1.0f, 0.0f}
		};
	
	geometry->elementCount = 6; 

	// these vertex attribute indices correspond to those specified for the
	// input variables in the vertex shader
	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;

	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// create another one for storing our colours
	glGenBuffers(1, &geometry->colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);

	// create a vertex array object encapsulating all our vertex attributes
	glGenVertexArrays(1, &geometry->vertexArray);
	glBindVertexArray(geometry->vertexArray);

	// associate the position array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glVertexAttribPointer(VERTEX_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	// assocaite the colour array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	// unbind our buffers, resetting to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
	glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader)
{
	// clear screen to a dark grey colour
	//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(shader->program);
	glBindVertexArray(geometry->vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);

	// reset state to default (no shader or geometry bound)
	glBindVertexArray(0);
	glUseProgram(0);

	// check for an report any OpenGL errors
	CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions
vector<float> light;

vector<float> planeVertices;
vector<float> planeColors;
vector<float> planeLight;

vector<float> sphereVertices;
vector<float> sphereColors;
vector<float> sphereLight;

vector<float> triangleVertices;
vector<float> triangleColors;
vector<float> triangleLight;

void scene1Vertices(MyShader shader)
{
	//clear values when switching scenes
	light.clear();
	
	planeVertices.clear();
	planeColors.clear();
	planeLight.clear();
	
	sphereVertices.clear();
	sphereColors.clear();
	sphereLight.clear();
	
	triangleVertices.clear();
	triangleColors.clear();
	triangleLight.clear();
	
	//light info
	light.push_back(0.0f);light.push_back(2.5f);light.push_back(-7.75f);
	
	//plane info
	planeVertices.push_back(0.0f);planeVertices.push_back(0.0f);planeVertices.push_back(1.0f);
	planeVertices.push_back(0.0f);	planeVertices.push_back(0.0f);planeVertices.push_back(-10.5f);		
	
	planeColors.push_back(1.0f);planeColors.push_back(1.0f);planeColors.push_back(1.0f);
	planeLight.push_back(0.3f);planeLight.push_back(0.5f);planeLight.push_back(0);planeLight.push_back(0);
	
	//sphere info
	sphereVertices.push_back(0.9f);sphereVertices.push_back(-1.925f);sphereVertices.push_back(-6.69f);
	sphereVertices.push_back(0.825f);
	
	sphereColors.push_back(0.40f);sphereColors.push_back(0.40f);sphereColors.push_back(0.40f);
	sphereLight.push_back(0.5f);sphereLight.push_back(0.5f);sphereLight.push_back(1);sphereLight.push_back(50);
	
	//triangle info
	//blue pyramid
	triangleVertices.push_back(-0.4f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-9.55f);
	triangleVertices.push_back(-0.93f);triangleVertices.push_back(0.55f);triangleVertices.push_back(-8.51f);
	triangleVertices.push_back(0.11f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-7.98f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);
	triangleLight.push_back(0.5f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);
	
	triangleVertices.push_back(0.11f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-7.98f);
	triangleVertices.push_back(-0.93f);triangleVertices.push_back(0.55f);triangleVertices.push_back(-8.51f);
	triangleVertices.push_back(-1.46f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-7.47f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);
	triangleLight.push_back(0.5f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);
	
	triangleVertices.push_back(-1.46f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-7.47f);
	triangleVertices.push_back(-0.93f);triangleVertices.push_back(0.55f);triangleVertices.push_back(-8.51f);
	triangleVertices.push_back(-1.97f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-9.04f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);	
	triangleLight.push_back(0.5f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-1.97f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-9.04f);
	triangleVertices.push_back(-0.93f);triangleVertices.push_back(0.55f);triangleVertices.push_back(-8.51f);
	triangleVertices.push_back(-0.4f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-9.55f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);	
	triangleLight.push_back(0.5f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);
		
	//ceiling
	triangleVertices.push_back(2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-10.5f);
	triangleVertices.push_back(2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-5.0f);
	
	triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);
	triangleLight.push_back(0.5f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
	
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-10.5f);
	triangleVertices.push_back(2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-10.5f);
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-5.0f);	
	
	triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);	
	triangleLight.push_back(0.5f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
	
	//green wall
	triangleVertices.push_back(2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-10.5f);	
	triangleVertices.push_back(2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-10.5f);	
	
	triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);
	triangleLight.push_back(0.2f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
	
	triangleVertices.push_back(2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-10.5f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
	triangleLight.push_back(0.2f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
	
	
	//red wall
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-10.5f);	
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-10.5f);
	
	triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);
	triangleLight.push_back(0.2f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
	
	
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-5.0f);		
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(2.75f);triangleVertices.push_back(-10.5f);	
	
	triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);
	triangleLight.push_back(0.2f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
	
	
	//floor
	triangleVertices.push_back(2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-10.5f);	
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-10.5f);	
	
	triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);	
	triangleLight.push_back(0.2f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
		
	
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-5.0f);		
	triangleVertices.push_back(-2.75f);triangleVertices.push_back(-2.75f);triangleVertices.push_back(-10.5f);	
	
	triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);triangleColors.push_back(1.0f);
	triangleLight.push_back(0.2f);triangleLight.push_back(0.5f);triangleLight.push_back(0);triangleLight.push_back(0);
	
	
	 //Copy vertex/color information into uniform arrays
     glUseProgram(shader.program);
     
      GLint loc0 = glGetUniformLocation(shader.program, "light");
      if(loc0 != -1)	
		glUniform1fv(loc0, light.size(), light.data());
     
      GLint loc1 = glGetUniformLocation(shader.program, "planeVert");
      if(loc1 != -1)
		glUniform1fv(loc1, planeVertices.size(), planeVertices.data()); 
		
      GLint loc2 = glGetUniformLocation(shader.program, "planeColor");
      if(loc2 != -1)	
		glUniform1fv(loc2, planeColors.size(), planeColors.data());
		
	  GLint loc3 = glGetUniformLocation(shader.program, "triangleVert");
      if(loc3 != -1)	
		glUniform1fv(loc3, triangleVertices.size(), triangleVertices.data());
		
	  GLint loc4 = glGetUniformLocation(shader.program, "triangleColor");
      if(loc4 != -1)	
		glUniform1fv(loc4, triangleColors.size(), triangleColors.data());
		
	  GLint loc5 = glGetUniformLocation(shader.program, "sphereVert");
      if(loc5 != -1)	
		glUniform1fv(loc5, sphereVertices.size(), sphereVertices.data());
		
	  GLint loc6 = glGetUniformLocation(shader.program, "sphereColor");
      if(loc6 != -1)	
		glUniform1fv(loc6, sphereColors.size(), sphereColors.data());
		
	  GLint loc7 = glGetUniformLocation(shader.program, "planeLight");
      if(loc7 != -1)	
		glUniform1fv(loc7, planeLight.size(), planeLight.data());	
		
	  GLint loc8 = glGetUniformLocation(shader.program, "sphereLight");
      if(loc8 != -1)	
		glUniform1fv(loc8, sphereLight.size(), sphereLight.data());	
		
	  GLint loc9 = glGetUniformLocation(shader.program, "triangleLight");
      if(loc9 != -1)	
		glUniform1fv(loc9, triangleLight.size(), triangleLight.data());	
}


void scene2Vertices(MyShader shader)
{
	//clear values when switching scenes
	light.clear();
	
	planeVertices.clear();
	planeColors.clear();
	planeLight.clear();
	
	sphereVertices.clear();
	sphereColors.clear();
	sphereLight.clear();
	
	triangleVertices.clear();
	triangleColors.clear();
	triangleLight.clear();
	
	//light info
	light.push_back(4.0f);light.push_back(6.0f);light.push_back(-1.0f);
	
	//plane info
	//floor
	planeVertices.push_back(0.0f);planeVertices.push_back(1.0f);planeVertices.push_back(0.0f);
	planeVertices.push_back(0.0f);	planeVertices.push_back(-1.0f);planeVertices.push_back(0.0f);	
	
	planeColors.push_back(1.0f);planeColors.push_back(1.0f);planeColors.push_back(1.0f);
	planeLight.push_back(0.5f);planeLight.push_back(0.5f);planeLight.push_back(0);planeLight.push_back(0);
	
	//back wall
	planeVertices.push_back(0.0f);planeVertices.push_back(0.0f);planeVertices.push_back(1.0f);
	planeVertices.push_back(0.0f);	planeVertices.push_back(0.0f);planeVertices.push_back(-12.0f);	
		
	planeColors.push_back(1.0f);planeColors.push_back(0.5f);planeColors.push_back(0.0f);
	planeLight.push_back(0.5f);planeLight.push_back(0.5f);planeLight.push_back(0);planeLight.push_back(0);
	
	//yellow sphere
	sphereVertices.push_back(1.0f);sphereVertices.push_back(-0.5f);sphereVertices.push_back(-3.5f);
	sphereVertices.push_back(0.5f);
	
	sphereColors.push_back(0.8f);sphereColors.push_back(0.8f);sphereColors.push_back(0.0f);
	sphereLight.push_back(0.3f);sphereLight.push_back(0.5f);sphereLight.push_back(0.5f);sphereLight.push_back(30);
	
	//grey sphere
	sphereVertices.push_back(0.0f);sphereVertices.push_back(1.0f);sphereVertices.push_back(-5.0f);
	sphereVertices.push_back(0.4f);
	
	sphereColors.push_back(0.4f);sphereColors.push_back(0.4f);sphereColors.push_back(0.4f);
	sphereLight.push_back(0.3f);sphereLight.push_back(0.5f);sphereLight.push_back(1);sphereLight.push_back(150);
	
	//purple sphere
	sphereVertices.push_back(-0.8f);sphereVertices.push_back(-0.75f);sphereVertices.push_back(-4.0f);
	sphereVertices.push_back(0.25f);
	
	sphereColors.push_back(0.604f);sphereColors.push_back(0.102f);sphereColors.push_back(0.604f);
	sphereLight.push_back(0.3f);sphereLight.push_back(0.5f);sphereLight.push_back(1);sphereLight.push_back(150);
	
	//green cone
	//12 sets of vertices
	triangleVertices.push_back(0.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.8f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.693f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
	triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.693f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.4f);
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);		
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
	
	triangleVertices.push_back(0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.4f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(0.8f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.0f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);			
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(0.8f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);		
	triangleVertices.push_back(0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.6f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		

	triangleVertices.push_back(0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.6f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.307f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.307f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.2f);
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);			
	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.2f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(-0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.307f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.307f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(-0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.6f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);			

	triangleVertices.push_back(-0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-4.6f);
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(-0.8f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.0f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	

	triangleVertices.push_back(-0.8f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.0f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(-0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.4f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		

	triangleVertices.push_back(-0.6928f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.4f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(-0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.693f);	
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);	
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-0.4f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.693f);	
	triangleVertices.push_back(0.0f);triangleVertices.push_back(0.6f);triangleVertices.push_back(-5.0f);
	triangleVertices.push_back(0.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-5.8f);		
	
		triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);
			triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
																		
	//red triangle
	//20 sets of vertices
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.474f);		
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.149f);
	
		triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);		
					triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.474f);	
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.526f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-7.0f);		
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.149f);	
	triangleVertices.push_back(-2.894f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.0f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	

	triangleVertices.push_back(-2.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-2.894f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.851f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(-1.0f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.851f);				
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.526f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.474f);	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.526f);	
	triangleVertices.push_back(-1.106f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.0f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.149f);	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.474f);	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.149f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
			
	triangleVertices.push_back(-2.894f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.149f);	
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.474f);
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.851f);	
	triangleVertices.push_back(-2.894f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.526f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.526f);
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.851f);	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.851f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.474f);	
	triangleVertices.push_back(-1.106f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.149f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-6.149f);	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.149f);					
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.474f);
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	

	triangleVertices.push_back(-2.894f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.474f);
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.526f);
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.851f);		
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.526f);	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.851f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-1.276f);triangleVertices.push_back(-0.4472f);triangleVertices.push_back(-7.526f);	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.851f);	
	triangleVertices.push_back(-1.106f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.0f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.149f);		
	triangleVertices.push_back(-1.106f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(1.0f);triangleVertices.push_back(-7.0f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.474f);
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.149f);	
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(1.0f);triangleVertices.push_back(-7.0f);
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.526f);	
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-6.474f);				
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(1.0f);triangleVertices.push_back(-7.0f);
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.851f);	
	triangleVertices.push_back(-2.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.526f);	
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(1.0f);triangleVertices.push_back(-7.0f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
	
	triangleVertices.push_back(-1.106f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.0f);	
	triangleVertices.push_back(-1.724f);triangleVertices.push_back(0.4472f);triangleVertices.push_back(-7.851f);
	triangleVertices.push_back(-2.0f);triangleVertices.push_back(1.0f);triangleVertices.push_back(-7.0f);	
	
			triangleColors.push_back(1.0f);triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);	
						triangleLight.push_back(0.3f);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);						
																		
	 //Copy vertex/color information into uniform arrays
     glUseProgram(shader.program);
     
      GLint loc0 = glGetUniformLocation(shader.program, "light");
      if(loc0 != -1)	
		glUniform1fv(loc0, light.size(), light.data());
     
      GLint loc1 = glGetUniformLocation(shader.program, "planeVert");
      if(loc1 != -1)
		glUniform1fv(loc1, planeVertices.size(), planeVertices.data()); 
		
      GLint loc2 = glGetUniformLocation(shader.program, "planeColor");
      if(loc2 != -1)	
		glUniform1fv(loc2, planeColors.size(), planeColors.data());
		
	  GLint loc3 = glGetUniformLocation(shader.program, "triangleVert");
      if(loc3 != -1)	
		glUniform1fv(loc3, triangleVertices.size(), triangleVertices.data());
		
	  GLint loc4 = glGetUniformLocation(shader.program, "triangleColor");
      if(loc4 != -1)	
		glUniform1fv(loc4, triangleColors.size(), triangleColors.data());
		
	  GLint loc5 = glGetUniformLocation(shader.program, "sphereVert");
      if(loc5 != -1)	
		glUniform1fv(loc5, sphereVertices.size(), sphereVertices.data());
		
	  GLint loc6 = glGetUniformLocation(shader.program, "sphereColor");
      if(loc6 != -1)	
		glUniform1fv(loc6, sphereColors.size(), sphereColors.data());
		
	  GLint loc7 = glGetUniformLocation(shader.program, "planeLight");
      if(loc7 != -1)	
		glUniform1fv(loc7, planeLight.size(), planeLight.data());	
		
	  GLint loc8 = glGetUniformLocation(shader.program, "sphereLight");
      if(loc8 != -1)	
		glUniform1fv(loc8, sphereLight.size(), sphereLight.data());	
		
	  GLint loc9 = glGetUniformLocation(shader.program, "triangleLight");
      if(loc9 != -1)	
		glUniform1fv(loc9, triangleLight.size(), triangleLight.data());	
	
}

void push_tri(float X, float Y, float Z){
	triangleVertices.push_back(X+-0.4f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-5.55f);
	triangleVertices.push_back(X+-0.93f);triangleVertices.push_back(Y+0.55f);triangleVertices.push_back(Z+-4.51f);
	triangleVertices.push_back(X+0.11f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-3.98f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);
	triangleLight.push_back(0);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);		
	
	triangleVertices.push_back(X+0.11f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-3.98f);
	triangleVertices.push_back(X+-0.93f);triangleVertices.push_back(Y+0.55f);triangleVertices.push_back(Z+-4.51f);
	triangleVertices.push_back(X+-1.46f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-3.47f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);
    triangleLight.push_back(0);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);
	
	triangleVertices.push_back(X+-1.46f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-3.47f);
	triangleVertices.push_back(X+-0.93f);triangleVertices.push_back(Y+0.55f);triangleVertices.push_back(Z+-4.51f);
	triangleVertices.push_back(X+-1.97f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-5.04f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);	
	triangleLight.push_back(0);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);
	
	triangleVertices.push_back(X+-1.97f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-5.04f);
	triangleVertices.push_back(X+-0.93f);triangleVertices.push_back(Y+0.55f);triangleVertices.push_back(Z+-4.51f);
	triangleVertices.push_back(X+-0.4f);triangleVertices.push_back(Y+-2.75f);triangleVertices.push_back(Z+-5.55f);
	
	triangleColors.push_back(0.0f);triangleColors.push_back(0.0f);triangleColors.push_back(1.0f);
	triangleLight.push_back(0);triangleLight.push_back(0.5f);triangleLight.push_back(1);triangleLight.push_back(50);	
}


void scene3Vertices(MyShader shader)
{
	//clear values when switching scenes
	light.clear();
	
	planeVertices.clear();
	planeColors.clear();
	planeLight.clear();
	
	sphereVertices.clear();
	sphereColors.clear();
	sphereLight.clear();
	
	triangleVertices.clear();
	triangleColors.clear();
	triangleLight.clear();
	
	//light info
	light.push_back(0.0f);light.push_back(0.0f);light.push_back(-5.5f);
	
	//plane info
	//floor
	planeVertices.push_back(0.0f);planeVertices.push_back(1.0f);planeVertices.push_back(0.0f);
	planeVertices.push_back(0.0f);planeVertices.push_back(-2.0f);planeVertices.push_back(0.3f);	
	
	planeColors.push_back(0.6f);planeColors.push_back(0.2f);planeColors.push_back(1.0f);
	planeLight.push_back(0.35f);planeLight.push_back(1);planeLight.push_back(0);planeLight.push_back(0);
	
	//back wall
	planeVertices.push_back(0.0f);planeVertices.push_back(0.0f);planeVertices.push_back(1.0f);
	planeVertices.push_back(0.0f);	planeVertices.push_back(0.0f);planeVertices.push_back(-20.0f);	
		
	planeColors.push_back(0.2f);planeColors.push_back(0.0f);planeColors.push_back(0.4f);
	planeLight.push_back(0.2f);planeLight.push_back(0.5f);planeLight.push_back(0);planeLight.push_back(0);
	
	//yellow sphere
	sphereVertices.push_back(0.0f);sphereVertices.push_back(2.0f);sphereVertices.push_back(-8.5f);
	sphereVertices.push_back(1.5f);
	
	sphereColors.push_back(1.0f);sphereColors.push_back(0.5f);sphereColors.push_back(0.15f);
	sphereLight.push_back(1);sphereLight.push_back(1);sphereLight.push_back(0);sphereLight.push_back(0);
	
	//white sphere
	sphereVertices.push_back(1.7f);sphereVertices.push_back(1.5f);sphereVertices.push_back(-6.5f);
	sphereVertices.push_back(0.3f);
	
	sphereColors.push_back(1.0f);sphereColors.push_back(1.0f);sphereColors.push_back(1.0f);
	sphereLight.push_back(0.5f);sphereLight.push_back(0.5f);sphereLight.push_back(0);sphereLight.push_back(0);
	
	push_tri(-1,0,0);
	push_tri(-0.5f,0,0);
	push_tri(0,0,0);
	push_tri(0.5f,0,0);
	push_tri(1,0,0);
	push_tri(1.5f,0,0);
	push_tri(2,0,0);
	push_tri(2.5f,0,0);

	
	//Copy vertex/color information into uniform arrays
     glUseProgram(shader.program);
     
      GLint loc0 = glGetUniformLocation(shader.program, "light");
      if(loc0 != -1)	
		glUniform1fv(loc0, light.size(), light.data());
     
      GLint loc1 = glGetUniformLocation(shader.program, "planeVert");
      if(loc1 != -1)
		glUniform1fv(loc1, planeVertices.size(), planeVertices.data()); 
		
      GLint loc2 = glGetUniformLocation(shader.program, "planeColor");
      if(loc2 != -1)	
		glUniform1fv(loc2, planeColors.size(), planeColors.data());
		
	  GLint loc3 = glGetUniformLocation(shader.program, "triangleVert");
      if(loc3 != -1)	
		glUniform1fv(loc3, triangleVertices.size(), triangleVertices.data());
		
	  GLint loc4 = glGetUniformLocation(shader.program, "triangleColor");
	  if(loc4 != -1)	
		glUniform1fv(loc4, triangleColors.size(), triangleColors.data());
		
	  GLint loc5 = glGetUniformLocation(shader.program, "sphereVert");
      if(loc5 != -1)	
		glUniform1fv(loc5, sphereVertices.size(), sphereVertices.data());
		
	  GLint loc6 = glGetUniformLocation(shader.program, "sphereColor");
      if(loc6 != -1)	
		glUniform1fv(loc6, sphereColors.size(), sphereColors.data());
		
	  GLint loc7 = glGetUniformLocation(shader.program, "planeLight");
      if(loc7 != -1)	
		glUniform1fv(loc7, planeLight.size(), planeLight.data());	
		
	  GLint loc8 = glGetUniformLocation(shader.program, "sphereLight");
      if(loc8 != -1)	
		glUniform1fv(loc8, sphereLight.size(), sphereLight.data());	
		
	  GLint loc9 = glGetUniformLocation(shader.program, "triangleLight");
      if(loc9 != -1)	
		glUniform1fv(loc9, triangleLight.size(), triangleLight.data());	
}

MyShader shader;
MyGeometry geometry;

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
		
	//Go forward	
	else if(key == GLFW_KEY_W)
	{
		z = z - 0.5f;
		
		glUseProgram(shader.program);
		GLint loc = glGetUniformLocation(shader.program, "z");
		if(loc != -1)
			glUniform1f(loc, z);
			
		RenderScene(&geometry, &shader);
	}
	
	//Go backward
	else if(key == GLFW_KEY_S)
	{
		z = z + 0.5f;
		
		glUseProgram(shader.program);
		GLint loc = glGetUniformLocation(shader.program, "z");
		
		if(loc != -1)
			glUniform1f(loc, z);
			
		RenderScene(&geometry, &shader);
	}
	
	//Go left
	else if(key == GLFW_KEY_A)
	{
		x = x - 0.5f;
		
		glUseProgram(shader.program);
		GLint loc = glGetUniformLocation(shader.program, "x");
		
		if(loc != -1)
			glUniform1f(loc, x);
			
		RenderScene(&geometry, &shader);
	}
	
	//Go right
	else if(key == GLFW_KEY_D)
	{
		x = x + 0.5f;
		
		glUseProgram(shader.program);
		GLint loc = glGetUniformLocation(shader.program, "x");
		
		if(loc != -1)
			glUniform1f(loc, x);
			
		RenderScene(&geometry, &shader);
	}
	
	//Go up
	else if(key == GLFW_KEY_O)
	{
		y = y + 0.5f;
		
		glUseProgram(shader.program);
		GLint loc = glGetUniformLocation(shader.program, "y");
		
		if(loc != -1)
			glUniform1f(loc, y);
			
		RenderScene(&geometry, &shader);
	}
	
	//Go down
	else if(key == GLFW_KEY_P)
	{
		y = y - 0.5f;
		
		glUseProgram(shader.program);
		GLint loc = glGetUniformLocation(shader.program, "y");
		
		if(loc != -1)
			glUniform1f(loc, y);
			
		RenderScene(&geometry, &shader);
	}	
	
	//Choose scene 1
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{	
		x = 0.f;
		y = 0.f;
		z = 0.f;

		//Set array sizes in fragment shader
	  glUseProgram(shader.program);
	  
	  GLint loc1 = glGetUniformLocation(shader.program, "pV");
      if(loc1 != -1)
		glUniform1i(loc1, 6); 
		
	  GLint loc2 = glGetUniformLocation(shader.program, "tV");
      if(loc2 != -1)	
		glUniform1i(loc2, 108);
		
	  GLint loc3 = glGetUniformLocation(shader.program, "sV");
      if(loc3 != -1)	
		glUniform1i(loc3, 4);
		
	  GLint loc4 = glGetUniformLocation(shader.program, "x");
		if(loc4 != -1)
			glUniform1f(loc4, x);
			
	  GLint loc5 = glGetUniformLocation(shader.program, "z");
		if(loc5 != -1)
			glUniform1f(loc5, z);	
			
	  GLint loc6 = glGetUniformLocation(shader.program, "y");
		if(loc6 != -1)
			glUniform1f(loc6, y);	
			
		scene1Vertices(shader);
		RenderScene(&geometry, &shader);
	}
	
	//Choose scene 2
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
			
		//Set array sizes in fragment shader	
      glUseProgram(shader.program);
      
      GLint loc1 = glGetUniformLocation(shader.program, "pV");
      if(loc1 != -1)
		glUniform1i(loc1, 12); 
		
	  GLint loc2 = glGetUniformLocation(shader.program, "tV");
      if(loc2 != -1)	
		glUniform1i(loc2, 288);
		
	  GLint loc3 = glGetUniformLocation(shader.program, "sV");
      if(loc3 != -1)	
		glUniform1i(loc3, 12);
		
	  GLint loc4 = glGetUniformLocation(shader.program, "x");
		if(loc4 != -1)
			glUniform1f(loc4, x);
			
	  GLint loc5 = glGetUniformLocation(shader.program, "z");
		if(loc5 != -1)
			glUniform1f(loc5, z);	

	  GLint loc6 = glGetUniformLocation(shader.program, "y");
		if(loc6 != -1)
			glUniform1f(loc6, y);	
			
		scene2Vertices(shader);
		RenderScene(&geometry, &shader);
	}
	
	//Choose scene 3
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		x = -0.1f;
		y = 0.f;
		z = -1.0f;
			
		//Set array sizes in fragment shader	
      glUseProgram(shader.program);
      
      GLint loc1 = glGetUniformLocation(shader.program, "pV");
      if(loc1 != -1)
		glUniform1i(loc1, 12); 
		
	  GLint loc2 = glGetUniformLocation(shader.program, "tV");
      if(loc2 != -1)	
		glUniform1i(loc2, 360);
		
	  GLint loc3 = glGetUniformLocation(shader.program, "sV");
      if(loc3 != -1)	
		glUniform1i(loc3, 8);
		
	  GLint loc4 = glGetUniformLocation(shader.program, "x");
		if(loc4 != -1)
			glUniform1f(loc4, x);
			
	  GLint loc5 = glGetUniformLocation(shader.program, "z");
		if(loc5 != -1)
			glUniform1f(loc5, z);	
			
	  GLint loc6 = glGetUniformLocation(shader.program, "y");
		if(loc6 != -1)
			glUniform1f(loc6, y);	
			
		scene3Vertices(shader);
		RenderScene(&geometry, &shader);
	}		
		
		
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(768, 768, "CPSC 453 OpenGL Boilerplate", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}
	
	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwMakeContextCurrent(window);

	//Intialize GLAD
	#ifndef LAB_LINUX
	if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}
	#endif

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	// call function to load and compile shader programs
	if (!InitializeShaders(&shader)) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}
	
		// call function to create and fill buffers with geometry data
	if (!InitializeGeometry(&geometry))
		cout << "Program failed to intialize geometry!" << endl;
		
	//scene1Vertices(shader);
	//RenderScene(&geometry, &shader);

	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// clean up allocated resources before exit
	DestroyGeometry(&geometry);
	DestroyShaders(&shader);
	glfwDestroyWindow(window);
	glfwTerminate();

	//cout << "Goodbye!" << endl;
	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}

	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}
