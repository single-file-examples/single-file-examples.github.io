/*
 * == COMPILING ==
 *
 * To compile this file with MinGW on Windows, run the following command:
 *     g++ -std=c++11 -Wall -lopengl32 -lgdi32 -Iinclude
 *
 * == DESCRIPTION ==
 *
 * This example shows how to clear the window with a color.
 *
 */

#define EXAMPLE_NAME __FILE__

#define APPLICATION_IMPLEMENTATION
#include "include/application.h"

bool Startup()
{
    std::cout << EXAMPLE_NAME << " startup() was executes\n";
    return true;
}

void Resize(int width, int height)
{
    std::cout << EXAMPLE_NAME << " resize() was executes\n";
    glViewport(0, 0, width, height);
}

void Destroy()
{
    std::cout << EXAMPLE_NAME << " destroy() was executes\n";
}

bool Tick()
{
    glClearColor(0.0f, 0.4f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    return true; // to keep running
}

int main(int argc, char *argv[])
{
    auto app = Application::Create(Startup, Resize, Destroy);

    return app->Run(Tick);
}
