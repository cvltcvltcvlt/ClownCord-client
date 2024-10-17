#include "renderGui.hpp"
#include <thread>
#include <iostream>

GLFWwindow* InitWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "VoIP Application", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    return window;
}

int main() {
    setlocale(LC_ALL, ".1251");
    // Initialize GLFW and create a window
    GLFWwindow* window = InitWindow();
    if (!window) {
        return -1;  // Return on failure
    }

    // Initialize the GUI
    Gui gui;
    gui.SetupImGui(window);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();  // Poll for events

        // Render the GUI
        gui.RenderUI();


        // Swap buffers to display the rendered GUI
        glfwSwapBuffers(window);
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
