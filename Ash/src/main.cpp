#include "pch.h"
#include <SDL.h>
#include <GL/glew.h> // extension library that allows us to use the latest OpenGL functionality
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"

bool RUNNING = true;

// Temp input variables
bool forward = false;
bool backward = false;
bool left = false;
bool right = false;


// CAMERA
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// TIME
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

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

void handleKeyboardInput(SDL_Event* event) {
    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE:
            RUNNING = false;
            break;
        case SDLK_w:
            // TEMP
   //         camera.ProcessKeyboard(Movement::FORWARD, deltaTime);
            forward = true;
            break;
        case SDLK_a:
   //         camera.ProcessKeyboard(Movement::LEFT, deltaTime);
            left = true;
            break;
        case SDLK_s:
   //         camera.ProcessKeyboard(Movement::BACKWARD, deltaTime);
            backward = true;
            break;
        case SDLK_d:
   //         camera.ProcessKeyboard(Movement::RIGHT, deltaTime);
            right = true;
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event->key.keysym.sym) {
        case SDLK_w:
            forward = false;
            break;
        case SDLK_a:
            left = false;
            break;
        case SDLK_s:
            backward = false;
            break;
        case SDLK_d:
            right = false;
            break;
        }
    }
}

void handleMouseInput(SDL_Event* event) {
    switch (event->type) {
    case SDL_MOUSEMOTION:
        camera.ProcessMouseMovement(event->motion.xrel, -event->motion.yrel);
        break;
    case SDL_MOUSEBUTTONDOWN:
        break;
    case SDL_MOUSEBUTTONUP:
        break;
    case SDL_MOUSEWHEEL:
        camera.ProcessMouseScroll(static_cast<float>(event->wheel.y));
        break;
    }
}

void handleInput(SDL_Event* event) {
    switch (event->type) {
    case SDL_QUIT:
        RUNNING = false;
        break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        handleKeyboardInput(event);
        break;
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEWHEEL:
        handleMouseInput(event);
        break;
    }
}

int main(int argc, char* argv[]) {
    // Initialize logging
    Dusty::Log::init();

    // Initialize SDL2
    SDL_Window* window;
    SDL_GLContext context;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        SDLKill("Unable to initialize SDL");

    // Request opengl context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_SetRelativeMouseMode(SDL_TRUE);

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

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    // SHADERS
    Shader shader("src/shaders/shader.vert", "src/shaders/shader.frag");

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // initialize buffers
        // VBO: Vertex Buffer Object, VAO: Vertex Array Object, EBO
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the VAO first, then bind and set vertex buffer(s), and then configure vertex attribute(s)
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // light
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already contains the data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // don't forget to use the corresponding shader program first (to set the uniform)
    shader.use();
    shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

    
    // RENDERING LOOP
    SDL_Event e;
    while (RUNNING) {
        //process deltaTime (we calculate it in seconds instead of milliseconds)
        float currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // process input
        while (SDL_PollEvent(&e))
            handleInput(&e);

        // movement vector
        glm::vec3 horizontal = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 backForth = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 movement;

        if (forward)
            backForth += camera.Front;
        else if (backward)
            backForth -= camera.Front;
        if (left)
            horizontal -= camera.Right;
        else if (right)
            horizontal += camera.Right;

        // normalize result if both vectors are greater than 0
        if (glm::length(backForth) > 0.0f && glm::length(horizontal) > 0.0f)
            movement = glm::normalize(backForth + horizontal);
        else
            movement = backForth + horizontal;
        
        camera.move(movement, deltaTime);

        // render
            // clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // activate shader
        shader.use();

        // projection
        // since the projection matrix rarely changes we set it before the loop
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        // sent transformation matrices to shader uniforms
        shader.setMat4("view", view);

        // render crates
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);
            
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        SDL_GL_SwapWindow(window);
    }

    // Delete our opengl context, destroy our window, and shutdown SDL
    SDLCleanup(context, window);

    return 0;
}