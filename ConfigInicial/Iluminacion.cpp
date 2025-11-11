// Alumno: Armando Luna Juarez 319056323
// Fecha: 12/10/2025
// Práctica 8: Iluminacion de un objeto 3D con luz direccional, puntual y de foco

#include <string>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "SOIL2/SOIL2.h"
#include "stb_image.h"
// Properties
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();


// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

float angDia = 0.0f;                         // ángulo de la luz día (rad)
float angNoche = glm::radians(180.0f);         // la noche arranca opuesta a la día

float diaV = 0.25f;                        // radianes/seg (ajustable con teclas)
float nocheV = 0.25f;

float diaR = 3.5f;                        // radio de la órbita (X-Z)
float nocheR = 3.5f;

float verticalScale = 1.0f;                      // cuánto sube/baja en Y (hemisferio)
glm::vec3 orbitCenter(0.0f, 0.0f, 0.0f);         // centro de la órbita

// Light attributes
glm::vec3 lightPos(0.5f, 0.5f, 2.5f);
float movelightPos = 0.0f;
glm::vec3 lightPos2(-1.5f, 1.0f, 1.0f);
float movelightPos2 = 0.0f;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
float rot = 0.0f;
bool activanim = false;
bool dia = true;
float tiltNightDeg = 90.0f;                 // rotación del plano de la órbita nocturna (°). 90 = transversal
float nightRadiusFactor = 0.5f;             // “a mitad” del radio de la del día
glm::vec3 tiltAxis = glm::vec3(1.0f, 0.0f, 0.0f); // eje de rotación del plano (X). Cambia a (0,0,1) si prefieres


int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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
    //glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

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

    // OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Setup and compile our shaders
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader lampshader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");



    // Load models
    Model red_dog((char*)"Models/RedDog.obj");
	Model fantasma((char*)"Models/fantasma.obj");
	Model playa((char*)"Models/mar.obj");
	Model buggy((char*)"Models/buggy.obj");
	Model lentes((char*)"Models/lentes.obj");
	Model sombrero((char*)"Models/sombrero.obj");
	Model luna((char*)"Models/moon.obj");
	Model sol((char*)"Models/sunmap.obj");
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

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

    // First, set the container's VAO (and VBO)
    GLuint VBO, VAO;
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

    // Load textures

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    int textureWidth, textureHeight, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    image = stbi_load("Models/Texture_albedo.jpg", &textureWidth, &textureHeight, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    if (image)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(image);


    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        angDia = fmod(angDia + diaV * deltaTime, glm::two_pi<float>());
        angNoche = fmod(angNoche + nocheV * deltaTime, glm::two_pi<float>());

        // Check and call events
        glfwPollEvents();
        DoMovement();

        // Clear the colorbuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // >>> REEMPLAZA EL BLOQUE DE LUCES EN EL RENDER LOOP (ANTES DE DIBUJAR MODELOS)
        lightingShader.Use();

        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // Usaremos solo 1 luz activa (la "día" o la "noche")
        glUniform1i(glGetUniformLocation(lightingShader.Program, "numLights"), 1);

        glm::vec3 L1 = orbitCenter + glm::vec3(
            diaR * cos(angDia),
            verticalScale * diaR * sin(angDia),   // altura (Y)
            diaR * sin(angDia)
        );

        glm::mat4 Rnight = glm::rotate(glm::mat4(1.0f), glm::radians(tiltNightDeg), glm::normalize(tiltAxis));

        // vector base en el mismo formato que L1 (antes de rotar el plano)
        glm::vec4 vNight(
            (nocheR* nightRadiusFactor)* cos(angNoche),
            verticalScale* (nocheR* nightRadiusFactor)* sin(angNoche),
            (nocheR* nightRadiusFactor)* sin(angNoche),
            1.0f
        );

        // rotamos ese vector para “inclinar” el plano de la órbita
        vNight = Rnight * vNight;

        glm::vec3 L2 = orbitCenter + glm::vec3(vNight);

        if (dia) {
            // LUZ DÍA: blanca más intensa (conserva “el color” original pero sube potencia)
            glUniform3fv(glGetUniformLocation(lightingShader.Program, "lights[0].position"), 1, glm::value_ptr(L1));
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].ambient"), 0.30f, 0.28f, 0.25f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].diffuse"), 1.20f, 1.10f, 1.00f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].specular"), 1.00f, 0.90f, 0.80f);
        }
        else {
            // LUZ NOCHE: azulada
            glUniform3fv(glGetUniformLocation(lightingShader.Program, "lights[0].position"), 1, glm::value_ptr(L2));
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].ambient"), 0.05f, 0.10f, 0.20f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].diffuse"), 0.30f, 0.50f, 1.00f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, "lights[0].specular"), 0.40f, 0.60f, 1.00f);
        }

        // (lo demás de matrices y material se queda igual)
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Material (igual que ya lo tienes)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.2f, 0.2f, 0.1f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);


        // Draw the loaded model
        // --- FANTASMA ---
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.5f));
        model = glm::rotate(model, glm::radians(-85.0f), glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 1.2f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        fantasma.Draw(lightingShader);

        // --- RED_DOG ---
        glm::mat4 model2 = glm::mat4(1.0f);                    
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model2));
        red_dog.Draw(lightingShader);
        
		// --- PLAYA ---
		glm::mat4 model3 = glm::mat4(1.0f);
		model3 = glm::translate(model3, glm::vec3(-1.8f, -1.0f, -5.0));
		model3 = glm::scale(model3, glm::vec3(1.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model3));
		playa.Draw(lightingShader);

		// --- BUGGY ---
        glm::mat4 model4 = glm::mat4(1.0f);
        model4 = glm::translate(model4, glm::vec3(-0.3f, -0.5f, 0.28f));
        model4 = glm::scale(model4, glm::vec3(0.3f));
        model4 = glm::rotate(model4, glm::radians(60.0f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model4));
        buggy.Draw(lightingShader);

		// --- LENTES ---
		glm::mat4 model5 = glm::mat4(1.0f);
		model5 = glm::translate(model5, glm::vec3(0.0f, 0.2f, 0.28f));
		model5 = glm::scale(model5, glm::vec3(2.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model5));
		lentes.Draw(lightingShader);

		// --- SOMBRERO ---
		glm::mat4 model6 = glm::mat4(1.0f);
		model6 = glm::translate(model6, glm::vec3(0.0f, 0.22f, 0.28f));
		model6 = glm::scale(model6, glm::vec3(0.3f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model6));
		sombrero.Draw(lightingShader);
        
        glBindVertexArray(0);



        lampshader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 lampM = glm::mat4(1.0f);

        // Dibuja SOLO la lámpara correspondiente al modo activo
        if (dia) {
            lampM = glm::translate(glm::mat4(1.0f), L1);
            lampM = glm::scale(lampM, glm::vec3(0.2f));  // ajusta escala a tu .obj
            glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampM));
            sol.Draw(lampshader);
        }
        else {
            lampM = glm::translate(glm::mat4(1.0f), L2);
            lampM = glm::scale(lampM, glm::vec3(0.2f));  // ajusta escala a tu .obj
            glUniformMatrix4fv(glGetUniformLocation(lampshader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampM));
            luna.Draw(lampshader);
        }


        glBindVertexArray(0);

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}


// Moves/alters the camera positions based on user input
void DoMovement()
{
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

    if (activanim)
    {
        if (rot > -90.0f)
            rot -= 0.1f;
    }
    
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
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
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        dia = !dia;
    }
    if (keys[GLFW_KEY_O])
    {
       
        diaV += 0.10f * deltaTime * 2;
    }

    if (keys[GLFW_KEY_L])
    {
        
        diaV = glm::max(0.0f, diaV - 0.10f * deltaTime);
    }
    if (keys[GLFW_KEY_I]) {
        nocheV += 0.10f * deltaTime * 2;
    }
    if (keys[GLFW_KEY_K]) {
        nocheV = glm::max(0.0f, nocheV - 0.10f * deltaTime);
    }


}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
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


