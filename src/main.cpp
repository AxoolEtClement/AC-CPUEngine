#include <GLFW/glfw3.h>

int main() {
    int data[250][250];
    for( size_t y = 0; y < 250; ++y )
    {
        for( size_t x = 0; x < 250; ++x )
        {
            data[y][x] = 0xFF000000 | (x * 255 / 250) << 16 | (y * 255 / 250) << 8;
        }
    }
    // Initialize GLFW
    if (!glfwInit()) {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Render here
        glDrawPixels( 250, 250, GL_RGBA, GL_UNSIGNED_BYTE, data );

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}