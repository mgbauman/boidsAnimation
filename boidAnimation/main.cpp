// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cstdlib>

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/vector_angle.hpp"

// specify that we want the OpenGL core profile before including GLFW headers
//#include "glad/glad.h"
#ifndef LAB_LINUX
#include <glad/glad.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLFW/glfw3.h>

#include "camera.h"
#include "Boid.h"
#include "Skybox.h"

#define PI 3.14159265359

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors(string location);
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
void randomlyPlaceBoids();
void initGameFromFile();

int enclosureSize = 50;

// Constraints for the positions of the boids
int XMAX = enclosureSize;
int XMIN = -enclosureSize;

int YMAX = enclosureSize;
int YMIN = -enclosureSize;

int ZMAX = enclosureSize;
int ZMIN = -enclosureSize;

vec3 goalPos = vec3(0,0,0);
bool goalMode = false;

vec2 mousePos;
bool leftmousePressed = false;
bool rightmousePressed = false;
bool middlemousePressed = false;

// Keyboard input flags
bool up = false;
bool right = false;
bool left = false;
bool down = false;

Camera* activeCamera;
Skybox * skybox;

float scaleFactor = 0.25f;
bool animate = true;

int numOfBoids = 100;
float deltaT = 0.008;

//vec3 obstaclePos = vec3(50, 25, 0);

vector<Boid*> boids;

GLFWwindow* window = 0;

Camera cam = Camera(vec3(0, 0, -1), vec3(0, 0, 200));
mat4 winRatio = mat4(1.f);


// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        cout << "Reloading Parameters..." << endl;
        
        for (unsigned int i = 0; i < boids.size(); i++) {
            delete(boids[i]);
        }
        boids.clear();

        initGameFromFile();

        for (int i = 0; i < numOfBoids; i++) {
           boids.push_back(new Boid());
           boids[i]->updateParams();
           boids[i]->velocity = vec3(0.f, 0, 0.f);
        }

        randomlyPlaceBoids();
        cam = Camera(vec3(0, 0, -1), vec3(0, 0, 200));
        scaleFactor = 0.25f;
        goalPos = vec3(0,0,0);
        goalMode = false;
    }

	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		goalMode = !goalMode;
	}

    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		::up = true;
    }
    
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
		::up = false;
	}
    
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		::right = true;
    } 
    
	if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		::right = false;
    } 
    
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		::left = true;
		
    }
    
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		::left = false;
		
    }
    
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		::down = true;
    }
    
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
		::down = false;
    }
    
    if(::up == true && goalMode == true){
		if(goalPos.y < YMAX){
			goalPos.y += 1.f;
		}
	}
	
	if(::right == true && goalMode == true){
		if(goalPos.x < XMAX){
			goalPos.x += 1.f;
		}
	}
	
	if(::left == true && goalMode == true){
		if(goalPos.x > XMIN){
			goalPos.x -= 1.f;
		}
	}
	
	if(::down == true && goalMode == true){
		if(goalPos.y > YMIN){
			goalPos.y -= 1.f;
		}
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if ((action == GLFW_PRESS) || (action == GLFW_RELEASE)) {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            leftmousePressed = !leftmousePressed;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            rightmousePressed = !rightmousePressed;
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
            middlemousePressed = !middlemousePressed;
    }
}

void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    vec2 newPos = vec2(xpos / (double)vp[2], -ypos / (double)vp[3])*2.f - vec2(1.f);

    vec2 diff = newPos - mousePos;
    if (leftmousePressed) {
        activeCamera->trackballRight(-diff.x);
        activeCamera->trackballUp(-diff.y);
    }
    else if (rightmousePressed) {
		float zoomBase = (diff.y > 0) ? 1.f / 2.f : 2.f;
		activeCamera->zoom(pow(zoomBase, abs(diff.y)));
    }
    else if (middlemousePressed) {
       // activeCamera->panLeftRight(-diff.x*50);
       // activeCamera->panUpDown(-diff.y * 50);
    }

    mousePos = newPos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //scaleFactor += yoffset*(scaleFactor / (5));
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    glViewport(0, 0, width, height);

    int min = min(width, height);

    float minDim = float(min);

    winRatio[0][0] = minDim / float(width);
    winRatio[1][1] = minDim / float(height);
}

//==========================================================================
// TUTORIAL STUFF


//vec2 and vec3 are part of the glm math library. 
//Include in your own project by putting the glm directory in your project, 
//and including glm/glm.hpp as I have at the top of the file.
//"using namespace glm;" will allow you to avoid writing everyting as glm::vec2

struct VertexBuffers {
    enum { VERTICES = 0, NORMALS, INDICES, COUNT };

    GLuint id[COUNT];
};

//Describe the setup of the Vertex Array Object
bool initVAO(GLuint vao, const VertexBuffers& vbo)
{
    glBindVertexArray(vao);		//Set the active Vertex Array

    glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::VERTICES]);		//Set the active Vertex Buffer
    glVertexAttribPointer(
        0,				//Attribute
        3,				//Size # Components
        GL_FLOAT,	//Type
        GL_FALSE, 	//Normalized?
        sizeof(vec3),	//Stride
        (void*)0			//Offset
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::NORMALS]);
    glVertexAttribPointer(
        1,				//Attribute
        3,				//Size # Components
        GL_FLOAT,	//Type
        GL_FALSE, 	//Normalized?
        sizeof(vec3),	//Stride
        (void*)0			//Offset
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[VertexBuffers::INDICES]);

    return !CheckGLErrors("initVAO");		//Check for errors in initialize
}


//Loads buffers with data
bool loadBuffer(const VertexBuffers& vbo,
    const vector<vec3>& points,
    const vector<vec3> normals,
    const vector<unsigned int>& indices)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::VERTICES]);
    glBufferData(
        GL_ARRAY_BUFFER,				//Which buffer you're loading too
        sizeof(vec3)*points.size(),		//Size of data in array (in bytes)
        &points[0],						//Start of array (&points[0] will give you pointer to start of vector)
        GL_STATIC_DRAW					//GL_DYNAMIC_DRAW if you're changing the data often
                                        //GL_STATIC_DRAW if you're changing seldomly
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::NORMALS]);
    glBufferData(
        GL_ARRAY_BUFFER,				//Which buffer you're loading too
        sizeof(vec3)*normals.size(),	//Size of data in array (in bytes)
        &normals[0],					//Start of array (&points[0] will give you pointer to start of vector)
        GL_STATIC_DRAW					//GL_DYNAMIC_DRAW if you're changing the data often
                                        //GL_STATIC_DRAW if you're changing seldomly
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[VertexBuffers::INDICES]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(unsigned int)*indices.size(),
        &indices[0],
        GL_STATIC_DRAW
    );

    return !CheckGLErrors("loadBuffer");
}

//Compile and link shaders, storing the program ID in shader array
GLuint initShader(string vertexName, string fragmentName)
{
    string vertexSource = LoadSource(vertexName);		//Put vertex file text into string
    string fragmentSource = LoadSource(fragmentName);		//Put fragment file text into string

    GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    return LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array
}

//Initialization
void initGL()
{
    glEnable(GL_DEPTH_TEST);
    glPointSize(15);
    glDepthFunc(GL_LEQUAL);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glClearColor(0.f, 0.f, 0.f, 0.f);		//Color to clear the screen with (R, G, B, Alpha)
}

bool loadUniforms(GLuint program, mat4 perspective, mat4 modelview)
{
    glUseProgram(program);

    glUniformMatrix4fv(glGetUniformLocation(program, "modelviewMatrix"),
        1,
        false,
        &modelview[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(program, "perspectiveMatrix"),
        1,
        false,
        &perspective[0][0]);

    return !CheckGLErrors("loadUniforms");
}

//Draws buffers to screen
void render(GLuint vao, int startElement, int numElements)
{
    glBindVertexArray(vao);		//Use the LINES vertex array

    glDrawElements(
        GL_TRIANGLES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
        numElements,		//How many indices
        GL_UNSIGNED_INT,	//Type
        (void*)0			//Offset
    );

    //DO NOT FORGET TO UNBIND!
    glBindVertexArray(0);
    CheckGLErrors("render");
}

//Draws buffers to screen
void renderPoints(GLuint vao, int startElement, int numElements)
{
    glBindVertexArray(vao);

    glDrawElements(
        GL_POINTS,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
        numElements,		//How many indices
        GL_UNSIGNED_INT,	//Type
        (void*)0			//Offset
    );

    //DO NOT FORGET TO UNBIND!
    glBindVertexArray(0);
    CheckGLErrors("renderPoints");
}

//Draws buffers to screen
void renderLines(GLuint vao, int startElement, int numElements)
{
    glBindVertexArray(vao);

    glDrawElements(
        GL_LINES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
        numElements,		//How many indices
        GL_UNSIGNED_INT,	//Type
        (void*)0			//Offset
    );

    //DO NOT FORGET TO UNBIND!
    glBindVertexArray(0);
    CheckGLErrors("renderLines");
}



//Draws buffers to screen
void renderLineStrip(GLuint vao, int startElement, int numElements)
{
    glBindVertexArray(vao);

    glDrawElements(
        GL_LINE_STRIP,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
        numElements,		//How many indices
        GL_UNSIGNED_INT,	//Type
        (void*)0			//Offset
    );

    //DO NOT FORGET TO UNBIND!
    glBindVertexArray(0);
    CheckGLErrors("renderLines");
}

void initSkybox() {

    std::string skybox_dir = "mp_whirlpool/";
    std::string filenames[6] = {
        skybox_dir + "whirlpool_rt.tga",
        skybox_dir + "whirlpool_lf.tga",
        skybox_dir + "whirlpool_up3.tga",
        skybox_dir + "whirlpool_dn2.tga",
        skybox_dir + "whirlpool_bk.tga",
        skybox_dir + "whirlpool_ft.tga",
    };

    skybox = new Skybox(filenames);
}

void drawSkybox(GLuint program, const Skybox* sb, glm::mat4 perspectiveMatrix)
{
    glDepthMask(GL_FALSE);
    //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glUseProgram(program);
    // Set object-specific VAO
    glBindVertexArray(sb->vao);

    glm::mat4 view = glm::mat4(1.f); // Convert to 3x3 to remove translation components

    glUniformMatrix4fv(glGetUniformLocation(program, "view"),
        1,
        false,
        &view[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(program, "projection"),
        1,
        false,
        &perspectiveMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sb->tex_id);

    //   GLuint uniformLocation = glGetUniformLocation(shader[SHADER::SKYBOX], skybox);
    //   glUniform1i(uniformLocation, 0);
    CheckGLErrors("loadUniforms in skybox");

    glDrawArrays(GL_TRIANGLES, 0, 36);

    CheckGLErrors("drawSkybox");
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}


void generatePSphere(vector<vec3>& positions, vector<vec3>& normals,vector<unsigned int>& indices,
					float r, vec3 center,
					int uDivisions, int vDivisions, bool red=false)
{
	// udivisions will be theta
	// vdivisions will be phi	
	float uStep = 1.f/(float)(uDivisions-1);
	float vStep = 1.f/(float)(vDivisions-1);

	float u = 0.f;
	
    // Iterate through phi and theta
    for (double phi = 0.; phi < uDivisions; phi ++) // Azimuth [0, 2PI]
    {
		float v = 0.f;
        for (double theta = 0.; theta < vDivisions; theta++) // Elevation [0, PI]
        {
            vec3 point;
            point.x = r * cos(v*2*PI) * sin(u*PI) + center.x;
            point.y = r * sin(v*2*PI) * sin(u*PI) + center.y;
            point.z = r               * cos(u*PI) + center.z;
            
            vec3 normal = normalize(point - center);
            
            positions.push_back(point);
            if(red != true){
				normals.push_back(normal);
			} else {
				normals.push_back(vec3(normal.x,0.2f,0.2f));
			}
            v+=vStep;
        }
        u+=uStep;
    }
   
    for(int i=0; i<uDivisions-1; i++)
	{
		for(int j=0; j<vDivisions -1; j++)
		{
			unsigned int p00 = i*vDivisions+j;
			unsigned int p01 = i*vDivisions+j+1;
			unsigned int p10 = (i+1)*vDivisions + j;
			unsigned int p11 = (i+1)*vDivisions + j + 1;

			indices.push_back(p00);
			indices.push_back(p10);
			indices.push_back(p01);

			indices.push_back(p01);
			indices.push_back(p10);
			indices.push_back(p11);
		}
	}
    
}

void generateBox(vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, float sideLength) {

    vec3 bottomLeftFront = vec3(-sideLength, -sideLength / 2, sideLength);
    vec3 bottomLeftBack = vec3(-sideLength, -sideLength / 2, -sideLength);
    vec3 topLeftBack = vec3(-sideLength, sideLength / 2, -sideLength);
    vec3 topLeftFront = vec3(-sideLength, sideLength / 2, sideLength);

    vec3 bottomRightFront = vec3(sideLength, -sideLength / 2, sideLength);
    vec3 bottomRightBack = vec3(sideLength, -sideLength / 2, -sideLength);
    vec3 topRightBack = vec3(sideLength, sideLength / 2, -sideLength);
    vec3 topRightFront = vec3(sideLength, sideLength / 2, sideLength);

    vertices->push_back(bottomLeftFront);
    vertices->push_back(bottomLeftBack);
    vertices->push_back(topLeftBack);
    vertices->push_back(topLeftFront);

    vertices->push_back(bottomRightFront);
    vertices->push_back(bottomRightBack);
    vertices->push_back(topRightBack);
    vertices->push_back(topRightFront);

    //vec3 center = vec3(0, 0, 0);

    //normals->push_back(bottomLeftFront - center);
    //normals->push_back(bottomLeftBack - center);
    //normals->push_back(topLeftBack - center);
    //normals->push_back(topLeftFront - center);

    //normals->push_back(bottomRightFront - center);
    //normals->push_back(bottomRightBack - center);
    //normals->push_back(topRightBack - center);
    //normals->push_back(topRightFront-center);

    for (int i = 0; i<1; i++) {
        normals->push_back(vec3(1.f, 1.f, 0.f));
    }
    for (int i = 0; i<2; i++) {
        normals->push_back(vec3(0, 0, 1));
    }for (int i = 0; i<2; i++) {
        normals->push_back(vec3(1.f, 1.f, 0.f));
    }
    for (int i = 0; i<2; i++) {
        normals->push_back(vec3(0, 0, 1));
    }

    for (int i = 0; i<1; i++) {
        normals->push_back(vec3(1.f, 1.f, 0.f));
    }

    //TOP
    indices->push_back(3); indices->push_back(2); indices->push_back(6);
    indices->push_back(6); indices->push_back(7); indices->push_back(3);

    //RIGHT
    indices->push_back(5);indices->push_back(6);indices->push_back(7);
    indices->push_back(4);indices->push_back(5);indices->push_back(7);

    //BACK
    indices->push_back(1); indices->push_back(2); indices->push_back(6);
    indices->push_back(1); indices->push_back(5); indices->push_back(6);

    //LEFT
    indices->push_back(1);indices->push_back(2);indices->push_back(3);
    indices->push_back(0);indices->push_back(1);indices->push_back(3);

    //FRONT
    indices->push_back(0);indices->push_back(4);indices->push_back(7);
    indices->push_back(0);indices->push_back(7);indices->push_back(3);

    //BOTTOM
    indices->push_back(0); indices->push_back(1); indices->push_back(5);
    indices->push_back(0); indices->push_back(5); indices->push_back(4);
}


void generateTower(vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, float sideLength) {

    vec3 bottomLeftFront = vec3(-sideLength, -sideLength, sideLength);
    vec3 bottomLeftBack = vec3(-sideLength, -sideLength, -sideLength);
    vec3 topLeftBack = vec3(-sideLength, sideLength, -sideLength);
    vec3 topLeftFront = vec3(-sideLength, sideLength, sideLength);

    vec3 bottomRightFront = vec3(sideLength, -sideLength, sideLength);
    vec3 bottomRightBack = vec3(sideLength, -sideLength, -sideLength);
    vec3 topRightBack = vec3(sideLength, sideLength, -sideLength);
    vec3 topRightFront = vec3(sideLength, sideLength, sideLength);

    vertices->push_back(bottomLeftFront);
    vertices->push_back(bottomLeftBack);
    vertices->push_back(topLeftBack);
    vertices->push_back(topLeftFront);

    vertices->push_back(bottomRightFront);
    vertices->push_back(bottomRightBack);
    vertices->push_back(topRightBack);
    vertices->push_back(topRightFront);

    for (int i = 0; i < 8; i++) {
        normals->push_back(vec3(.543f, 0.270f, 0.074f));
    }

    //TOP
    indices->push_back(3); indices->push_back(2); indices->push_back(6);
    indices->push_back(6); indices->push_back(7); indices->push_back(3);

    //RIGHT
    indices->push_back(5); indices->push_back(6); indices->push_back(7);
    indices->push_back(4); indices->push_back(5); indices->push_back(7);

    //BACK
    indices->push_back(1); indices->push_back(2); indices->push_back(6);
    indices->push_back(1); indices->push_back(5); indices->push_back(6);

    //LEFT
    indices->push_back(1); indices->push_back(2); indices->push_back(3);
    indices->push_back(0); indices->push_back(1); indices->push_back(3);

    //FRONT
    indices->push_back(0); indices->push_back(4); indices->push_back(7);
    indices->push_back(0); indices->push_back(7); indices->push_back(3);

    //BOTTOM
    indices->push_back(0); indices->push_back(1); indices->push_back(5);
    indices->push_back(0); indices->push_back(5); indices->push_back(4);
}


void generateSquare(vector<vec3>& vertices, vector<vec3>& normals,
    vector<unsigned int>& indices, float width)
{
    vertices.push_back(vec3(-width*0.5f, -22.f, -width*0.5f));
    vertices.push_back(vec3(width*0.5f, -22.f, -width*0.5f));
    vertices.push_back(vec3(width*0.5f, -22.f, width*0.5f));
    vertices.push_back(vec3(-width*0.5f, -22.f, width*0.5f));


    normals.push_back(vec3(0.8f, 0.8f, 0.8f));
    normals.push_back(vec3(0.8f, 0.8f, 0.8f));
    normals.push_back(vec3(0.8f, 0.8f, 0.8f));
    normals.push_back(vec3(0.8f, 0.8f, 0.8f));

    //First triangle
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    //Second triangle
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);
}

void generateBoid(vector<vec3>& vertices, vector<vec3>& normals,
    vector<unsigned int>& indices, float width)
{
    vertices.push_back(vec3(-width, -width, -width));
    vertices.push_back(vec3(width, -width, -width));
    vertices.push_back(vec3(width, width, -width));
    vertices.push_back(vec3(-width, width, -width));
    vertices.push_back(vec3(0.f, 0.f, width));

   /* normals.push_back(vec3(0.7f, 0.7f, 0.7f));
    normals.push_back(vec3(0.7f, 0.7f, 0.7f));
    normals.push_back(vec3(0.7f, 0.7f, 0.7f));
    normals.push_back(vec3(0.7f, 0.7f, 0.7f));*/


    //rgb(202, 242, 112)
    normals.push_back(vec3(0.789f, 0.945f, 0.438f));
    normals.push_back(vec3(0.789f, 0.945f, 0.438f));
    normals.push_back(vec3(0.789f, 0.945f, 0.438f));
    normals.push_back(vec3(0.789f, 0.945f, 0.438f));

    normals.push_back(vec3(0.1f, 0.1f, 0.1f));

    //First triangle
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    //Second triangle
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    //First triangle
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(4);
    //Second triangle
    indices.push_back(4);
    indices.push_back(3);
    indices.push_back(0);

    //First triangle
    indices.push_back(4);
    indices.push_back(3);
    indices.push_back(2);
    //Second triangle
    indices.push_back(2);
    indices.push_back(4);
    indices.push_back(1);

}

GLFWwindow* createGLFWWindow()
{
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
        return NULL;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(512, 512, "Assignment 4", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return NULL;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    return window;
}

float magnitude(vec3 v) {
    return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}


vector<Boid *> findAllBoidsInRadius(Boid * currentBoid, int radius) {
    vector<Boid *> nearbyBoids;

    for (Boid * boid : boids) {
       if ( glm::distance(currentBoid->position, boid->position) < radius) {
            nearbyBoids.push_back(boid);
        }
    }
    return nearbyBoids;
}

// Semi implicit Euler
vec3 calculateNewVelocity(vec3 initialVelocity, vec3 acceleration) {
    return initialVelocity + acceleration * deltaT;
}

// Semi implicit Euler
vec3 calculateNewPosition(vec3 initialPosition, vec3 velocity) {
    return initialPosition + velocity * deltaT;
}

//Keep the boids confined to a certain area
void constrainPosition(Boid * boid) {
	vec3 pullFactor = -boid->position;

    if (boid->position.x > XMAX) {
        boid->accelerationVector.x += pullFactor.x;
    } 
    else if (boid->position.x < XMIN) {
        boid->accelerationVector.x += pullFactor.x;
    }
    if (boid->position.y > YMAX) {
        boid->accelerationVector.y += pullFactor.y;
    }
    else if (boid->position.y < YMIN) {
        boid->accelerationVector.y += pullFactor.y;
    }
    if (boid->position.z > ZMAX) {
        boid->accelerationVector.z += pullFactor.z;
    }
    else if (boid->position.z < ZMIN) {
        boid->accelerationVector.z += pullFactor.z;
    }
  
}

void placeObstacles(vector<vec3>& obstacles){
	obstacles.push_back(vec3(100,50,0));
	obstacles.push_back(vec3(-100,50,0));
	obstacles.push_back(vec3(0,50,-100));//vec3(0,50,0) looked good too (pre goal)
	obstacles.push_back(vec3(50,-75,0));
}

void randomlyPlaceBoids() {
    for (auto boid : boids) {
//        boid->position = vec3((((rand() % (2*enclosureSize)) / 2) - (enclosureSize)), (((rand() % (2*enclosureSize)) / 2) - (enclosureSize)), (((rand() % (2*enclosureSize)) / 2) - (enclosureSize)));
		boid->position = vec3(((rand() % (2*enclosureSize)) - (enclosureSize)),((rand() % (2*enclosureSize)) - (enclosureSize)),((rand() % (2*enclosureSize)) - (enclosureSize)));

  
  /*      
        if ((0 < boid->position.x)&& (boid->position.x < 15))
            boid->position.x += 15;
            
        if ((0 < boid->position.y)&&(boid->position.y < 15))
            boid->position.y += 15;
            
        if ((0 < boid->position.z)&&(boid->position.z < 15))
            boid->position.z += 15;

        if ((0 > boid->position.x)&&(boid->position.x > -15))
            boid->position.x -= 15;
        
        if ((0 > boid->position.y) && ( boid->position.y > -15))
            boid->position.y -= 15;
            
        if ((0 < boid->position.z) && (boid->position.z > -15))
            boid->position.z -= 15;

*/

      //  boid->velocity = vec3(0.f, 0, 0.f);
		boid->velocity = vec3((rand()%(int)boid->maxSpeed),(rand()%(int)boid->maxSpeed),(rand()%(int)boid->maxSpeed));
    }
}

void initGameFromFile() {
    vector<float> parameters;

    char line[252];
    filebuf fName;

    fName.open("initConfig.txt", std::ios::in);
    istream myfile(&fName);

    if (myfile.good())
    {
        while (!myfile.eof())
        {
            myfile.ignore(512, '=');

            myfile.getline(line, 512);

            //Uncomment to see what its actually getting if desired
           // cout << line << '\n';
            parameters.push_back(atof(line));
        }
    }
    else {
        cout << "Unable to open file";
    }

   /* for (auto p : parameters) {
        cout << "intit Game p: " << p << endl;
    }*/

    numOfBoids = parameters[0];
    enclosureSize = parameters[1];
    deltaT = parameters[2];
    
    XMAX = enclosureSize;
    XMIN = -enclosureSize;

    YMAX = enclosureSize;
    YMIN = -enclosureSize;

    ZMAX = enclosureSize;
    ZMIN = -enclosureSize;
    
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    window = createGLFWWindow();
    if (window == NULL)
        return -1;

    //   //Initialize glad
    //   if (!gladLoadGL())
    //{
    //	cout << "GLAD init failed" << endl;
    //	return -1;
    //}


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

    initGL();

    initGameFromFile();

    GLuint skyboxProgram = initShader("skybox_vert.glsl", "skybox_frag.glsl");

    //Initialize shader
    GLuint program = initShader("vertex.glsl", "fragment.glsl");
    GLuint vao, vaoSquare, vaoBoid, vaoObs, vaoGoal;
    VertexBuffers vbo, vboSquare, vboBoid, vboObs, vboGoal;

    //Generate object ids
    glGenVertexArrays(1, &vao);
    glGenBuffers(VertexBuffers::COUNT, vbo.id);

    initVAO(vao, vbo);

    glGenVertexArrays(1, &vaoSquare);
    glGenBuffers(VertexBuffers::COUNT, vboSquare.id);

    initVAO(vaoSquare, vboSquare);

    glGenVertexArrays(1, &vaoBoid);
    glGenBuffers(VertexBuffers::COUNT, vboBoid.id);

    initVAO(vaoBoid, vboBoid);

    glGenVertexArrays(1, &vaoObs);
    glGenBuffers(VertexBuffers::COUNT, vboObs.id);

    initVAO(vaoObs, vboObs);

	glGenVertexArrays(1, &vaoGoal);
    glGenBuffers(VertexBuffers::COUNT, vboGoal.id);

    initVAO(vaoGoal, vboGoal);


    //Geometry information
    vector<vec3> points, normals;
    vector<unsigned int> indices;

    vector<vec3> pointsSquare, normalsSquare;
    vector<unsigned int> indicesSquare;

    vector<vec3> pointsBoid, normalsBoid;
    vector<unsigned int> indicesBoid;

    vector<vec3> pointsObs, normalsObs;
    vector<unsigned int> indicesObs;

    vector<vec3> pointsGoal, normalsGoal;
    vector<unsigned int> indicesGoal;

	vector<vec3> obstacles;

    generateBox(&points, &normals, &indices, 10);
    loadBuffer(vbo, points, normals, indices);

    generateSquare(pointsSquare, normalsSquare, indicesSquare, 600.f);
    loadBuffer(vboSquare, pointsSquare, normalsSquare, indicesSquare);

    generateBoid(pointsBoid, normalsBoid, indicesBoid, 10);
    loadBuffer(vboBoid, pointsBoid, normalsBoid, indicesBoid);

	generatePSphere(pointsGoal, normalsGoal, indicesGoal, 3.f, vec3(0,0,0), 10.f, 10.f, true);
	loadBuffer(vboGoal, pointsGoal, normalsGoal, indicesGoal);

    //generateTower(&pointsObs, &normalsObs, &indicesObs, 10);

	generatePSphere(pointsObs, normalsObs, indicesObs, 5.f, vec3(0,0,0), 10.f, 10.f);
    loadBuffer(vboObs, pointsObs, normalsObs, indicesObs);


    for (int i = 0; i < numOfBoids; i++) {
        boids.push_back(new Boid());
        boids[i]->updateParams();
        boids[i]->velocity = vec3(0.f, 0, 0.f);
       //(i%numOfBoids)?boids[i]->pred = false:boids[i]->pred = true;
    }

    randomlyPlaceBoids();

	placeObstacles(obstacles);

    //Camera cam = Camera(vec3(0, 0, -1), vec3(0, 0, 10));
    
    activeCamera = &cam;

    //float fovy, float aspect, float zNear, float zFar
    //mat4 perspectiveMatrix = perspective(radians(80.f), 1.f, 0.1f, 200.f);

    mat4 perspectiveMatrix = perspective(radians(80.f), 1.f, 0.1f, 20000.f);
    initSkybox();

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//Clear color and depth buffers (Haven't covered yet)

        mat4 goalModel = mat4(1.f);

        drawSkybox(skyboxProgram, skybox, winRatio*perspectiveMatrix*cam.getMatrix()*scale(mat4(1.f), vec3(8000.f)));
		
		if(goalMode == true){
			goalModel = translate(goalModel, goalPos);
			loadUniforms(program, winRatio*perspectiveMatrix*cam.getMatrix(),goalModel);
			render(vaoGoal, 0, indicesGoal.size());
		}
		for(auto obstaclePos: obstacles){
			mat4 obsModel = mat4(1.f);
			obsModel = translate(obsModel, obstaclePos);
			loadUniforms(program, winRatio*perspectiveMatrix*cam.getMatrix(),obsModel);
			render(vaoObs, 0, indicesObs.size());
		}
        for (auto boid : boids) {

            //AVOIDANCE CALCULATION
            vec3 averageAvoidBoidPosition = vec3(0.f, 0.f, 0.f);
            float weightSum = 0;
           // int totalCloseBoids;
            if (boid->pred == false) {
                for (Boid * nearBoid : boids) {
                    float distance = glm::distance(nearBoid->position, boid->position);
                   // if ((distance < (nearBoid->pred)? boid->avoidanceRadius*3:boid->avoidanceRadius) && boid->calcVisibility(nearBoid->position)) {
                      if ((distance < boid->avoidanceRadius) && boid->calcVisibility(nearBoid->position)) {
                  
                       // float weight = (nearBoid->pred == true)?10*(1-(distance / boid->avoidanceRadius)): (1 - (distance / boid->avoidanceRadius));
                        float weight = (1 - (distance / boid->avoidanceRadius));
                        averageAvoidBoidPosition += (weight * nearBoid->position);
                        weightSum += 1;
                    }
                }
            }

            if (weightSum != 0) {
                averageAvoidBoidPosition = -averageAvoidBoidPosition / weightSum;
            }

            //VELOCITY MATCHING CALCULATION
            vec3 averageVelocityMatch = vec3(0.f, 0.f, 0.f);
            weightSum = 0;
           // int totalCloseBoids;
            for (Boid * nearBoid : boids) {
				float distance = glm::distance(nearBoid->position, boid->position);
				if (  (distance < boid->velocityMatchRadius && distance > boid->avoidanceRadius) && boid->calcVisibility(nearBoid->position))  {
					float weight =1-( (distance - boid->avoidanceRadius )/ ( boid->velocityMatchRadius - boid->avoidanceRadius));
					
                    averageVelocityMatch  += (weight * nearBoid->velocity);
                    weightSum += 1;
				}
            }
            
            if(weightSum != 0){
				averageVelocityMatch  = averageVelocityMatch  / weightSum;
			}

            //FOLLOW CALCULATION
            vec3 averageFollowing = vec3(0.f, 0.f, 0.f);
            weightSum = 0;
           // int totalCloseBoids;
            for (Boid * nearBoid : boids) {
				float distance = glm::distance(nearBoid->position, boid->position);
				if (  (distance < boid->followRadius && distance > boid->avoidanceRadius) && boid->calcVisibility(nearBoid->position)) {
					float weight = 1-((distance - boid->velocityMatchRadius)/ ( boid->followRadius - boid->velocityMatchRadius));
					
                    averageFollowing += (weight * nearBoid->position);
                    weightSum += 1;
				}
            }
            
            if(weightSum != 0){
				averageFollowing  = averageFollowing  / weightSum;
			}
			
			vec3 flockGoal = vec3(0,0,0);
			if(goalMode == true){
				flockGoal = 10.f*(goalPos - boid->position);
			}
			//float distance = glm::distance(boid->position, goalPos);
			//if(distance <


            vec3 obstacleAvoid = vec3(0,0,0);
            for( auto obstaclePos: obstacles){
				float distance = glm::distance(boid->position, obstaclePos);
				if (distance < (3*boid->avoidanceRadius)) {
				   float weight = (boid->avoidanceWeight)* (1 - (distance / boid->avoidanceRadius));
				  // if(!isnan(weight)){
						obstacleAvoid += (weight * obstaclePos);
					//}
				}
			}
          //  cout << "ObstacleAvoid: " << obstacleAvoid.x << " " << obstacleAvoid.y << " " << obstacleAvoid.z << " " << endl;
            
//			cout << "averageAvoidBoidPosition: " << averageAvoidBoidPosition.x << " " <<averageAvoidBoidPosition.y<<" "<<averageAvoidBoidPosition.z<< endl;
//			cout << "averageVelocityMatch: " << averageVelocityMatch.x << " " <<averageVelocityMatch.y<<" "<<averageVelocityMatch.z<< endl;
//			cout << "averageFollowing: " << averageFollowing.x << " " <<averageFollowing.y<<" "<<averageFollowing.z<< endl;
      
            
            
            
            //FINAL HEADING CALCULATION
            boid->accelerationVector = flockGoal + -obstacleAvoid + vec3(boid->avoidanceWeight)*averageAvoidBoidPosition + vec3(boid->velocityMatchWeight)*averageVelocityMatch + vec3(boid->followWeight)*averageFollowing;
//			cout << "Final acceleration vector: " << boid->accelerationVector.x << " " <<boid->accelerationVector.y<<" "<<boid->accelerationVector.z<< endl;
        }

        for (auto boid : boids) {
            constrainPosition(boid);

            // Calc and set new velocity
            if (length(calculateNewVelocity(boid->velocity, boid->accelerationVector)) > boid->maxSpeed) {
                if( length(boid->velocity) == 0){
					boid->velocity = normalize(boid->accelerationVector) * boid->maxSpeed;
				}
                boid->velocity = normalize(boid->velocity) * boid->maxSpeed;
                //   cout << "danger" << endl;
            }
            else {
                boid->velocity = calculateNewVelocity(boid->velocity, boid->accelerationVector);
            }

            // Calc and set new position
            boid->position = calculateNewPosition(boid->position, boid->velocity);

            mat4 model = mat4(1.f);
            
            quat myQuat = rotation(vec3(0,0,1), normalize(boid->velocity));
            
            mat4 rotMat = mat4_cast(myQuat);
            model = translate(model, boid->position);
            model = model*rotMat;
            model = scale(model, vec3(scaleFactor));
            loadUniforms(program, winRatio*perspectiveMatrix*cam.getMatrix(), model);

            if (boid->pred == false) {
                render(vaoBoid, 0, indicesBoid.size());
            }
            else {
                render(vao, 0, indices.size());
            }
        }
        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();
    }

    // clean up allocated resources before exit
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(VertexBuffers::COUNT, vbo.id);
    glDeleteBuffers(VertexBuffers::COUNT, vboSquare.id);
    glDeleteProgram(program);


    glfwDestroyWindow(window);
    glfwTerminate();

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

bool CheckGLErrors(string location)
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << location << ": " << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << location << ": " << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << location << ": " << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << location << ": " << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << location << ": " << "GL_OUT_OF_MEMORY" << endl; break;
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

// ==========================================================================
