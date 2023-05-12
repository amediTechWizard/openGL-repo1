#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

// Include GLEW (GL Extension Wrangler) to access OpenGL functions
#define GLEW_STATIC
#include <GL/glew.h>

// Include GLFW (GL Framework) to create a window and handle user input
#include <GLFW/glfw3.h>

// Include GLM (OpenGL Mathematics) for math operations and transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "bmploader.h"
#include "FileReading.h"
#include "setUpWindow.h"

struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texcoord;
};

struct TriData {
    std::vector<int> indices;
};

class TexturedMesh {
public:
    TexturedMesh(const std::string& ply_path, const std::string& bmp_path);
    void draw(const glm::mat4& MVP);
private:
    void setup_mesh();
    std::vector<VertexData> vertices;
    std::vector<TriData> faces;
    GLuint vbo_positions_;
    GLuint vbo_texcoords_;
    GLuint vbo_indices_;
    GLuint texture_;
    GLuint vao_;
    GLuint shader_program_;
};

TexturedMesh::TexturedMesh(const std::string& ply_path, const std::string& bmp_path) {
    unsigned int width, height;
    std::vector<unsigned char> texture_;
    std::vector<VertexData> vertices;
    std::vector<TriData> faces;

        //readPLYFile(ply_path, vertices, faces);
        texture_ = loadARGB_BMP(bmp_path.c_str(), &width, &height);
        setup_mesh();

    // Compile vertex shader
    const char* vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoords;\n"
        "uniform mat4 MVP;\n"
        "out vec2 TexCoords;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = MVP * vec4(aPos, 1.0);\n"
        "    TexCoords = aTexCoords;\n"
        "}\n";
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    // Compile fragment shader
    const char* fragment_shader_source =
        "#version 330 core\n"
        "in vec2 TexCoords;\n"
        "uniform sampler2D uTexture;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(uTexture, TexCoords);\n"
        "}\n";
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    // Create shader program and link shaders
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader);
    glAttachShader(shader_program_, fragment_shader);
    glBindAttribLocation(shader_program_, 0, "aPos");
    glBindAttribLocation(shader_program_, 1, "aTexCoords");
    glLinkProgram(shader_program_);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Create VAO and set up vertex attributes
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_positions_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords_);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(offsetof(VertexData, texcoord)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices_);

    // Set up texture sampler uniform
    GLint texture_uniform = glGetUniformLocation(shader_program_, "uTexture");
    glUseProgram(shader_program_);
    glUniform1i(texture_uniform, 0); // Texture unit 0

    // Unbind VAO
    glBindVertexArray(0);
}


void TexturedMesh::draw(const glm::mat4& MVP) {
    // Bind shader program and VAO
    glUseProgram(shader_program_);
    glBindVertexArray(vao_);

    // Set MVP uniform
    GLuint MVP_id = glGetUniformLocation(shader_program_, "MVP");
    glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &MVP[0][0]);

    // Bind texture and set texture uniform
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    GLuint tex_id = glGetUniformLocation(shader_program_, "texture_sampler");
    glUniform1i(tex_id, 0);

    // Draw triangles using glDrawElements
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices_);
    glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
}

void TexturedMesh::setup_mesh() {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<int> indices;
    for (const auto& v : vertices) {
        positions.push_back(v.position);
        texcoords.push_back(v.texcoord);
    }
    for (const auto& f : faces) {
        indices.insert(indices.end(), f.indices.begin(), f.indices.end());
    }

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Create and bind VBO for vertex positions
    glGenBuffers(1, &vbo_positions_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_positions_);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Create and bind VBO for texture coordinates
    glGenBuffers(1, &vbo_texcoords_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords_);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), &texcoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Create and bind VBO for indices
    glGenBuffers(1, &vbo_indices_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_indices_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    // Unbind VAO
    glBindVertexArray(0);
}

void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp) {
    float cameraSpeed = 0.05f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}



// camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

int main() {
    // setup GLFW window and GLEW
    GLFWwindow* window = setupWindow(800, 600, "My Room"); // Example function that sets up a GLFW window
    if (!window) return -1;

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    // create TexturedMesh objects and setup projection matrix
    TexturedMesh mesh1("LinksHouse/table.ply", "LinksHouse/table.bmp");
    TexturedMesh mesh2("LinksHouse/walls.ply", "LinksHouse/walls.bmp");
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window)) {
        // handle user input to move camera around
        processInput(window, cameraPos, cameraFront, cameraUp);

        // clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // setup model-view-projection matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP = projection * view * model;

        // draw meshes
        mesh1.draw(MVP);
        mesh2.draw(MVP);

        // swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glfwTerminate();
    return 0;
}
