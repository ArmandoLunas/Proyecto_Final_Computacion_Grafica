// Alumno: Armando Luna Ju�rez 3190566323
// Pr�ctica 04: Modelado Geom�trico
// Fecha de entrega: 07/09/2025

#include <iostream>

// *** IMPORTANTE: NO definir GLEW_STATIC ***
// #define GLEW_STATIC   // <-- NO USAR en este arreglo

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

// ---------- PRAGMAS para MSVC (solo libs necesarias) ----------
#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")     // GLEW din�mico
#pragma comment(lib, "glfw3.lib")      // si usas DLL de GLFW, cambia a glfw3dll.lib
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")
#endif
// --------------------------------------------------------------

void Inputs(GLFWwindow* window);

// Forzar color por atributo constante (sin uniforms)
static constexpr GLint COLOR_ATTR_LOC = 1;
static inline void SetColorConst(const glm::vec3& c) {
    glDisableVertexAttribArray(COLOR_ATTR_LOC);
    glVertexAttrib3f(COLOR_ATTR_LOC, c.x, c.y, c.z);
}

const GLint WIDTH = 800, HEIGHT = 600;
float movX = 0.0f, movY = 0.0f, movZ = -12.0f;
float rot = 0.0f;

int main() {
    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return EXIT_FAILURE; }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Armando Luna Ju�rez", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "Failed to initialise GLEW\n"; return EXIT_FAILURE; }

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);

    Shader ourShader("Shader/core.vs", "Shader/core.frag");

    // Cubo (posiciones)
    float vertices[] = {
        // front
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
         // back
         -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
          0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
          // right
           0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
           0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
           // left
           -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
           -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
           // bottom
           -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
            0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
            // top
            -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
             0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // Paleta
    const glm::vec3 ICE_TOP{ 0.92f, 0.98f, 1.00f };
    const glm::vec3 ICE_DEEP{ 0.70f, 0.88f, 1.00f };
    const glm::vec3 ICE_MID{ 0.25f, 0.55f, 0.90f };
    const glm::vec3 SKY_TEAL{ 0.00f, 0.78f, 0.80f };
    const glm::vec3 P_BLACK{ 0.05f, 0.05f, 0.06f };
    const glm::vec3 P_WHITE{ 1.00f, 1.00f, 1.00f };
    const glm::vec3 P_ORANGE{ 1.00f, 0.72f, 0.18f };

    while (!glfwWindowShouldClose(window)) {
        Inputs(window);
        glfwPollEvents();

        glClearColor(SKY_TEAL.x, SKY_TEAL.y, SKY_TEAL.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.Use();

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(movX, movY, movZ));
        view = glm::rotate(view, glm::radians(rot), glm::vec3(0, 1, 0));

        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glm::mat4 model;
        glm::vec3 islandCenter = { 0.0f, -1.2f , 0.0f };

        // -------- ISLA TOP --------
        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0, 1.0f, 0));
        model = glm::scale(model, glm::vec3(6.0f, 0.9f, 6.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-2, 1.0f, 0));
        model = glm::scale(model, glm::vec3(2.0f, 0.9f, 8.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0.0, 1.0f, -0.3));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(3.0f, 0.9f, 8.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0.1, 1.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.4f, 0.9f, 5.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(2.1, 1.0f, 1.8));
        model = glm::scale(model, glm::vec3(2.0f, 0.9f, 5.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(2.1, 1.6f, 1.8));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-2.1, 1.6f, 0.3));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-3.7, 0.3f, -0.3));
        model = glm::scale(model, glm::vec3(0.3f, 0.5f, 3.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-3.7, 0.0f, -0.3));
        model = glm::scale(model, glm::vec3(0.3f, 0.5f, 0.9f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(3.7, 0.3f, -0.3));
        model = glm::scale(model, glm::vec3(0.3f, 0.5f, 3.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(3.7, 0.0f, -0.3));
        model = glm::scale(model, glm::vec3(0.3f, 0.5f, 0.9f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-2.0, 0.5f, 3.5));
        model = glm::scale(model, glm::vec3(1.8f, 0.5f, 0.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-2.0, 0.1f, 3.5));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(2.0, 0.5f, 3.5));
        model = glm::scale(model, glm::vec3(1.8f, 0.5f, 0.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(2.0, 0.1f, 3.5));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0.0, 0.3f, 3.0));
        model = glm::scale(model, glm::vec3(0.3f, 0.5f, 0.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // -------- ISLA MID --------
        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0.0, 0.1f, 0.0));
        model = glm::scale(model, glm::vec3(5.0f, 0.9f, 5.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-2.5, 0.1f, -0.3));
        model = glm::scale(model, glm::vec3(2.5f, 0.9f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(2.5, 0.1f, -0.3));
        model = glm::scale(model, glm::vec3(2.5f, 0.9f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(2.0, 0.1f, 2.0));
        model = glm::scale(model, glm::vec3(1.5f, 0.9f, 2.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-2.0, 0.1f, 2.0));
        model = glm::scale(model, glm::vec3(1.5f, 0.9f, 2.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-0.5, 0.1f, 2.0));
        model = glm::scale(model, glm::vec3(0.5f, 0.9f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0.5, 0.1f, 2.0));
        model = glm::scale(model, glm::vec3(0.5f, 0.9f, 2.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-1.5, 0.1f, -2.0));
        model = glm::scale(model, glm::vec3(1.5f, 0.9f, 2.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        SetColorConst(ICE_MID);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(1.5, 0.1f, -2.0));
        model = glm::scale(model, glm::vec3(0.9f, 0.9f, 2.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // -------- ISLA DEEP --------
        SetColorConst(ICE_DEEP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0, -0.8f, 0));
        model = glm::scale(model, glm::vec3(4.2f, 1.0f, 4.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glm::vec3 dripPos[] = {
            { 2.2f,-0.8f,1.8f},{ 2.0f,-0.8f,2.3f},{-1.8f,-0.8f, 2.2f}, {-2.3f,-0.8f, 1.0f},{-2.3f,-0.8f,-1.6f},
            { 1.6f,-0.8f,-2.1f},{ 0.0f,-0.8f, 2.3f},{ 2.4f,-0.8f, 0.2f}
        };
        for (const glm::vec3& p : dripPos) {            
            SetColorConst(ICE_DEEP);
            model = glm::mat4(1.0f);
            model = glm::translate(model, islandCenter + p);
            model = glm::scale(model, glm::vec3(0.6f, 1.0f, 0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        SetColorConst(ICE_DEEP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0, -1.8f, 0));
        model = glm::scale(model, glm::vec3(1.3f, 1.2f, 1.3f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // trozos
        SetColorConst(ICE_TOP);
        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0.8f, -3.0f, 1.1f));
        model = glm::rotate(model, glm::radians(12.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.7f, 0.2f, 1.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-2.9f, -3.0f, -0.8f));
        model = glm::rotate(model, glm::radians(-18.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(1.0f, 0.5f, 0.7f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(2.9f, -3.0f, -0.8f));
        model = glm::rotate(model, glm::radians(-18.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(1.0f, 0.5f, 1.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(0.8f, -2.8f, 1.1f));
        model = glm::rotate(model, glm::radians(12.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.8f, 0.5f, 0.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, islandCenter + glm::vec3(-0.5f, -3.0f, -1.0f));
        model = glm::rotate(model, glm::radians(8.0f), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(2.5f, 0.20f, 1.4f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // -------- PING�INO --------
        glm::vec3 base = islandCenter + glm::vec3(0.0f, 1.45f, 0.0f);

        SetColorConst(P_BLACK);  // cuerpo
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.6f, 2.2f, 1.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(P_WHITE);  // panza
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.0f, 0.8f, 0.65f));
        model = glm::scale(model, glm::vec3(1.1f, 1.6f, 0.20f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(P_BLACK);  // cabeza
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.0f, 2.2f, 0.0f));
        model = glm::scale(model, glm::vec3(1.2f, 1.2f, 1.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(P_WHITE);  // ojos
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.35f, 2.35f, 0.6f));
        model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.10f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(-0.35f, 2.35f, 0.6f));
        model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.10f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(P_BLACK);  // pupilas
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.35f, 2.40f, 0.65f));
        model = glm::scale(model, glm::vec3(0.10f, 0.10f, 0.08f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(-0.35f, 2.40f, 0.65f));
        model = glm::scale(model, glm::vec3(0.10f, 0.10f, 0.08f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(P_ORANGE); // pico
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.0f, 2.00f, 0.80f));
        model = glm::scale(model, glm::vec3(0.60f, 0.20f, 0.35f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.0f, 1.90f, 0.95f));
        model = glm::scale(model, glm::vec3(0.40f, 0.18f, 0.25f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(P_BLACK);  // alas
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(1.0f, 1.2f, 0.0f));
        model = glm::rotate(model, glm::radians(8.0f), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.35f, 1.2f, 0.9f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(-1.0f, 1.2f, 0.0f));
        model = glm::rotate(model, glm::radians(-8.0f), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.35f, 1.2f, 0.9f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        SetColorConst(P_ORANGE); // patas
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(0.45f, 0.1f, 0.75f));
        model = glm::scale(model, glm::vec3(0.55f, 0.18f, 0.55f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, base + glm::vec3(-0.45f, 0.1f, 0.75f));
        model = glm::scale(model, glm::vec3(0.55f, 0.18f, 0.55f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return EXIT_SUCCESS;
}

void Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movX += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movX -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) movY += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) movY -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movZ -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movZ += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rot += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rot -= 0.5f;
}