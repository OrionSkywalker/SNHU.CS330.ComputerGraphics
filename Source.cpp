#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.H>

using namespace std;

int width, height;
const double PI = 3.14159;
const float toRadians = PI / 180.0f;

//used for perspective loop
bool perspectiveChecker = false;

// Input Callback Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

//Declare View Matrix
glm::mat4 viewMatrix;

//Initialize Camera FOV
GLfloat fov = 45.f;

void initCamera();

//Define Camera Attributes
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.f, 10.f);
glm::vec3 target = glm::vec3(0.0f, 0.f, 0.f);
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target);
glm::vec3 worldUp = glm::vec3(0.0f, 1.f, 0.f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.f, -1.f));

//Declare target prototype
glm::vec3 getTarget();

//Camera Transformation prototype
void TransformCamera();

//Boolean for keys and mouse buttons
bool keys[1024], mouseButtons[3];

//Boolean to check camera tranformation
bool isPanning = false, isOrbiting = false;

//Radius, Pitch, and Yaw
GLfloat radius = 20.f, rawYaw = 0.f, rawPitch = 0.f, degYaw, degPitch;

//variables used for keeping movements consistent regardless of system
GLfloat deltaTime = 0.f, lastFrame = 0.f;
//variables for center of screen and difference or change in x or y
GLfloat lastX = 320, lastY = 240, xChange, yChange; //center mouse cursor

//Detect initial mouse movement
bool firstMouseMove = true;

void initCamera();

//Light source position
glm::vec3 lightPosition(0.0f, 0.35f, 0.0f); //adjust position with these
glm::vec3 lightPosition2(5.0f, 0.8f, 1.0f); //added a second position for the second light

// Draw Primitive(s)
void draw()
{
	GLenum mode = GL_TRIANGLES;
	GLsizei indices = 48;  //having less indices isn't a problem
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);


}

// Draw Primitive(s)
void drawCylinder()
{
	GLenum mode = GL_TRIANGLES;
	GLsizei indices = 36;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);


}

// Create and Compile Shaders
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// Create Shader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code to Shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile Shader
	glCompileShader(shaderID);

	// Return ID of Compiled shader
	return shaderID;

}

// Create Program Object
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);

	// Return Shader Program
	return shaderProgram;

}


int main(void)
{
	width = 640; height = 480;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	//Set input callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;

	GLfloat lampVertices[] = {
		-0.5, -0.5, 0.0, // index 0
		-0.5, 0.5, 0.0, // index 1
		0.5, -0.5, 0.0,  // index 2	
		0.5, 0.5, 0.0  // index 3	
	};

	//vertices for the provided boxes
	GLfloat vertices[] = {

		// Triangle 1
		-0.5, -0.5, 0.0, // index 0
		1.0, 0.65, 0.0, // orange

		-0.5, 0.5, 0.0, // index 1
		1.0, 0.65, 0.0, // orange

		0.5, -0.5, 0.0,  // index 2	
		1.0, 0.65, 0.0, // orange

		// Triangle 2	
		0.5, 0.5, 0.0,  // index 3	
		1.0, 0.65, 0.0 // orange
	};

	GLfloat cylinderVertices[] = {
		//rear panel points 

		//bottom left
		0.25, 0.0, 1.0,	//point (0)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		//bottom right
		-0.25, 0.0, 1.0,  //1
		0.0, 0.0, 0.0,
		0.32, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top left
		0.25, 2.0, 1.0,  //2
		0.0, 0.0, 0.0,
		0.0, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top right
		-0.25, 2.0, 1.0,  //3
		0.0, 0.0, 0.0,
		0.32, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z

		//left rear panel points

		//bottom left
		-0.25, 0.0, 1.0, //4
		0.0, 0.0, 0.0,
		0.33, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//botom right
		-0.5, 0.0, 0.5, //5
		0.0, 0.0, 0.0,
		0.67, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top left 
		-0.25, 2.0, 1.0, //6
		0.0, 0.0, 0.0,
		0.33, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top right
		-0.5, 2.0, 0.5, //7
		0.0, 0.0, 0.0,
		0.67, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z

		//Left front panel
		//bottom left
		-0.5, 0.0, 0.5, //8
		0.0, 0.0, 0.0,
		0.68,0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//bottom right
		-0.25, 0.0, 0.0, //9
		0.0, 0.0, 0.0,
		1.0, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top left
		-0.5, 2.0, 0.5, //10
		0.0, 0.0, 0.0,
		0.68, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top right 
		-0.25, 2.0, 0.0, //11
		0.0, 0.0, 0.0,
		1.0, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		 
		//Front Panel
		//bottom left
		-0.25, 0.0, 0.0, //12
		0.0, 0.0, 0.0,
		0.0, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//bottom right
		0.25, 0.0, 0.0, //13
		0.0, 0.0, 0.0,
		0.32, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top left
		-0.25, 2.0, 0.0, //14
		0.0, 0.0, 0.0,
		0.0, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top right
		0.25, 2.0, 0.0,  //15
		0.0, 0.0, 0.0,
		0.32, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		
		//right front panel
		//bottom left
		0.25, 0.0, 0.0, //16
		0.0, 0.0, 0.0,
		0.33, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//bottom right
		0.5, 0.0, 0.5, //17
		0.0, 0.0, 0.0,
		0.67, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top left
		0.25, 2.0, 0.0, //18
		0.0, 0.0, 0.0,
		0.33, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top right
		0.5, 2.0, 0.5,  //19
		0.0, 0.0, 0.0,
		0.67, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z

		//right rear panel
		//bottom left
		0.5, 0.0, 0.5, //20
		0.0, 0.0, 0.0,
		0.68, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//bottom right
		0.25, 0.0, 1.0, //21
		0.0, 0.0, 0.0,
		1.0, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top left
		0.5, 2.0, 0.5, //22
		0.0, 0.0, 0.0,
		0.68, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z
		//top right
		0.25, 2.0, 1.0, //23
		0.0, 0.0, 0.0,
		1.0, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z


		//make a top piece
		//top middle
			-.25, 2.0, 1.0, //24
			0.0, 0.0, 0.0,
			0.0, 1.0,
			0.0f, 0.0f, 1.0f, //normal positive z
			0.25, 2.0, 1.0, //25
			0.0, 0.0, 0.0,
			1.0, 1.0,
			0.0f, 0.0f, 1.0f, //normal positive z
			0.25, 2.0, 0.0, //26
			0.0, 0.0, 0.0,
			1.0, 0.0,
			0.0f, 0.0f, 1.0f, //normal positive z
		//bottom middle
			//24 again
			//26 again
			-0.25, 2.0, 0.0, //27
			0.0, 0.0, 0.0,
			0.0, 0.0,
			0.0f, 0.0f, 1.0f, //normal positive z
		//left 
			-0.5, 2.0, 0.5, //28
			0.0, 0.0, 0.0,
			0.0, 0.0,
			0.0f, 0.0f, 1.0f, //normal positive z
			//24 again
			//27 again
		//right
			0.5, 2.0, 0.5, //29
			0.0, 0.0, 0.0,
			1.0, 0.0,
			0.0f, 0.0f, 1.0f //normal positive z

			//25 again
			//26 again
	
	};
	GLubyte cylinderIndices[] = {
		0, 1, 2,
		1, 2, 3,
		4, 5, 6,
		5, 6, 7,
		8, 9, 10,
		9, 10, 11,
		12, 13, 14,
		13, 14, 15,
		16, 17, 18,
		17, 18, 19,
		20, 21, 22,
		21, 22, 23,
		24, 25, 26,
		24, 26, 27,
		28, 24, 27,
		29, 25, 26
	};

	GLubyte cubeIndices[] = {
		0, 3, 1,
		0, 3, 2,
		4, 7, 5,
		4, 7, 6,
		8, 11, 9, 
		8, 11, 10,
		12, 15, 13,
		12, 15, 14,
		16, 19, 17,
		16, 19, 18,
		20, 23, 21,
		20, 23, 22
	};

	GLfloat verticesCube[] = {
		//Cube Points, Top, bottom, left, right points. 
		//Order of points is top left, top right, bottom left, bottom right
		//pattern to draw cube with points is index 0,3,1 then 0,3,2 This completes the panel

		//The top, points 0-3:
		0.0, 1.0, 1.0,	//point (0)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 1.0, 1.0,	//point (1)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		0.0, 1.0, 0.0,	//point (2)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 1.0, 0.0,	//point (3)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z

		//The bottom, points 4-7:
		0.0, 0.0, 1.0,	//point (4)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 0.0, 1.0,	//point (5)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		0.0, 0.0, 0.0,	//point (6)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 0.0, 0.0,	//point (7)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z

		//The left, points 8-11:
		0.0, 1.0, 1.0,	//point (8)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		0.0, 1.0, 0.0,	//point (9)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		0.0, 0.0, 1.0,	//point (10)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		0.0, 0.0, 0.0,	//point (11)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z

		//The right, points 12-15:
		1.0, 1.0, 1.0,	//point (12)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 1.0, 0.0,	//point (13)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 0.0, 1.0,	//point (14)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 0.0, 0.0,	//point (15)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z

		//The front, points 16-19:
		0.0, 1.0, 0.0,	//point (16)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 1.0, 0.0,	//point (17)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		0.0, 0.0, 0.0,	//point (18)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 0.0, 0.0,	//point (19)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z

		//The back, points 20-23:
		0.0, 1.0, 1.0,	//point (20)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 1.0, 1.0,	//point (21)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 1.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		0.0, 0.0, 1.0,	//point (22)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		1.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f, //normal positive z
		1.0, 0.0, 1.0,	//point (23)
		0.0, 0.0, 0.0, //color not used but kept to use with current shader
		0.0, 0.0,	//uv
		0.0f, 0.0f, 1.0f //normal positive z
	};


	//vertices for the floor
	GLfloat verticesFloor[] = {

		// Triangle 1
		-0.5, -0.5, 0.0, // index 0
		0.0, 0.5, 0.0, // color
		0.0, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z

		-0.5, 0.5, 0.0, // index 1
		0.0, 0.5, 0.0, // color
		0.0, 1.0,
		0.0f, 0.0f, 1.0f, //normal positive z

		0.5, -0.5, 0.0,  // index 2	
		0.0, 0.5, 0.0, // color
		1.0, 0.0,
		0.0f, 0.0f, 1.0f, //normal positive z

		// Triangle 2	
		0.5, 0.5, 0.0,  // index 3	
		1.0, 0.5, 1.0, // color
		1.0, 1.0,
		0.0f, 0.0f, 1.0f //normal positive z
	};

	// Define element indices
	GLubyte indices[] = {
		0, 1, 2,
		1, 2, 3
	};

	// Plane Transforms
	glm::vec3 planePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.5f), // front plane
		glm::vec3(0.5f,  0.0f,  0.0f), // Right Plane
		glm::vec3(0.0f,  0.0f,  -0.5f), // Back Plane
		glm::vec3(-0.5f, 0.0f,  0.0f) // Left Plane
	};

	// For Lights
	glm::vec3 planePositions3[] = {
		glm::vec3(0.0f,  0.0f,  0.5f),
		glm::vec3(0.5f,  0.0f,  0.0f),
		glm::vec3(0.0f,  0.0f,  -0.5f),
		glm::vec3(-0.5f, 0.0f,  0.0f),
		glm::vec3(0.0f, 0.5f,  0.0f),
		glm::vec3(0.0f, -0.5f,  0.0f)
	};

	//editing to make cylinder
	glm::vec3 planePositions2[] = {
		glm::vec3(0.f,  0.0f,  0.4f), // front plane
		glm::vec3(0.375f,  0.0f,  0.18f), // Right front Plane
		glm::vec3(0.f,  0.0f,  -0.455f), // Back Plane
		glm::vec3(-0.375f, 0.0f,  0.18f),  // Left front Plane
		glm::vec3(-0.375f, 0.0f, -0.25f), //left rear
		glm::vec3(0.375f, 0.0f, -0.25f) //right rear
	};

	//used to rotate cube
	glm::float32 planeRotations[] = {
		0.0f, 90.0f, 0.0f, 90.0f
	};

	glm::float32 planeRotations3[] = {
		0.0f, 90.0f, 180.0f, -90.0f, -90.f, 90.f
	};

	//used to rotate cylinder pieces
	glm::float32 planeRotations2[] = {
		0.0f, 60.0f, 0.0f, 300.0f,60.f,300.f
	};

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);



	GLuint  floorVBO, floorEBO, floorVAO, cylinderVBO, cylinderEBO, cylinderVAO, lampVBO, lampEBO, lampVAO, cubeVBO, cubeEBO, cubeVAO, boardVBO, boardEBO, boardVAO;

	

	glGenBuffers(1, &floorVBO); // Create VBO
	glGenBuffers(1, &floorEBO); // Create EBO

	glGenBuffers(1, &cylinderVBO); // Create VBO
	glGenBuffers(1, &cylinderEBO); // Create EBO

	glGenBuffers(1, &lampVBO); // Create VBO
	glGenBuffers(1, &lampEBO); // Create EBO

	glGenBuffers(1, &cubeVBO); // Create VBO
	glGenBuffers(1, &cubeEBO); // Create EBO

	glGenBuffers(1, &boardVBO); // Create VBO
	glGenBuffers(1, &boardEBO); // Create EBO
	
	glGenVertexArrays(1, &floorVAO); // Create VAO
	glGenVertexArrays(1, &cylinderVAO); // Create VAO
	glGenVertexArrays(1, &lampVAO); // Create VOA
	glGenVertexArrays(1, &cubeVAO); // Create VOA
	glGenVertexArrays(1, &boardVAO); // Create VOA



	glBindVertexArray(boardVAO);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, boardVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boardEBO); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	glBindVertexArray(cubeVAO);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)


	glBindVertexArray(floorVAO);

	glBindBuffer(GL_ARRAY_BUFFER, floorVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO); // Select EBO

	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesFloor), verticesFloor, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//arguments are(index of pointer, size, glfloat, glfalse, span, where to start)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);



	glBindVertexArray(cylinderVAO);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderEBO); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderVertices), cylinderVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cylinderIndices), cylinderIndices, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	//lampVAO
	glBindVertexArray(lampVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lampVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lampEBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//Load textures
	int glueTexWidth, glueTexHeight, woodTexWidth, woodTexHeight, cubeTexWidth, cubeTexHeight, boardTexWidth, boardTexHeight;
	unsigned char* glueImage = SOIL_load_image("glueStick.png", &glueTexWidth, &glueTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* woodImage = SOIL_load_image("woodTexture.jpeg", &woodTexWidth, &woodTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* cubeImage = SOIL_load_image("rubik_cube_PNG53.png", &cubeTexWidth, &cubeTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* boardImage = SOIL_load_image("board.png", &boardTexWidth, &boardTexHeight, 0, SOIL_LOAD_RGB);

	//Generate Textures
	GLuint glueTexture;
	glGenTextures(1, &glueTexture); //number of objects and where they go
	glBindTexture(GL_TEXTURE_2D, glueTexture); //the type and the reference
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, glueTexWidth, glueTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, glueImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(glueImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint woodTexture;
	glGenTextures(1, &woodTexture); //number of objects and where they go
	glBindTexture(GL_TEXTURE_2D, woodTexture); //the type and the reference
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, woodTexWidth, woodTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, woodImage);
	glGenerateMipmap(GL_TEXTURE_2D); //handles resolution
	SOIL_free_image_data(woodImage); //free resource
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint cubeTexture;
	glGenTextures(1, &cubeTexture); //number of objects and where they go
	glBindTexture(GL_TEXTURE_2D, cubeTexture); //the type and the reference
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cubeTexWidth, cubeTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeImage);
	glGenerateMipmap(GL_TEXTURE_2D); //handles resolution
	SOIL_free_image_data(cubeImage); //free resource
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint boardTexture;
	glGenTextures(1, &boardTexture); //number of objects and where they go
	glBindTexture(GL_TEXTURE_2D, boardTexture); //the type and the reference
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, boardTexWidth, boardTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, boardImage);
	glGenerateMipmap(GL_TEXTURE_2D); //handles resolution
	SOIL_free_image_data(boardImage); //free resource
	glBindTexture(GL_TEXTURE_2D, 0);


	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec3 aColor;"
		"layout(location = 2) in vec2 texCoord;"
		"layout(location = 3) in vec3 normal;"
		"out vec3 oColor;"
		"out vec2 oTexCoord;"
		"out vec3 oNormal;"
		"out vec3 FragPos;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"oColor = aColor;"
		"oTexCoord = texCoord;"
		"oNormal = mat3(transpose(inverse(model))) * normal;"
		"FragPos = vec3(model * vec4(vPosition, 1.0f));"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec3 oColor;"
		"in vec2 oTexCoord;"
		"in vec3 oNormal;"
		"in vec3 FragPos;"
		"out vec4 fragColor;"
		"uniform sampler2D myTexture;"
		"uniform vec3 objectColor;"
		"uniform vec3 lightColor;"
		"uniform vec3 lightColor2;"
		"uniform vec3 lightPos;"
		"uniform vec3 lightPos2;"
		"uniform vec3 viewPos;"
		"void main()\n"
		"{\n"
		"//Ambient\n"
		"float ambientStrength = 3.0f;"
		"vec3 ambient = ambientStrength * lightColor;"
		"//Ambient 2\n"
		"float ambientStrength2 = 3.0f;"
		"vec3 ambient2 = ambientStrength2 * lightColor2;"
		"//Diffuse\n"
		"vec3 norm = normalize(oNormal);"
		"vec3 lightDir = normalize(lightPos - FragPos);"
		"float diff = max(dot(norm, lightDir), 0.0);"
		"vec3 diffuse = diff * lightColor;"
		"//Diffuse 2\n"
		"vec3 norm2 = normalize(oNormal);"
		"vec3 lightDir2 = normalize(lightPos2 - FragPos);"
		"float diff2 = max(dot(norm2, lightDir2), 0.0);"
		"vec3 diffuse2 = diff2 * lightColor2;"
		"//Specularity\n"
		"float specularStrength = 5.0f;"
		"vec3 viewDir = normalize(viewPos - FragPos);"
		"vec3 reflectDir = reflect(-lightDir, norm);"
		"float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);"
		"vec3 specular = specularStrength * spec * lightColor;"
		"//Specularity 2\n"
		"float specularStrength2 = 5.0f;"
		"vec3 specular2 = specularStrength2 * spec * lightColor2;"
		"vec3 result = (ambient + diffuse + specular) * objectColor;"
		"result += (ambient2 + diffuse2 + specular2) * objectColor;"
		"fragColor = texture(myTexture, oTexCoord) * vec4(result, 1.0f);"
		"}\n";

	// Lamp Vertex shader source code
	string lampVertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"}\n";

	// Lamp Fragment shader source code
	string lampFragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor =vec4(1.0f);"
		"}\n";

	// Creating Shader Program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	// Creating Lamp Shader Program
	GLuint lampShaderProgram = CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource);

	// Use Shader Program exe once
	//glUseProgram(shaderProgram);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Set Delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //removed "|  GL_DEPTH_BUFFER_BIT" to check if makes an ortho view

		// Use Shader Program exe and select VAO before drawing 
		glUseProgram(shaderProgram); // Call Shader per-frame when updating attributes


		// Declare transformations (can be initialized outside loop)
		//glm::mat4 modelMatrix;
		//glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;

		viewMatrix = glm::lookAt(cameraPosition, getTarget(), worldUp);



		//modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		//modelMatrix = glm::rotate(modelMatrix, 0.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		//modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));		
		/*
		viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, -5.0f));
		viewMatrix = glm::rotate(viewMatrix, 45.0f * toRadians, glm::vec3(1.f, 0.0f, 0.0f));//change the float*radians to adjust the angle and the vec3 is x,y,z
		*/
		//Pseudocode: if p pressed then switch projections
		
		if (keys[GLFW_KEY_P] || perspectiveChecker == false) {
			perspectiveChecker = true;
			if (projectionMatrix == glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f)) {
				projectionMatrix == glm::ortho(0.0f, 10.0f, 0.0f, 10.0f);
			}
			else
				projectionMatrix == glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
			
		}
		projectionMatrix = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		//projectionMatrix = glm::ortho(0.0f, 10.0f, 0.0f, 10.0f);

		// Get matrix's uniform location and set matrix
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

		//Get light and object color, and light position location
		GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
		GLint lightColorLoc2 = glGetUniformLocation(shaderProgram, "lightColor2");
		GLint lightPosLoc2 = glGetUniformLocation(shaderProgram, "lightPos2");

		//Assign Light and Object Colors, 0.46f, 0.36f, 0.25f,  0.79f, 0.39f, 0.13f
		glUniform3f(objectColorLoc, 0.1f, 0.1f, 0.1f);
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightColorLoc2, 1.0f, 1.0f, 1.0f);


		//Set light position 
		glUniform3f(lightPosLoc, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightPosLoc2, lightPosition2.x, lightPosition2.y, lightPosition2.y);

		//Specify view position (camera)
		glUniform3f(viewPosLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		

		/*
		//experimenting on cube, creating cylinder with 6 sides using planePositions2[]. Moving points experimentally by changing position and rotation.
		glBindVertexArray(cubeVAO); // User-defined VAO must be called before draw.
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, planePositions2[i]);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, planeRotations2[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 2.5f, 0.5f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw();
		}
		glBindVertexArray(0); //Incase different VAO wii be used after
		*/

		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, glueTexture);

		// Select and transform cylinder
		glBindVertexArray(cylinderVAO);
		glm::mat4 modelMatrix;
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 2.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		draw();
		glBindVertexArray(0); //Incase different VAO will be used after

		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, cubeTexture);

		// Select and transform cube
		glBindVertexArray(cubeVAO);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(2.2f, 1.5f, 2.2f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.f, 0.0f, 1.f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		draw();
		glBindVertexArray(0); //Incase different VAO will be used after

		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, boardTexture);

		// Select and transform cube
		glBindVertexArray(boardVAO);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(3.f, 0.15f, 1.f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.0f, 0.f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		draw();
		glBindVertexArray(0); //Incase different VAO will be used after

		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		
	    // Select and transform floor
		glBindVertexArray(floorVAO);
		
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, 0.0f, 0.f));
		modelMatrix = glm::rotate(modelMatrix, 90.f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(20.f, 20.f, 20.f)); //increased the plane size 
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		draw();
		glBindVertexArray(0); //Incase different VAO will be used after

		//use shader
		glUseProgram(lampShaderProgram);

		//get matrix uniform location and set matrix
		GLint lampModelLoc = glGetUniformLocation(lampShaderProgram, "model");
		GLint lampViewLoc = glGetUniformLocation(lampShaderProgram, "view");
		GLint lampProjLoc = glGetUniformLocation(lampShaderProgram, "projection");
		glUniformMatrix4fv(lampViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(lampProjLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindVertexArray(lampVAO); // User-defined VAO must be called before draw. 

		// Transform planes to form cube
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, planePositions3[i] / glm::vec3(8., 8., 8.) + lightPosition);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, 5.0f, 0.f));
			modelMatrix = glm::rotate(modelMatrix, planeRotations3[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f, 0.125f, 0.125f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, planeRotations3[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw();
		}
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, planePositions3[i] / glm::vec3(8., 8., 8.) + lightPosition2);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, 5.0f, 0.f));
			modelMatrix = glm::rotate(modelMatrix, planeRotations3[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.125f, 0.125f, 0.125f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, planeRotations3[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw();
		}

		// Unbind Shader exe and VOA after drawing per frame
		glBindVertexArray(0); //Incase different VAO wii be used after






		glUseProgram(0); // Incase different shader will be used after

	    /* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		//Poll Camera Transformations
		TransformCamera();
	}

	//Clear GPU resources
	glDeleteVertexArrays(1, &cylinderVAO);
	glDeleteBuffers(1, &cylinderVBO);
	glDeleteBuffers(1, &cylinderEBO);
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &floorVBO);
	glDeleteBuffers(1, &floorEBO);


	glfwTerminate();
	return 0;
}

//Define input callback functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//Display ASCII keycode
	//cout << " ASCII: " << key << endl;

	if (action == GLFW_PRESS) {
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		keys[key] = false;
	}
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	/*
	//Display scroll offset
	if (yoffset > 0)
		cout << "Scroll up: ";
	if (yoffset < 0)
		cout << "Scroll down: ";
	cout << yoffset << endl;
	*/

	//Clamp fov
	if (fov >= 1.f && fov <= 45.f)
		fov -= yoffset * 0.05f;//changing the float value changes the zoom speed up/down

	//Default fov if outside bounds
	if (fov < 1.f)
		fov = 1.f;
	if (fov > 45.f)
		fov = 45.f;

}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	/*
	// Detect mouse button clicks
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		cout << "LMB clicked!" << endl;
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		cout << "MMB clicked!" << endl;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		cout << "RMB clicked!" << endl;
	*/

	if (action == GLFW_PRESS)
		mouseButtons[button] = true;
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	//Display mouse x and y coordinates
	//cout << "Mouse X: " << xpos << endl;
	//cout << "Mouse Y: " << ypos << endl;

	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	//calculate cursor offset
	xChange = xpos - lastX;
	yChange = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	//Pan camera
	if (isPanning)
	{
		//attempt at rectifying issues when panning
		if (cameraPosition.z < 0.f)
			cameraFront.z = 1.f;
		//else
			//cameraFront.z = -1.f;


		GLfloat cameraSpeed = xChange * deltaTime;//x
		cameraPosition += cameraSpeed * cameraRight;

		cameraSpeed = yChange * deltaTime;//y or "up"
		cameraPosition += cameraSpeed * cameraUp;
	}


	//Orbit camera
	if (isOrbiting)
	{
		rawYaw += xChange;
		rawPitch += yChange;

		//Convert yaw and pitch to degrees
		degYaw = glm::radians(rawYaw);
		//degPitch = glm::radians(rawPitch);
		degPitch = glm::clamp(glm::radians(rawPitch), -glm::pi<float>() / 2.f + 0.1f, glm::pi<float>() / 2.f - 0.1f);

		//Azimuth Altitude formula
		cameraPosition.x = target.x + radius * cosf(degPitch) * sinf(degYaw);
		cameraPosition.y = target.y + radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degPitch) * cosf(degYaw);
	}


}

//Define get target function
glm::vec3 getTarget() {
	if (isPanning)
		target = cameraPosition + cameraFront;

	return target;
}
//Define TransformCamera function
void TransformCamera() {
	//Pan Camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE])
		isPanning = true;
	else
		isPanning = false;

	//Orbit camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
		isOrbiting = true;
	else
		isOrbiting = false;

	//reset camera
	if (keys[GLFW_KEY_F])
		initCamera();
}

void initCamera()
{
	cameraPosition = glm::vec3(0.0f, 0.f, 3.f);
	target = glm::vec3(0.0f, 0.f, 0.f);
	cameraDirection = glm::normalize(cameraPosition - cameraDirection);
	worldUp = glm::vec3(0.0f, 1.f, 0.f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.0f, 0.f, -1.f));
}