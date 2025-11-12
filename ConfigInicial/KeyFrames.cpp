// Estudiantes: 319056323, 319302149, 422022569
// Proyecto Final
// Fecha de entrega: 12/11/2025

#include <iostream>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>

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

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Animation.h"
#include "Animator.h"

struct Painting {
    glm::vec3 pos;
    std::string titulo;
    std::string autor;
    int anio;
    std::string desc;
};

std::vector<Painting> gPaints;
float gShowRadius = 2.0f;   // radio para considerar “cerca”
int   gCurrentPaint = -1;   // índice de pintura cercana
bool  gShowCard = false;    // TAB alterna si hay pintura cercana

int FindNearestPainting(const glm::vec3& eye, float radius) {
    int best = -1;
    float bestD2 = radius * radius;
    for (int i = 0; i < (int)gPaints.size(); ++i) {
        glm::vec3 v = gPaints[i].pos - eye;
        float d2 = glm::dot(v, v);
        if (d2 <= bestD2) { bestD2 = d2; best = i; }
    }
    return best;
}

void SetupPaintings() {
    gPaints.clear();
    gPaints.push_back({ glm::vec3(0.337259f, 2.33063f,  -4.45026f), "Obra 1: Night Ride", "Mariana Pandele", 2022 ,"Se observan las gotas, como si la imagen fuera en un cristal." });
    gPaints.push_back({ glm::vec3(-5.79022f,  2.09737f,  -8.17831f), "Obra 2: Fishing", "Zac Retz", 2025,"Pintura digital basada en el río Ganges." });
    gPaints.push_back({ glm::vec3(6.55929f,  2.09173f,  -8.17831f), "Obra 3: Sailing", "Anónimo", 2024,"Pintura acrílica, mostrando un barco navegando ante la salida del sol. (O caída, según se mire.)" });
    gPaints.push_back({ glm::vec3(4.62123f,  2.10394f, -11.08750f), "Obra 4: ", "Anónimo", 2022,"" });
    gPaints.push_back({ glm::vec3(4.34043f,  1.63884f, -10.72130f), "Obra 5", "Anónimo", 2024 ,"" });
    gPaints.push_back({ glm::vec3(-5.21808f,  1.80790f, -10.93570f), "Obra 6: El Grito", "Edvard Munch", 1893 ,"Figura andrógina en un estado de angustia existencial" });
    gPaints.push_back({ glm::vec3(-0.313893f, 2.48509f,  -8.50147f), "Obra 7 : Mirada", "Armando Luna", 2025 ,"Pintura digital hecha por 319056323." });
    gPaints.push_back({ glm::vec3(6.06299f,  2.50170f, -10.93570f), "Obra 8: Noche Apagada", "Anónimo", 2023,"Denota la parte central, pues mientras más se sube, menos luz hay." });
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

void Animation_esculturas();
bool SaveAnimation(const char* path);
bool LoadAnimation(const char* path);
void saveFrame(void);
void resetElements(void);
void interpolation(void);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 2.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024]{};
bool firstMouse = true;
// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active = false;

int gActivePoint = 0;        // 0 = sala principal, 1 = cafetería
float gLightMoveStep = 2.5f; // unidades por segundo

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(1.0f,5.5f, -7.5f), // Sala principal 
    glm::vec3(4.0f,5.5f, 4.8f) // Cafetería
};

// Temperaturas aproximadas (RGB) en [0..1]
const glm::vec3 COLD_RGB = glm::vec3(0.80f, 0.87f, 1.00f); // ~6500–7000 K
const glm::vec3 WARM_RGB = glm::vec3(1.00f, 0.78f, 0.55f); // ~3000 K
const glm::vec3 SUN10_RGB = glm::vec3(1.00f, 0.95f, 0.88f); // ~5200–5500 K
const glm::vec3 SPOT_RGB = glm::vec3(0.75f, 0.82f, 1.00f); // fría para spotlight

float vertices[] = {
     -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

       -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,

       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
       -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
};

glm::vec3 Light1 = glm::vec3(0);

// ESTADO DE ESCULTURA 01
float PosX = 0.0f, PosY = 0.0f, PosZ = 0.0f;
float cue = 0.0f;
float Pd = 0.0f;
float Pder = 0.0f;
float Pf = 0.0f;
float Pi = 0.0f;
float Pizq = 0.0f;
float Ptobj = 0.0f;
float rotStem = 0.0f;

#define MAX_FRAMES 200
int i_max_steps = 190;
int i_curr_steps = 0;

typedef struct _frame {
    float PosX, PosY, PosZ;  float incX, incY, incZ;
    float cue, cueInc;
    float Pd, PdInc;
    float Pder, PderInc;
    float Pf, PfInc;
    float Pi, PiInc;
    float Pizq, PizqInc;
    float Ptobj, PtobjInc;
    float rotStem, rotStemInc;
} FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;
bool play = false;
int playIndex = 0;

const char* ANIM_FILE = "frame_anim.txt";
// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool SaveAnimation(const char* path) {
    std::ofstream ofs(path);
    if (!ofs.is_open()) return false;

    ofs << FrameIndex << " " << i_max_steps << "\n";
    for (int i = 0; i < FrameIndex; ++i) {
        const FRAME& k = KeyFrame[i];
        ofs << k.PosX << " " << k.PosY << " " << k.PosZ << " "
            << k.cue << " " << k.Pd << " " << k.Pder << " "
            << k.Pf << " " << k.Pi << " " << k.Pizq << " "
            << k.Ptobj << " " << k.rotStem << "\n";
    }
    return true;
}

bool LoadAnimation(const char* path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;

    int count = 0;
    ifs >> count >> i_max_steps;
    if (!ifs.good()) return false;

    FrameIndex = std::min(count, (int)MAX_FRAMES);

    for (int i = 0; i < FrameIndex; ++i) {
        FRAME k{};
        ifs >> k.PosX >> k.PosY >> k.PosZ
            >> k.cue >> k.Pd >> k.Pder
            >> k.Pf >> k.Pi >> k.Pizq
            >> k.Ptobj >> k.rotStem;
        if (!ifs.good()) { FrameIndex = i; break; }

        k.incX = k.incY = k.incZ = 0.0f;
        k.cueInc = 0.0f;
        k.PizqInc = 0.0f;
        k.PderInc = 0.0f;
        k.PfInc = k.PiInc = k.PdInc = k.PtobjInc = 0.0f;
        k.rotStemInc = 0.0f;

        KeyFrame[i] = k;
    }

    if (FrameIndex > 0) resetElements();
    return true;
}

void saveFrame(void) {
    std::printf("frameindex %d\n", FrameIndex);

    KeyFrame[FrameIndex].PosX = PosX;
    KeyFrame[FrameIndex].PosY = PosY;
    KeyFrame[FrameIndex].PosZ = PosZ;
    KeyFrame[FrameIndex].cue = cue;
    KeyFrame[FrameIndex].Pd = Pd;
    KeyFrame[FrameIndex].Pder = Pder;
    KeyFrame[FrameIndex].Pf = Pf;
    KeyFrame[FrameIndex].Pi = Pi;
    KeyFrame[FrameIndex].Pizq = Pizq;
    KeyFrame[FrameIndex].Ptobj = Ptobj;
    KeyFrame[FrameIndex].rotStem = rotStem;

    FrameIndex = std::min(FrameIndex + 1, MAX_FRAMES);
}

void resetElements(void) {
    if (FrameIndex == 0) return;

    PosX = KeyFrame[0].PosX;
    PosY = KeyFrame[0].PosY;
    PosZ = KeyFrame[0].PosZ;
    cue = KeyFrame[0].cue;
    Pd = KeyFrame[0].Pd;
    Pder = KeyFrame[0].Pder;
    Pf = KeyFrame[0].Pf;
    Pi = KeyFrame[0].Pi;
    Pizq = KeyFrame[0].Pizq;
    Ptobj = KeyFrame[0].Ptobj;
    rotStem = KeyFrame[0].rotStem;
}

void interpolation(void) {
    KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].PosX - KeyFrame[playIndex].PosX) / i_max_steps;
    KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].PosY - KeyFrame[playIndex].PosY) / i_max_steps;
    KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].PosZ - KeyFrame[playIndex].PosZ) / i_max_steps;

    KeyFrame[playIndex].cueInc = (KeyFrame[playIndex + 1].cue - KeyFrame[playIndex].cue) / i_max_steps;
    KeyFrame[playIndex].PdInc = (KeyFrame[playIndex + 1].Pd - KeyFrame[playIndex].Pd) / i_max_steps;
    KeyFrame[playIndex].PderInc = (KeyFrame[playIndex + 1].Pder - KeyFrame[playIndex].Pder) / i_max_steps;
    KeyFrame[playIndex].PfInc = (KeyFrame[playIndex + 1].Pf - KeyFrame[playIndex].Pf) / i_max_steps;
    KeyFrame[playIndex].PiInc = (KeyFrame[playIndex + 1].Pi - KeyFrame[playIndex].Pi) / i_max_steps;
    KeyFrame[playIndex].PizqInc = (KeyFrame[playIndex + 1].Pizq - KeyFrame[playIndex].Pizq) / i_max_steps;
    KeyFrame[playIndex].PtobjInc = (KeyFrame[playIndex + 1].Ptobj - KeyFrame[playIndex].Ptobj) / i_max_steps;
    KeyFrame[playIndex].rotStemInc = (KeyFrame[playIndex + 1].rotStem - KeyFrame[playIndex].rotStem) / i_max_steps;
}

int main() {
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return EXIT_FAILURE;
    }

    // Create a GLFWwindow object
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Armando Luna Juarez", nullptr, nullptr);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Set callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    // GLEW
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Viewport
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Shaders
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader shaderEsqueletico("Shader/lighting_skeletical.vs", "Shader/lighting.frag");

    // Models
    Model Piso((char*)"Models/galeria.obj");

    // Modelos de la escultura
    Model Ecue((char*)"Models/cue.obj");
    Model EPd((char*)"Models/Pd.obj");
    Model EPder((char*)"Models/Pder.obj");
    Model EPf((char*)"Models/Pf.obj");
    Model EPi((char*)"Models/Pi.obj");
    Model EPizq((char*)"Models/Pizq.obj");
    Model EPtobj((char*)"Models/Ptobj.obj");

    // Animación esquelética (humano y vaca)
    Model vaca((char*)"Models/vaca.fbx");
    Animation vacaAnimation((char*)"Models/vaca.fbx", &vaca);
    Animator vacaAnimator(&vacaAnimation);

    Model humanModel((char*)"Models/humano-animation.fbx");
    Animation humanAnimation((char*)"Models/humano-animation.fbx", &humanModel);
    Animator humanAnimator(&humanAnimation);

    // Inicializa keyframes
    for (int i = 0; i < MAX_FRAMES; i++) {
        KeyFrame[i].PosX = 0;  KeyFrame[i].PosY = 0;  KeyFrame[i].PosZ = 0;
        KeyFrame[i].incX = 0;  KeyFrame[i].incY = 0;  KeyFrame[i].incZ = 0;
        KeyFrame[i].cue = 0;   KeyFrame[i].cueInc = 0;
        KeyFrame[i].Pd = 0;    KeyFrame[i].PdInc = 0;
        KeyFrame[i].Pder = 0;  KeyFrame[i].PderInc = 0;
        KeyFrame[i].Pf = 0;    KeyFrame[i].PfInc = 0;
        KeyFrame[i].Pi = 0;    KeyFrame[i].PiInc = 0;
        KeyFrame[i].Pizq = 0;  KeyFrame[i].PizqInc = 0;
        KeyFrame[i].Ptobj = 0; KeyFrame[i].PtobjInc = 0;
        KeyFrame[i].rotStem = 0; KeyFrame[i].rotStemInc = 0;
    }

    if (LoadAnimation(ANIM_FILE)) {
        std::cout << "Animacion cargada desde " << ANIM_FILE << " con " << FrameIndex << " frames.\n";
    }
    else {
        std::cout << "No se encontro animacion previa. Empezando en blanco.\n";
    }

    // Precarga de pinturas
    SetupPaintings();

    // VAO/VBO simple (lamparita demo)
    GLuint VBO = 0, VAO = 0;
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

    // Material samplers (nombres típicos)
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

    // OpenGL options
    glEnable(GL_DEPTH_TEST);

    // Game loop
    while (!glfwWindowShouldClose(window)) {
        // Deltatime
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        glfwPollEvents();
        DoMovement();
        Animation_esculturas();

        // Clear
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Shaders activos
        lightingShader.Use();

        // Update animaciones por huesos
        vacaAnimator.UpdateAnimation(deltaTime);
        humanAnimator.UpdateAnimation(deltaTime);

        glm::mat4 view = camera.GetViewMatrix();
        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f)));

        // Iluminación (igual que tu versión, compactado)
        GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
        glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // Luz direccional = sol 10am (ligeramente cálida, dirección oblicua)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.3f, -1.0f, -0.2f);              
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.40f * SUN10_RGB.r, 0.40f * SUN10_RGB.g, 0.40f * SUN10_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 1.00f * SUN10_RGB.r, 1.00f * SUN10_RGB.g, 1.00f * SUN10_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.30f, 0.30f, 0.30f);


        glm::vec3 lightColor;
        lightColor.x = fabsf(sinf(currentFrame * Light1.x));
        lightColor.y = fabsf(sinf(currentFrame * Light1.y));
        lightColor.z = sinf(currentFrame * Light1.z);

        // -------- pointLight[0] : Sala principal (fría) -----------
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.12f * COLD_RGB.r, 0.12f * COLD_RGB.g, 0.12f * COLD_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.85f * COLD_RGB.r, 0.85f * COLD_RGB.g, 0.85f * COLD_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.30f, 0.35f, 0.40f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.020f);

        // -------- pointLight[1] : Cafetería (cálida) ---------------
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), 0.08f * WARM_RGB.r, 0.08f * WARM_RGB.g, 0.08f * WARM_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), 0.90f * WARM_RGB.r, 0.90f * WARM_RGB.g, 0.90f * WARM_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), 0.35f, 0.28f, 0.20f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.020f);


        // Spotlight desde la cámara (frío y ÁNGULO MAYOR)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);

        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.03f * SPOT_RGB.r, 0.03f * SPOT_RGB.g, 0.03f * SPOT_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.85f * SPOT_RGB.r, 0.85f * SPOT_RGB.g, 0.85f * SPOT_RGB.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.25f, 0.28f, 0.35f);

        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.20f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.20f);

        // Ángulos más grandes (antes 12/18). Ahora 22° interno y 30° externo aprox.
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(22.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(30.0f)));


        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 8.0f);

        // Piso + escultura compuesta
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Piso.Draw(lightingShader);

        glm::mat4 base = glm::mat4(1.0f);
        base = glm::translate(base, glm::vec3(-7.5f, 1.02f, -13.0f));
        base = glm::rotate(base, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        base = glm::scale(base, glm::vec3(0.18f));
        base = glm::rotate(base, glm::radians(rotStem), glm::vec3(0.0f, 1.0f, 0.0f));
        base = glm::translate(base, glm::vec3(PosX, PosY, PosZ));

        model = base; model = glm::rotate(model, glm::radians(cue), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));  Ecue.Draw(lightingShader);

        model = base; model = glm::rotate(model, glm::radians(Pd), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));  EPd.Draw(lightingShader);

        model = base; model = glm::rotate(model, glm::radians(Pder), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));  EPder.Draw(lightingShader);

        model = base; model = glm::rotate(model, glm::radians(Pf), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));  EPf.Draw(lightingShader);

        model = base; model = glm::rotate(model, glm::radians(Pi), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));  EPi.Draw(lightingShader);

        model = base; model = glm::rotate(model, glm::radians(Pizq), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));  EPizq.Draw(lightingShader);

        model = base; model = glm::rotate(model, glm::radians(Ptobj), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));  EPtobj.Draw(lightingShader);

        // Lámpara demo (cubito)
        // Lámparas demo (cubitos para ver dónde están las point lights)
        lampShader.Use();
        GLint modelLocLamp = glGetUniformLocation(lampShader.Program, "model");
        GLint viewLocLamp = glGetUniformLocation(lampShader.Program, "view");
        GLint projLocLamp = glGetUniformLocation(lampShader.Program, "projection");
        glUniformMatrix4fv(viewLocLamp, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLocLamp, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        for (int i = 0; i < 2; ++i) {                // dibuja [0] y [1]
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, pointLightPositions[i]);
            m = glm::scale(m, glm::vec3(0.05f));
            glUniformMatrix4fv(modelLocLamp, 1, GL_FALSE, glm::value_ptr(m));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);


        // shader para animación por huesos
        shaderEsqueletico.Use();
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "viewPos"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // --- Sol 10am (dirLight) ---
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.direction"), -0.3f, -1.0f, -0.2f);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.ambient"),
            0.20f * SUN10_RGB.r, 0.20f * SUN10_RGB.g, 0.20f * SUN10_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.diffuse"),
            0.80f * SUN10_RGB.r, 0.80f * SUN10_RGB.g, 0.80f * SUN10_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "dirLight.specular"), 0.30f, 0.30f, 0.30f);

        // --- pointLight[0] (fría) ---
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].position"),
            pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].ambient"),
            0.12f * COLD_RGB.r, 0.12f * COLD_RGB.g, 0.12f * COLD_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].diffuse"),
            0.85f * COLD_RGB.r, 0.85f * COLD_RGB.g, 0.85f * COLD_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].specular"), 0.30f, 0.35f, 0.40f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[0].quadratic"), 0.020f);

        // --- pointLight[1] (cálida, cafetería) ---
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[1].position"),
            pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[1].ambient"),
            0.08f * WARM_RGB.r, 0.08f * WARM_RGB.g, 0.08f * WARM_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[1].diffuse"),
            0.90f * WARM_RGB.r, 0.90f * WARM_RGB.g, 0.90f * WARM_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[1].specular"), 0.35f, 0.28f, 0.20f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[1].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[1].linear"), 0.045f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "pointLights[1].quadratic"), 0.020f);

        // --- spotlight frío y abierto ---
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.position"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.direction"),
            camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.ambient"),
            0.03f * SPOT_RGB.r, 0.03f * SPOT_RGB.g, 0.03f * SPOT_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.diffuse"),
            0.85f * SPOT_RGB.r, 0.85f * SPOT_RGB.g, 0.85f * SPOT_RGB.b);
        glUniform3f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.specular"),
            0.25f, 0.28f, 0.35f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.linear"), 0.20f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.quadratic"), 0.20f);
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.cutOff"),
            glm::cos(glm::radians(22.0f)));
        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "spotLight.outerCutOff"),
            glm::cos(glm::radians(30.0f)));


        glUniform1f(glGetUniformLocation(shaderEsqueletico.Program, "material.shininess"), 5.0f);
        glUniform1i(glGetUniformLocation(shaderEsqueletico.Program, "diffuse"), 0);

        glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        {
            auto boneMatrix = vacaAnimator.GetFinalBoneMatrices();
            for (size_t i = 0; i < boneMatrix.size(); ++i) {
                std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
                glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(boneMatrix[i]));
            }
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(8.0f, 1.0f, 3.42f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.01f));
            glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            vaca.Draw(shaderEsqueletico);
        }

        {
            auto boneMatrices = humanAnimator.GetFinalBoneMatrices();
            for (size_t i = 0; i < boneMatrices.size(); ++i) {
                std::string uniformName = "finalBoneMatrices[" + std::to_string(i) + "]";
                glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, uniformName.c_str()), 1, GL_FALSE, glm::value_ptr(boneMatrices[i]));
            }
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.01f));
            glUniformMatrix4fv(glGetUniformLocation(shaderEsqueletico.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            humanModel.Draw(shaderEsqueletico);
        }

        // Actualiza pintura cercana (si te alejas, se cierra)
        {
            int nearIdx = FindNearestPainting(camera.GetPosition(), gShowRadius);
            if (nearIdx < 0) { gCurrentPaint = -1; gShowCard = false; }
            else              gCurrentPaint = nearIdx;
        }

        // --- UI ImGui ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Animación de entrada
        static float alpha = 0.0f, slide = 20.0f;
        float dt = ImGui::GetIO().DeltaTime;
        float aTarget = (gShowCard && gCurrentPaint >= 0) ? 1.0f : 0.0f;
        float sTarget = (gShowCard && gCurrentPaint >= 0) ? 0.0f : 20.0f;
        auto damp = [](float c, float t, float spd, float dt2) { return c + (t - c) * (1.0f - std::exp(-spd * dt2)); };
        alpha = damp(alpha, aTarget, 10.0f, dt);
        slide = damp(slide, sTarget, 10.0f, dt);

        if (alpha > 0.01f && gCurrentPaint >= 0) {
            const Painting& P = gPaints[gCurrentPaint];
            ImGui::SetNextWindowBgAlpha(0.92f * alpha);
            ImGui::SetNextWindowPos(ImVec2(20, 20 + slide), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(360, 0), ImGuiCond_Always);

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 12));
            if (ImGui::Begin("##card_info", nullptr, flags)) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

                // Header rojo
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(230, 50, 50, (int)(0.85f * 255)));
                ImGui::BeginChild("hdr", ImVec2(0, 28), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                ImGui::TextUnformatted("Tarjeta de pintura");
                ImGui::EndChild();
                ImGui::PopStyleColor();

                ImGui::Separator();
                ImGui::Text("Titulo: %s", P.titulo.c_str());
                ImGui::Text("Autor : %s", P.autor.c_str());
                ImGui::Text("Año  : %d", P.anio); // <-- corregido
                ImGui::Spacing();
                ImGui::TextWrapped("%s", P.desc.c_str());
                ImGui::Spacing();

                if (ImGui::Button("Ocultar (TAB)")) gShowCard = false;

                ImGui::PopStyleVar(); // Alpha
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
        }

        // Render ImGui encima
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap
        glfwSwapBuffers(window);
    }

    // Shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Terminate GLFW
    glfwTerminate();
    return 0;
}

//Animación
void DoMovement() {
    if (keys[GLFW_KEY_W])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])    camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])    camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])    camera.ProcessKeyboard(RIGHT, deltaTime);
    // Mover la luz puntual activa (IJKL/U/O)
    float step = gLightMoveStep * deltaTime;
    if (keys[GLFW_KEY_L]) pointLightPositions[gActivePoint].x += step; // +X
    if (keys[GLFW_KEY_J]) pointLightPositions[gActivePoint].x -= step; // -X
    if (keys[GLFW_KEY_I]) pointLightPositions[gActivePoint].z -= step; // -Z (hacia adelante)
    if (keys[GLFW_KEY_K]) pointLightPositions[gActivePoint].z += step; // +Z (hacia atrás)
    if (keys[GLFW_KEY_U]) pointLightPositions[gActivePoint].y += step; // +Y (subir)
    if (keys[GLFW_KEY_O]) pointLightPositions[gActivePoint].y -= step; // -Y (bajar)

}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) gActivePoint = 0;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) gActivePoint = 1;


    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)      keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    // Reproducir / detener
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        if (!play && (FrameIndex > 1)) {
            resetElements();
            interpolation();
            play = true;
            playIndex = 0;
            i_curr_steps = 0;
        }
        else {
            play = false;
        }
    }

    // --- TAB: alterna tarjeta solo si hay pintura cercana ---
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        gCurrentPaint = FindNearestPainting(camera.GetPosition(), gShowRadius);
        if (gCurrentPaint >= 0) gShowCard = !gShowCard; else gShowCard = false;
    }

    // Guardar / Cargar animación rápido
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        if (SaveAnimation(ANIM_FILE)) std::cout << "Animación guardada.\n";
        else std::cout << "No se pudo guardar animación.\n";
    }
    if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
        if (LoadAnimation(ANIM_FILE)) std::cout << "Animación cargada.\n";
        else std::cout << "No se pudo cargar animación.\n";
    }
}

void Animation_esculturas() {
    if (!play) return;

    if (i_curr_steps >= i_max_steps) {
        playIndex++;
        if (playIndex > FrameIndex - 2) {
            std::printf("termina anim\n");
            playIndex = 0;
            play = false;
        }
        else {
            i_curr_steps = 0;
            interpolation();
        }
    }
    else {
        PosX += KeyFrame[playIndex].incX;
        PosY += KeyFrame[playIndex].incY;
        PosZ += KeyFrame[playIndex].incZ;

        cue += KeyFrame[playIndex].cueInc;
        Pd += KeyFrame[playIndex].PdInc;
        Pder += KeyFrame[playIndex].PderInc;
        Pf += KeyFrame[playIndex].PfInc;
        Pi += KeyFrame[playIndex].PiInc;
        Pizq += KeyFrame[playIndex].PizqInc;
        Ptobj += KeyFrame[playIndex].PtobjInc;
        rotStem += KeyFrame[playIndex].rotStemInc;

        i_curr_steps++;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = (GLfloat)xPos;
        lastY = (GLfloat)yPos;
        firstMouse = false;
    }
    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;
    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}
