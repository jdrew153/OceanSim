// WaterSimOpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "shader.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include "fileSystem.h"


#include "model.h"

int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(char const* path);


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

const GLfloat vertices[] = {
    -1.0F, -1.0F, 0.0F,
    1.0F, -1.0F, 0.0F,
    1.0F, 1.0F, 0.0F,
    1.0F, 1.0F, 0.0F,
    -1.0F, 1.0F, 0.0F,
    -1.0F, -1.0F, 0.0F
};

const GLfloat UVs[] = {
        0.0F, 0.0F,
        1.0F, 0.0F,
        1.0F, 1.0F,
        1.0F, 1.0F,
        0.0F, 1.0F,
        0.0F, 0.0F,
};

float skyBoxVertices[] = {
    // positions          
    -2.0f,  2.0f, -2.0f,
    -2.0f, -2.0f, -2.0f,
     2.0f, -2.0f, -2.0f,
     2.0f, -2.0f, -2.0f,
     2.0f,  2.0f, -2.0f,
    -2.0f,  2.0f, -2.0f,

    -2.0f, -2.0f,  2.0f,
    -2.0f, -2.0f, -2.0f,
    -2.0f,  2.0f, -2.0f,
    -2.0f,  2.0f, -2.0f,
    -2.0f,  2.0f,  2.0f,
    -2.0f, -2.0f,  2.0f,

     2.0f, -2.0f, -2.0f,
     2.0f, -2.0f,  2.0f,
     2.0f,  2.0f,  2.0f,
     2.0f,  2.0f,  2.0f,
     2.0f,  2.0f, -2.0f,
     2.0f, -2.0f, -2.0f,

    -2.0f, -2.0f,  2.0f,
    -2.0f,  2.0f,  2.0f,
     2.0f,  2.0f,  2.0f,
     2.0f,  2.0f,  2.0f,
     2.0f, -2.0f,  2.0f,
    -2.0f, -2.0f,  2.0f,

    -2.0f,  2.0f, -2.0f,
     2.0f,  2.0f, -2.0f,
     2.0f,  2.0f,  2.0f,
     2.0f,  2.0f,  2.0f,
    -2.0f,  2.0f,  2.0f,
    -2.0f,  2.0f, -2.0f,

    -2.0f, -2.0f, -2.0f,
    -2.0f, -2.0f,  2.0f,
     2.0f, -2.0f, -2.0f,
     2.0f, -2.0f, -2.0f,
    -2.0f, -2.0f,  2.0f,
     2.0f, -2.0f,  2.0f
};



Camera camera(glm::vec3(20.0f, 5.0f, 10.0f));

struct Wave {
    float wV; // wavelength
    float a; // amplitude
    vec3 dir; // direction of the wave
    float speed; // speed of the wave. 
   
    // Constructor with default parameter values
    Wave(float wavelength = 1.0f, float amplitude = 1.0f, glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f), float waveSpeed = 1.0f)
        : wV(wavelength), a(amplitude), dir(direction), speed(waveSpeed) {
    }
};

const int NUM_TILES = 400;    // Total number of tiles
const int ROW_LENGTH = 20;    // Number of tiles in each row
const int COL_LENGTH = 20;    // Number of tiles in each column

mat4 modelMatrices[NUM_TILES]; // Array to store matrices for each model

void createScene() {
    // Ensure that we do not attempt to create more models than the array can hold
    if (ROW_LENGTH * COL_LENGTH > NUM_TILES) {
        // Handle error: grid size exceeds the number of available tiles
        std::cerr << "Error: Grid size exceeds the number of available tiles." << std::endl;
        return;
    }

    // Base transformation for all models (identity matrix)
    mat4 baseModel = mat4(1.0f);

    // Calculate positions for a grid defined by ROW_LENGTH x COL_LENGTH
    int index = 0;
    for (int z = 0; z < COL_LENGTH; z++) { // Rows
        for (int x = 0; x < ROW_LENGTH; x++) { // Columns
            vec3 position = vec3(2 * x, 0, -2 * z); // Position each model 2 units apart in x and z
            modelMatrices[index++] = translate(baseModel, position);
        }
    }
}





int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const char* glsl_version = "#version 430";

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Water Simulation", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    /*glfwSetCursorPosCallback(window, mouse_callback);*/
    glfwSetScrollCallback(window, scroll_callback);

   

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image to flip the image on the y-axis bc of how OpenGL renders pixels
    stbi_set_flip_vertically_on_load(false);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    // setup the initial tiles for the scene
    createScene();

    shader simShader = shader("./shaders/modelVertex.glsl", "./shaders/modelFrag.glsl");

    shader skyboxshader = shader("./shaders/skyboxVertex.glsl", "./shaders/skyboxFrag.glsl");

    
    // plane with higher vertices
    std::string plane_2 = "./objects/plane_high.obj";

    Model planeModel = Model(plane_2);

    // let's try setting up a skybox...
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyBoxVertices), &skyBoxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces{
        "textures/cloudy/yellowcloud_lf.jpg",
        "textures/cloudy/yellowcloud_rt.jpg",
        "textures/cloudy/yellowcloud_up.jpg",
        "textures/cloudy/yellowcloud_dn.jpg",
        "textures/cloudy/yellowcloud_ft.jpg",
        "textures/cloudy/yellowcloud_bk.jpg",

    };

    GLuint cubeMapTex = loadCubemap(faces);
    

    skyboxshader.use();
    skyboxshader.setInt("skybox", 0);


    simShader.use();
    simShader.setInt("skybox", 0);

    GLuint causticTex = loadTexture("textures/c2.jpg");
    GLuint foamTex = loadTexture("textures/FoamGradient.png");

    simShader.use();
    simShader.setInt("caustic", 1);
    simShader.setInt("foam", 2);

    
    float totalTime = 0;

    vec3 planeNormal = vec3(0, 1, 0);
    vec3 cameraPos = vec3(0, 2, 0);
    vec3 lightPos = vec3(0, 25, 2);

    float planeDistance = 2.0f;
    float amplitude = 0.1f;
    float wavelength = 5.0f;
    float speed = 0.2f;


    vec3 windDirection = vec3(0, 0, -1);

    float specPower = 4;

  
    int NUM_WAVES = 200;

    // sea green to start off with.
    float oceanColor[3] = { 0.18, 0.545, 0.341 };

    bool addCausticTex = false;

    

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        totalTime += deltaTime;

        // Process user input
        processInput(window);


        // Clear the screen
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Application Controls");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        ImGui::Spacing();

        ImGui::Text("Wave Controls");
        ImGui::SliderFloat("Amplitude", &amplitude, 0.01, 1.0);
        ImGui::SliderFloat("Wavelength", &wavelength, 0, 100);
        ImGui::SliderFloat("Speed", &speed, 0, 50);
        
        std::string wave_ct = "Current wave count : " + std::to_string(NUM_WAVES);
        ImGui::Text(wave_ct.c_str());
        if (ImGui::Button("Add Waves"))
        {
            NUM_WAVES += 10;
        }
        if (ImGui::Button("Remove Waves"))
        {
            if (NUM_WAVES > 1)
            {
                NUM_WAVES -= 5;
            }
        }

        

        ImGui::Spacing();
        ImGui::Text("Light Controls");
        ImGui::SliderFloat("Light Position.x", &lightPos.x, -10, 50);
        ImGui::SliderFloat("Light Position.y", &lightPos.y, -10, 50);
        ImGui::SliderFloat("Light Position.z", &lightPos.z, -10, 50);

      

        ImGui::SliderFloat("Spec Power", &specPower, 0, 100);

        ImGui::ColorEdit3("OceanColor", oceanColor);

        ImGui::Text("Add Caustics to Ocean");
        ImGui::Checkbox("Toggle Caustic", &addCausticTex);

        ImGui::End();

       

        simShader.use();
        
        simShader.setFloat("totalTime", totalTime);
        simShader.setFloat("time", totalTime);
        simShader.setFloat("amplitude", amplitude);
        simShader.setFloat("wavelength", wavelength);
        simShader.setFloat("speed", speed);
        simShader.setInt("gridWidth", ROW_LENGTH);

        simShader.setVec3("lightPos", lightPos);
        simShader.setVec3("windDirection", windDirection);

        simShader.setFloat("specPower", specPower);

        simShader.setBool("causticToggle", addCausticTex);

        mat4 projection = perspective(radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        mat4 view = camera.GetViewMatrix();

        simShader.setMat4("projection", projection);
        simShader.setMat4("view", view);
        simShader.setVec3("viewPos",  camera.Position);

        simShader.setInt("NUM_WAVES", NUM_WAVES);
        simShader.setVec3("oceanColor", vec3(oceanColor[0], oceanColor[1], oceanColor[2]));
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, causticTex);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, foamTex);

        for (int i = 0; i < NUM_TILES; i++)
        {
            mat4 model = modelMatrices[i];
            model = scale(model, vec3(1.0f));
            simShader.setMat4("model", model);
            planeModel.Draw(simShader);
        }

        
        


        // last draw the skybox
        glDepthFunc(GL_LEQUAL);
        skyboxshader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        view = scale(view, vec3(2.0));
        skyboxshader.setMat4("view", view);
        skyboxshader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        ImGui::End();

    
        // Render ImGui draw data
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Q, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(E, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const* path)
{
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return textureId;
}
