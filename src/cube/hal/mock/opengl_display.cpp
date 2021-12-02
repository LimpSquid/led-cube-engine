#include <cube/hal/mock/opengl_display.h>
#include <cube/core/color.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace cube::core;
using namespace cube::hal;

// Todo: eventually remove
#include <iostream>
namespace
{
    GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };

    GLfloat g_vertex_buffer_data2[] = {
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        0.0f,  -1.0f, 0.0f,
    };

    GLFWwindow* window;
    GLuint vertexbuffer;
    GLuint vertexbuffer2;
    bool toggle;

    void glfw_error_cb (int, const char * desc)
    {
        std::cout << std::string(desc) << "\n";
    }
}

opengl_display::opengl_display()
{
    glfwSetErrorCallback(glfw_error_cb);
    if (!glfwInit()) {
        std::cout << "glfwinit error!\n";
        exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow( 1024, 768, "Tutorial 01", NULL, NULL);
    if (window == NULL)
        exit(1);
    glfwMakeContextCurrent(window); // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "glewinit error!\n";
        exit(1);
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &vertexbuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data2), g_vertex_buffer_data2, GL_STATIC_DRAW);
}

opengl_display::~opengl_display()
{

}

void opengl_display::poll()
{
    glClear( GL_COLOR_BUFFER_BIT );

    // Draw triangle
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, toggle ? vertexbuffer : vertexbuffer2);
    glVertexAttribPointer(
    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        exit(0);
}

void opengl_display::draw_voxel(int x, int y, int z, color const & color)
{
    // Draw a voxel
}

void opengl_display::draw_line(int x1, int y1, int z1, int x2, int y2, int z2, color const & color)
{
    // Draw a voxel
}

void opengl_display::fill(color const & color)
{
    // Fill all the voxels
}

void opengl_display::show()
{
    // Show voxels
    toggle = !toggle;
}
