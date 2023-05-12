#ifndef SETUP_WINDOW_H
#define SETUP_WINDOW_H

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLFWwindow* setupWindow(int width, int height, const char* title)
{
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }

    // Set up GLFW window properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return NULL;
    }

    // Make the context of the window the current one
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return NULL;
    }

    // Set up viewport
    glViewport(0, 0, width, height);

    return window;
}

#endif // SETUP_WINDOW_H
