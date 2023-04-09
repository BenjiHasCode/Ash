#include "pch.h"
#include <SDL.h>
#include <GL/glew.h> // extension library that allows us to use the latest OpenGL functionality
// #include <SDL_opengl.h>
// #include <gl/GLU.h>
// #include <GL/GL.h>
#include <stdio.h>
#include "Shader.h"

//const int SCREEN_WIDTH = 800;
//const int SCREEN_HEIGHT = 600;

void SDLKill(const char* msg) {
    DUSTY_CORE_ERROR(msg);
    DUSTY_CORE_ERROR(SDL_GetError());
    SDL_Quit();
    exit(1);    // a bit abrupt
}

void checkSDLError(int line = -1) {
#ifndef DUSTY_RELEASE   // if release is not defined then print error messages
    const char* error = SDL_GetError();

    if (*error != '\0') {
        DUSTY_CORE_ERROR("SDL ERROR: ", error);
        if (line != -1) {
            DUSTY_CORE_ERROR(" + line: ", line);
        }
        SDL_ClearError();
    }
#endif
}

void SDLCleanup(SDL_GLContext context, SDL_Window* window) {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// TODO find where to use this in SDL2
void framebufferSizeCallback(SDL_Window* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(int argc, char* argv[]) {
    // Initialize logging
    Dusty::Log::init();


    // Initialize SDL2
    SDL_Window* window;
    SDL_GLContext context; // Why is this not a pointer???

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        SDLKill("Unable to initialize SDL");

        // Request opengl 3.3 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

        // base window size on systems current resolution instead of hardcoded value
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    const auto ASPECT_RATIO = 4.0 / 3.0;
    const auto SCREEN_HEIGHT = DM.h * 0.8;
    const auto SCREEN_WIDTH = SCREEN_HEIGHT * ASPECT_RATIO;

        // Create window centered at specified resolution
    window = SDL_CreateWindow(
        "Ash rendering",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!window) // kill if creation failed
        SDLKill("Unable to create window");

    checkSDLError(__LINE__);

        // Create our opengl context and attach it to our window
    context = SDL_GL_CreateContext(window);
    checkSDLError(__LINE__);

        // This makes our buffer swap syncronized with the monitor's vertical refresh
    SDL_GL_SetSwapInterval(1);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // to get the latest features
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        // DUSTY_CORE_ERROR("Error initializing glew: %s\n", glewGetErrorString(glewError));
        printf("Error initializing GLEW: %s\n", glewGetErrorString(glewError));
        return -1;
    }

    // viewport size
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // quite uncertain what this does as reducing is doesn't clarify its effect on clear color


    // SHADERS
    Shader shader("src/shaders/shader.vert", "src/shaders/shader.frag");

    // Vertices
    float vertices[] = {
         // positions       // colors
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f  // top
    };

    // initialize buffers
        // VBO: Vertex Buffer Object , VAO: Vertex Array Object
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
        // bind the VAO first, then bind and set vertex buffer(s), and then configure vertex attribute(s)
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
        // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // rendering loop
    SDL_Event e;
    bool running = true;
    while (running) {
        // process input
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                running = false;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                }
            }
        }

        // render
            // clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
            // activate shader program
        shader.use();
        shader.setFloat("someUniform", 1.0f);
            // render triangle
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        // swap
        SDL_GL_SwapWindow(window);
        //glfwPollEvents();
        //SDL_Delay(2000);
    }

    // Delete our opengl context, destroy our window, and shutdown SDL
    SDLCleanup(context, window);

    return 0;
}