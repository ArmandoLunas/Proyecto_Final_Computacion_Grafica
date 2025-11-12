// Estudiante: Armando Luna Juárez 319056323
// Práctica 12: Animación por KeyFrames
// Fecha de entrega: 06/11/2025
#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"


// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Animation.h"
#include "Animator.h"

#include <fstream>
#include <sstream>
#include <string>

const char* KEYFRAMES_PATH = "dog_keyframes.txt";

// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();
void Animation_dog();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,2.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f,  0.0f),
	glm::vec3(0.0f,0.0f, 0.0f)
};

float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};


glm::vec3 Light1 = glm::vec3(0);
//Anim
float rotBall = 0.0f;
float rotDog = 0.0f;
int dogAnim = 0;
float FLegs = 0.0f; 
float RLegs = 0.0f;
float legFL = 0.0f; // Pata delantera Izquierda
float legFR = 0.0f; // Pata delantera Derecha
float legBL = 0.0f; // Pata trasera Izquierda
float legBR = 0.0f; // Pata trasera Derecha
float head = 0.0f;
float tail = 0.0f;
float rotXDog = 0.0f;



//KeyFrames
float dogPosX , dogPosY , dogPosZ  ;

#define MAX_FRAMES 9
int i_max_steps = 190;
int i_curr_steps = 0;
typedef struct _frame {
	
	float rotDog;
	float rotDogInc;
	float dogPosX;
	float dogPosY;
	float dogPosZ;
	float incX;
	float incY;
	float incZ;
	float head;
	float headInc;
	float legFL;    float legFLInc;
	float legFR;    float legFRInc;
	float legBL;    float legBLInc;
	float legBR;    float legBRInc;
	float tail;     float tailInc;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//introducir datos
bool play = false;
int playIndex = 0;



void saveFrame(void)
{

	printf("frameindex %d\n", FrameIndex);

	KeyFrame[FrameIndex].dogPosX = dogPosX;
	KeyFrame[FrameIndex].dogPosY = dogPosY;
	KeyFrame[FrameIndex].dogPosZ = dogPosZ;

	KeyFrame[FrameIndex].rotDog = rotDog;
	KeyFrame[FrameIndex].head = head;

	KeyFrame[FrameIndex].legFL = legFL;
	KeyFrame[FrameIndex].legFR = legFR;
	KeyFrame[FrameIndex].legBL = legBL;
	KeyFrame[FrameIndex].legBR = legBR;
	KeyFrame[FrameIndex].tail = tail;

	FrameIndex++;
}

void resetElements(void)
{
	dogPosX = KeyFrame[0].dogPosX;
	dogPosY = KeyFrame[0].dogPosY;
	dogPosZ = KeyFrame[0].dogPosZ;
	head = KeyFrame[0].head;
	rotDog = KeyFrame[0].rotDog;
	legFL = KeyFrame[0].legFL;
	legFR = KeyFrame[0].legFR;
	legBL = KeyFrame[0].legBL;
	legBR = KeyFrame[0].legBR;
	tail = KeyFrame[0].tail;

}
void interpolation(void)
{

	KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].dogPosX - KeyFrame[playIndex].dogPosX) / i_max_steps;
	KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].dogPosY - KeyFrame[playIndex].dogPosY) / i_max_steps;
	KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].dogPosZ - KeyFrame[playIndex].dogPosZ) / i_max_steps;
	KeyFrame[playIndex].headInc = (KeyFrame[playIndex + 1].head - KeyFrame[playIndex].head) / i_max_steps;
	KeyFrame[playIndex].rotDogInc = (KeyFrame[playIndex + 1].rotDog - KeyFrame[playIndex].rotDog) / i_max_steps;
	KeyFrame[playIndex].legFLInc = (KeyFrame[playIndex + 1].legFL - KeyFrame[playIndex].legFL) / i_max_steps;
	KeyFrame[playIndex].legFRInc = (KeyFrame[playIndex + 1].legFR - KeyFrame[playIndex].legFR) / i_max_steps;
	KeyFrame[playIndex].legBLInc = (KeyFrame[playIndex + 1].legBL - KeyFrame[playIndex].legBL) / i_max_steps;
	KeyFrame[playIndex].legBRInc = (KeyFrame[playIndex + 1].legBR - KeyFrame[playIndex].legBR) / i_max_steps;
	KeyFrame[playIndex].tailInc = (KeyFrame[playIndex + 1].tail - KeyFrame[playIndex].tail) / i_max_steps;


}

void SaveAllKeyframesToFile(const char* path)
{
	std::ofstream out(path, std::ios::out | std::ios::trunc);
	if (!out.is_open()) {
		std::cerr << "[Keyframes] No se pudo abrir para escribir: " << path << "\n";
		return;
	}

	// encabezado: cantidad de frames e i_max_steps actual
	out << FrameIndex << " " << i_max_steps << "\n";

	// por cada frame escribe 10 valores
	for (int i = 0; i < FrameIndex; ++i) {
		out << KeyFrame[i].dogPosX << " "
			<< KeyFrame[i].dogPosY << " "
			<< KeyFrame[i].dogPosZ << " "
			<< KeyFrame[i].rotDog << " "
			<< KeyFrame[i].head << " "
			<< KeyFrame[i].legFL << " "
			<< KeyFrame[i].legFR << " "
			<< KeyFrame[i].legBL << " "
			<< KeyFrame[i].legBR << " "
			<< KeyFrame[i].tail
			<< "\n";
	}

	out.close();
	std::cout << "[Keyframes] Guardados " << FrameIndex << " frames en " << path << "\n";
}

bool LoadKeyframesFromFile(const char* path)
{
	std::ifstream in(path);
	if (!in.is_open()) {
		std::cerr << "[Keyframes] No existe o no se pudo abrir: " << path << "\n";
		return false;
	}

	int nFrames = 0;
	int loadedMaxSteps = i_max_steps; // fallback al actual si no se puede leer

	// Lee encabezado
	{
		std::string header;
		if (!std::getline(in, header)) {
			std::cerr << "[Keyframes] Archivo vacío o inválido\n";
			return false;
		}
		std::istringstream hs(header);
		if (!(hs >> nFrames >> loadedMaxSteps)) {
			std::cerr << "[Keyframes] Encabezado inválido\n";
			return false;
		}
	}

	if (nFrames <= 0) {
		std::cerr << "[Keyframes] nFrames inválido\n";
		return false;
	}

	// Limita por seguridad
	if (nFrames > MAX_FRAMES) {
		std::cerr << "[Keyframes] Aviso: " << nFrames
			<< " excede MAX_FRAMES (" << MAX_FRAMES
			<< "). Se truncará.\n";
		nFrames = MAX_FRAMES;
	}

	// Inicializa por si acaso
	for (int i = 0; i < MAX_FRAMES; ++i) {
		KeyFrame[i].dogPosX = KeyFrame[i].dogPosY = KeyFrame[i].dogPosZ = 0.0f;
		KeyFrame[i].incX = KeyFrame[i].incY = KeyFrame[i].incZ = 0.0f;
		KeyFrame[i].rotDog = KeyFrame[i].rotDogInc = 0.0f;
		KeyFrame[i].head = KeyFrame[i].headInc = 0.0f;
		KeyFrame[i].legFL = KeyFrame[i].legFLInc = 0.0f;
		KeyFrame[i].legFR = KeyFrame[i].legFRInc = 0.0f;
		KeyFrame[i].legBL = KeyFrame[i].legBLInc = 0.0f;
		KeyFrame[i].legBR = KeyFrame[i].legBRInc = 0.0f;
		KeyFrame[i].tail = KeyFrame[i].tailInc = 0.0f;
	}

	// Lee cada frame
	int i = 0;
	for (; i < nFrames; ++i) {
		std::string line;
		if (!std::getline(in, line)) {
			std::cerr << "[Keyframes] Fin de archivo antes de tiempo\n";
			break;
		}
		std::istringstream ls(line);
		float px, py, pz, rdog, h, fl, fr, bl, br, tt;
		if (!(ls >> px >> py >> pz >> rdog >> h >> fl >> fr >> bl >> br >> tt)) {
			std::cerr << "[Keyframes] Línea de frame inválida en índice " << i << "\n";
			break;
		}

		KeyFrame[i].dogPosX = px;
		KeyFrame[i].dogPosY = py;
		KeyFrame[i].dogPosZ = pz;
		KeyFrame[i].rotDog = rdog;
		KeyFrame[i].head = h;
		KeyFrame[i].legFL = fl;
		KeyFrame[i].legFR = fr;
		KeyFrame[i].legBL = bl;
		KeyFrame[i].legBR = br;
		KeyFrame[i].tail = tt;
	}

	FrameIndex = i;                          // actualiza cuántos frames hay
	i_max_steps = loadedMaxSteps;             // usa los pasos del archivo
	in.close();

	if (FrameIndex > 0) {
		// copia el primer frame a los estados actuales
		resetElements();
		std::cout << "[Keyframes] Cargados " << FrameIndex
			<< " frames desde " << path
			<< " (i_max_steps = " << i_max_steps << ")\n";
		return true;
	}

	std::cerr << "[Keyframes] No se cargó ningún frame válido\n";
	return false;
}


// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Armando Luna Juarez", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);



	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
	
	// Shader para modelos con huesos
	Shader shaderEsqueletico("Shader/lighting_skeletical.vs", "Shader/lighting.frag");
	
	Model humanModel((char*)"Models/humano-animation.fbx");
	Animation humanAnimation((char*)"Models/humano-animation.fbx", &humanModel);
	Animator humanAnimator(&humanAnimation);

	//models
	Model DogBody((char*)"Models/DogBody.obj");
	Model HeadDog((char*)"Models/HeadDog.obj");
	Model DogTail((char*)"Models/TailDog.obj");
	Model F_RightLeg((char*)"Models/F_RightLegDog.obj");
	Model F_LeftLeg((char*)"Models/F_LeftLegDog.obj");
	Model B_RightLeg((char*)"Models/B_RightLegDog.obj");
	Model B_LeftLeg((char*)"Models/B_LeftLegDog.obj");
	Model Piso((char*)"Models/galeria.obj");
	Model Ball((char*)"Models/ball.obj");

	


	//KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].dogPosX = 0;
		KeyFrame[i].dogPosY = 0;
		KeyFrame[i].dogPosZ = 0;
		KeyFrame[i].incX = 0;
		KeyFrame[i].incY = 0;
		KeyFrame[i].incZ = 0;
		KeyFrame[i].rotDog = 0;
		KeyFrame[i].rotDogInc = 0;
		KeyFrame[i].legFL = 0;   KeyFrame[i].legFLInc = 0;
		KeyFrame[i].legFR = 0;   KeyFrame[i].legFRInc = 0;
		KeyFrame[i].legBL = 0;   KeyFrame[i].legBLInc = 0;
		KeyFrame[i].legBR = 0;   KeyFrame[i].legBRInc = 0;
		KeyFrame[i].tail = 0;   KeyFrame[i].tailInc = 0;
	}

	LoadKeyframesFromFile(KEYFRAMES_PATH);

	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO,EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	
	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		Animation_dog();

		humanAnimator.UpdateAnimation(deltaTime);

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	   
		// OpenGL options
		glEnable(GL_DEPTH_TEST);

		
		glm::mat4 modelTemp = glm::mat4(1.0f); //Temp
		
	

		// Use cooresponding shader when setting uniforms/drawing objects
		lightingShader.Use();

        glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "specular"),1);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);


		// Directional light
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"),0.6f,0.6f,0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"),0.3f, 0.3f, 0.3f);


		// Point light 1
	    glm::vec3 lightColor;
		lightColor.x= abs(sin(glfwGetTime() *Light1.x));
		lightColor.y= abs(sin(glfwGetTime() *Light1.y));
		lightColor.z= sin(glfwGetTime() *Light1.z);

		
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), lightColor.x,lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor.x,lightColor.y,lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"),0.075f);


		// SpotLight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));
		

		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model(1);

	
		
		//Carga de modelo 
        view = camera.GetViewMatrix();	
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Piso.Draw(lightingShader);

		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		//Body
		modelTemp = model = glm::rotate(model, glm::radians(rotXDog), glm::vec3(1.0f, 0.0f, 0.0f));
		modelTemp = model = glm::translate(model, glm::vec3(dogPosX, dogPosY, dogPosZ));
		modelTemp = model = glm::rotate(model, glm::radians(rotDog), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		DogBody.Draw(lightingShader);
		//Head
		model = modelTemp;
		model = glm::translate(model, glm::vec3(0.0f, 0.093f, 0.208f));
		model = glm::rotate(model, glm::radians(head), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		HeadDog.Draw(lightingShader);
		//Tail 
		model = modelTemp;
		model = glm::translate(model, glm::vec3(0.0f, 0.026f, -0.288f));
		model = glm::rotate(model, glm::radians(tail), glm::vec3(0.0f, 0.0f, -1.0f)); 
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); 
		DogTail.Draw(lightingShader);
		//Front Left Leg
		model = modelTemp;
		model = glm::translate(model, glm::vec3(0.112f, -0.044f, 0.074f));
		model = glm::rotate(model, glm::radians(legFL), glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		F_LeftLeg.Draw(lightingShader);
		//Front Right Leg
		model = modelTemp; 
		model = glm::translate(model, glm::vec3(-0.111f, -0.055f, 0.074f));
		model = glm::rotate(model, glm::radians(legFR), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		F_RightLeg.Draw(lightingShader);
		//Back Left Leg
		model = modelTemp; 
		model = glm::translate(model, glm::vec3(0.082f, -0.046, -0.218)); 
		model = glm::rotate(model, glm::radians(legBL), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); 
		B_LeftLeg.Draw(lightingShader);
		//Back Right Leg
		model = modelTemp; 
		model = glm::translate(model, glm::vec3(-0.083f, -0.057f, -0.231f));
		model = glm::rotate(model, glm::radians(legBR), glm::vec3(-1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		B_RightLeg.Draw(lightingShader); 


		model = glm::mat4(1);
		glEnable(GL_BLEND);//Avtiva la funcionalidad para trabajar el canal alfa
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		model = glm::rotate(model, glm::radians(rotBall), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	    Ball.Draw(lightingShader); 
		glDisable(GL_BLEND);  //Desactiva el canal alfa 
		glBindVertexArray(0);

		shaderEsqueletico.Use();

		// viewPos (Posición de la cámara)
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		
		// Luz Direccional
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.ambient"),0.6f,0.6f,0.6f);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.specular"),0.3f, 0.3f, 0.3f);

		// Luz Puntual (Point light 1)
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].ambient"), lightColor.x,lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].diffuse"), lightColor.x,lightColor.y,lightColor.z);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].specular"), 1.0f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].quadratic"),0.075f);

		// Linterna (SpotLight)
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));
		
		// Propiedades del Material
		glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "material.shininess"), 5.0f);
		glUniform1i(glGetUniformLocation(shaderEsqueletico.Program, "diffuse"), 0);
		
		
		glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		auto boneMatrices = humanAnimator.GetFinalBoneMatrices();
		for (int i = 0; i < boneMatrices.size(); ++i)
		{
			string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";

			// DEBE SER shaderEsqueletico.Program
			glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(boneMatrices[i]));
		}

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01));
		glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		humanModel.Draw(shaderEsqueletico);
	

		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)
		
		model = glm::mat4(1);
		model = glm::translate(model, pointLightPositions[0]);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		glBindVertexArray(0);

		
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	if (FrameIndex > 1) {
		SaveAllKeyframesToFile(KEYFRAMES_PATH);
	}
	
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	//Dog Controls
	
	// Cabeza (opcional, si quieres alternativas a 4/5)
	if (keys[GLFW_KEY_I]) head += 1.0f;
	if (keys[GLFW_KEY_O]) head -= 1.0f;

	// Pata delantera izquierda
	if (keys[GLFW_KEY_Q]) legFL += 1.0f;
	if (keys[GLFW_KEY_E]) legFL -= 1.0f;

	// Pata delantera derecha (saludo)
	if (keys[GLFW_KEY_R]) legFR += 1.0f;
	if (keys[GLFW_KEY_F]) legFR -= 1.0f;

	// Pata trasera izquierda
	if (keys[GLFW_KEY_V]) legBL += 1.0f;
	if (keys[GLFW_KEY_B]) legBL -= 1.0f;

	// Pata trasera derecha
	if (keys[GLFW_KEY_N]) legBR += 1.0f;
	if (keys[GLFW_KEY_M]) legBR -= 1.0f;

	// Cola
	if (keys[GLFW_KEY_Z]) tail += 1.0f;
	if (keys[GLFW_KEY_X]) tail -= 1.0f;

	if (keys[GLFW_KEY_4])   rotXDog += 1.0f;  
	if (keys[GLFW_KEY_5]) rotXDog -= 1.0f;


	if (keys[GLFW_KEY_2])
	{
		
			rotDog += 1.0f;

	}

	if (keys[GLFW_KEY_3])
	{
		
			rotDog -= 1.0f;

	}
			
	if (keys[GLFW_KEY_H])
	{
		dogPosZ += 0.01;
	}

	if (keys[GLFW_KEY_Y])
	{
		dogPosZ -= 0.01;
	}

	if (keys[GLFW_KEY_G])
	{
		dogPosX -= 0.01;
	}

	if (keys[GLFW_KEY_J])
	{
		dogPosX += 0.01;
	}

	if (keys[GLFW_KEY_2])
	{
		dogPosY -= 0.01;
	}

	if (keys[GLFW_KEY_3])
	{
		dogPosY += 0.01;
	}

	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);


	}

	if (keys[GLFW_KEY_T])
	{
		pointLightPositions[0].x += 0.01f;
	}
	if (keys[GLFW_KEY_G])
	{
		pointLightPositions[0].x -= 0.01f;
	}

	if (keys[GLFW_KEY_Y])
	{
		pointLightPositions[0].y += 0.01f;
	}

	if (keys[GLFW_KEY_H])
	{
		pointLightPositions[0].y -= 0.01f;
	}
	if (keys[GLFW_KEY_U])
	{
		pointLightPositions[0].z -= 0.1f;
	}
	if (keys[GLFW_KEY_J])
	{
		pointLightPositions[0].z += 0.01f;
	}
	
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	// Reproducir animación cargada al presionar X
	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		if (FrameIndex > 1) {
			resetElements();
			play = true;
			playIndex = 0;
			i_curr_steps = 0;
			interpolation();
			std::cout << "[Keyframes] Reproduciendo animación cargada (" << FrameIndex << " frames)\n";
		}
		else {
			std::cout << "[Keyframes] No hay animación cargada o solo 1 frame\n";
		}
	}

	if (keys[GLFW_KEY_L])
	{
		if (play == false && (FrameIndex > 1))
		{

			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
		}

	}

	if (keys[GLFW_KEY_K])
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}

	}



	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
		{
			Light1 = glm::vec3(0.2f, 0.8f, 1.0f);
			
		}
		else
		{
			Light1 = glm::vec3(0);//Cuado es solo un valor en los 3 vectores pueden dejar solo una componente
		}
	}
	
	
}
void Animation_dog() {

	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			dogPosX += KeyFrame[playIndex].incX;
			dogPosY += KeyFrame[playIndex].incY;
			dogPosZ += KeyFrame[playIndex].incZ;
			head += KeyFrame[playIndex].headInc;
			rotDog += KeyFrame[playIndex].rotDogInc;
			legFL += KeyFrame[playIndex].legFLInc;
			legFR += KeyFrame[playIndex].legFRInc;
			legBL += KeyFrame[playIndex].legBLInc;
			legBR += KeyFrame[playIndex].legBRInc;
			tail += KeyFrame[playIndex].tailInc;

			i_curr_steps++;
		}

	}
	
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

