//
//
//
#include <iostream>
#include <vector>
#include <array>

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Shaders
#include "Shader.h"

void resize(GLFWwindow* window, int width, int height);

const GLint WIDTH = 800, HEIGHT = 600;

static const std::array<std::array<float, 3>, 5> COLOR = { {
    {0.45f, 0.25f, 0.05f}, // 0: café oscuro (rama / bordes)
    {0.70f, 0.45f, 0.10f}, // 1: dorado oscuro
    {0.90f, 0.65f, 0.15f}, // 2: dorado medio
    {1.00f, 0.85f, 0.35f}, // 3: dorado claro
    {1.00f, 0.95f, 0.60f}  // 4: dorado brillante (brillo superior)
} };

static const int W = 12; // columnas
static const int H = 14; // filas

static const int CREEPER[H][W] = {
 {-1,-1,-1,-1,-1,-1,-1,0,-1,-1,-1,-1},
 {-1,-1,-1,-1,-1,-1,1,0,-1,-1,-1,-1},
 {-1,-1,-1,-1,-1,-1,0,-1,-1,-1,-1,-1},
 {-1,-1,-1,1,1,0,0,0,0,-1,-1,-1},
 {-1,1,2,4,2,0,1,2,1,0,0,-1},
 {1,2,3,3,4,4,4,3,2,1,0,0},
 {1,2,3,3,3,3,3,3,2,3,1,0},
 {1,2,2,3,1,3,1,1,1,3,2,0},
 {0,1,2,2,1,1,2,1,1,3,2,0},
 {0,2,2,2,2,1,2,2,2,3,2,0},
 {-1,0,2,2,3,2,2,2,3,2,0,-1},
 {-1,0,1,2,2,2,2,2,2,1,0,-1},
 {-1,-1,0,1,2,1,1,2,1,0,-1,-1},
 {-1,-1,-1,0,0,0,0,0,0,-1,-1,-1},
};

static void buildPixelArt(std::vector<float>& vertices, std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    const float margin = 0.1f;          // margen en NDC
    float aspect = (float)W / (float)H; // ancho/alto de la rejilla
    float HNDC = 1.0f - margin;         // "half-height" útil
    float WNDC = HNDC * aspect;         // "half-width" para mantener aspecto

    float cellW = (2.0f * WNDC) / W;    // ancho celda en NDC
    float cellH = (2.0f * HNDC) / H;    // alto  celda en NDC

    unsigned int base = 0;

    for (int r = 0; r < H; ++r)
    {
        for (int c = 0; c < W; ++c)
        {
            int idx = CREEPER[r][c];
            if (idx < 0) continue; // transparente

            auto col = COLOR[idx];

            // Esquina superior-izquierda de la celda en NDC
            float x0 = -WNDC + c * cellW;
            float y0 = HNDC - r * cellH;
            float x1 = x0 + cellW;
            float y1 = y0 - cellH;
            const float z = 0.0f;

            // 4 vértices (x,y,z,r,g,b) en sentido TL, BL, BR, TR
            vertices.insert(vertices.end(), { x0, y0, z, col[0], col[1], col[2] });
            vertices.insert(vertices.end(), { x0, y1, z, col[0], col[1], col[2] });
            vertices.insert(vertices.end(), { x1, y1, z, col[0], col[1], col[2] });
            vertices.insert(vertices.end(), { x1, y0, z, col[0], col[1], col[2] });

            // 2 triángulos (0,1,2) y (0,2,3) relativos a la celda
            indices.insert(indices.end(), { base + 0, base + 1, base + 2, base + 0, base + 2, base + 3 });

            base += 4;
        }
    }
}

int main() {
    glfwInit();

    // (Tus hints opcionales se quedan comentados)
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Dibujo de Primitivas en 2D", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, resize);

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialise GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // (Opcional) Fijar viewport inicial por si el callback aún no se dispara
    resize(window, WIDTH, HEIGHT);

    // Info de OpenGL
    std::cout << "> Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "> Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "> Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "> SL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    Shader ourShader("Shader/core.vs", "Shader/core.frag");

    // =================== Construcción de la geometría ===================
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    buildPixelArt(vertices, indices);

    // =================== Buffers y atributos ===================
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vértices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Atributos (igual que tu código original)
    // Posición: location 0, 3 floats, stride 6, offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color: location 1, 3 floats, stride 6, offset 3 floats
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // =================== Loop de render ===================
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.Use();
        glBindVertexArray(VAO);

        // Importante: dibujamos TODOS los triángulos usando el EBO
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

void resize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}