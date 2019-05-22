/*
 * == COMPILING ==
 *
 * To compile this file with MinGW on Windows, run the following command:
 *     g++ -std=c++11 -Wall -lopengl32 -lgdi32 -Iinclude
 *
 * == DESCRIPTION ==
 *
 * This example shows how to render a colored square from an angle with a GL buffer, a GL shader and matrices.
 *
 */

#define EXAMPLE_NAME __FILE__

#define APPLICATION_IMPLEMENTATION
#include "include/application.h"
#include "include/glmath.h"
#include "include/glshader.h"
#include "include/glbuffer.h"


static struct {
    glm::mat4 matrix;
    glm::vec3 position;
    ShaderType shader;
    BufferType vertexBuffer;
} State;

bool Startup()
{
    std::cout << EXAMPLE_NAME << " startup()\n";
    
    glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
    
    State.shader.compileDefaultShader();
    
    State.vertexBuffer
        .color(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)).vertex(glm::vec3(-10.0f, -10.0f, 0.0f))   // mint
        .color(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)).vertex(glm::vec3(-10.0f, 10.0f, 0.0f))    // geel
        .color(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)).vertex(glm::vec3(10.0f, 10.0f, 0.0f))     // paars
        .color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)).vertex(glm::vec3(10.0f, -10.0f, 0.0f))    // wit
        .setup(GL_TRIANGLE_FAN, State.shader);
    
    return true;
}

void Resize(int width, int height)
{
    std::cout << EXAMPLE_NAME << " resize()\n";
    glViewport(0, 0, width, height);

    // Calculate the projection and view matrix
    State.matrix = glm::perspective(glm::radians(90.0f), float(width) / float(height), 0.1f, 4096.0f) * glm::lookAt(State.position + glm::vec3(12.0f), State.position, glm::vec3(0.0f, 0.0f, 1.0f));
}

void Destroy()
{
    std::cout << EXAMPLE_NAME << " destroy()\n";
}

bool Tick()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Select shader
    State.shader.use();

    // Upload projection and view matrix into shader
    State.shader.setupMatrices(State.matrix);

    // Render vertex buffer with selected shader
    State.vertexBuffer.render();
    
    return true; // to keep running
}

int main(int argc, char *argv[])
{
    auto app = Application::Create(Startup, Resize, Destroy);

    return app->Run(Tick);
}
