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

//const int SCREEN_WIDTH = 800;
//const int SCREEN_HEIGHT = 600;

bool RUNNING = true;

// Temp input variables
/*bool FORWARD = false;
bool BACKWARD = false;
bool LEFT = false;
bool RIGHT = false;*/


// CAMERA
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX, lastY; //TODO does 0 create bugs?


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
            camera.ProcessKeyboard(Movement::FORWARD, deltaTime);
   //         FORWARD = true;
            break;
        case SDLK_a:
            camera.ProcessKeyboard(Movement::LEFT, deltaTime);
   //         LEFT = true;
            break;
        case SDLK_s:
            camera.ProcessKeyboard(Movement::BACKWARD, deltaTime);
   //         BACKWARD = true;
            break;
        case SDLK_d:
            camera.ProcessKeyboard(Movement::RIGHT, deltaTime);
   //         RIGHT = true;
            break;
        }
        break;
 /* case SDL_KEYUP:
        switch (event->key.keysym.sym) {
        case SDLK_w:
            FORWARD = false;
            break;
        case SDLK_a:
            LEFT = false;
            break;
        case SDLK_s:
            BACKWARD = false;
            break;
        case SDLK_d:
            RIGHT = false;
            break;
        }*/
    }
}

void handleMouseInput(SDL_Event* event) {
    switch (event->type) {
    case SDL_MOUSEMOTION:
        // TODO what are these even used for???
        lastX += event->motion.xrel;
        lastY += event->motion.yrel;

        camera.ProcessMouseMovement(event->motion.xrel, -event->motion.xrel);
        break;
    case SDL_MOUSEBUTTONDOWN:
        break;
    case SDL_MOUSEBUTTONUP:
        break;
    case SDL_MOUSEWHEEL:
        // TODO make sure this works
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

    lastX = SCREEN_WIDTH * 0.5;
    lastY = SCREEN_HEIGHT * 0.5;

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
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture
        // opengl expects 0.0 to be bottom of image, but
        // images usually have 0.0 at the top, so we flip
        // the image loading
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture1, texture2;
    // texture 1
        // arguments: amount of textures, array to store textures in (since we only have one we just pass the address)
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping/filtering options
    // (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("texture/container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        DUSTY_CORE_ERROR("Failed to load texture");
    // now that texture is generated we'll free the data
    stbi_image_free(data);
    // texture 2
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping/filtering options
    // (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    data = stbi_load("texture/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        DUSTY_CORE_ERROR("Failed to load texture");
    // now that texture is generated we'll free the data
    stbi_image_free(data);

    // tell OpenGL to which texture unit each shader sampler belongs
    // to by setting each sampler using glUniform1i. We only have to
    // set this once, so we can do this before we enter the render loop
    shader.use();   // we have to activate the shader before we can change the uniforms
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);
    
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
        /*glm::vec3 horizontal = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 backForth = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 movement;

        
        const float cameraSpeed = 4.0f * deltaTime; // adjust accordingly
        if (FORWARD)
            backForth += cameraFront;
        if (BACKWARD)
            backForth -= cameraFront;
        if (LEFT)
            horizontal -= glm::normalize(glm::cross(cameraFront, cameraUp));
        if (RIGHT)
            horizontal += glm::normalize(glm::cross(cameraFront, cameraUp));

        // normalize result if both vectors are greater than 0
        if (glm::length(backForth) > 0.0f && glm::length(horizontal) > 0.0f)
            movement = glm::normalize(backForth + horizontal) * cameraSpeed;
        else
            movement = (backForth + horizontal) * cameraSpeed;
        
        cameraPos += movement;*/

        // render
            // clear the color buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures to the corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

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
        //glfwPollEvents();
    }

    // Delete our opengl context, destroy our window, and shutdown SDL
    SDLCleanup(context, window);

    return 0;
}