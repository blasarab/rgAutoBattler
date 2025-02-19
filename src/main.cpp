#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>
#include "rg/Camera.h"
#include "Game/table.h"

#include <iostream>
#include <string>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.125f, 2.0f, -1.3f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct ProgramState{
    bool ImGuiEnabled = true;
    bool ShowButtons = true;
    bool game = false;
    int treasuresFound = 0;
    int Width;
    int Height;
    int startingTime;
    int killedByTrap = false;
    int vreme = 30;
    bool lavirintPostavljen = false;
    bool zavrsenGame = false;
    bool enableGrass = true;
    Table *table = new Table();

    ImGuiWindowFlags window_flags = (unsigned)0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
                                    | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                    | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;

    ProgramState(){}

    void UpdateRatio(int width, int height);


};

void ProgramState::UpdateRatio(int width, int height){
    Width = width;
    Height = height;
}


ProgramState *programState = new ProgramState();

void DrawImgui(float);

void setLights(Shader shader, glm::vec3 pVec[4]);

void updateGame(int time);

glm::vec2 indexToCoords(int);


glm::vec3 indexToWorld(int ind){
    glm::vec2 dvodimenzioni = indexToCoords(ind);
    return glm::vec3(dvodimenzioni.x * 0.25f - 0.875f, -0.05f, dvodimenzioni.y * 0.25f - 0.875f);
}


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    int count;
    GLFWmonitor **monitor = glfwGetMonitors(&count);
    const GLFWvidmode *return_struct = glfwGetVideoMode(monitor[count-1]);
    programState->Width = return_struct->width;
    programState->Height = return_struct->height;

    GLFWwindow* window = glfwCreateWindow(programState->Width, programState->Height, "Labyrinth Hunt", monitor[count-1], nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);


    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);

    // build and compile shaders
    Shader screenShader("resources/shaders/framebuffers.vs", "resources/shaders/framebuffers.fs");
    Shader lightingShader("resources/shaders/lighting/lights.vs", "resources/shaders/lighting/lights.fs");
    Shader skyboxShader("resources/shaders/skyboxShader.vs", "resources/shaders/skyboxShader.fs");

    Model knight(FileSystem::getPath("resources/objects/knight/knight.obj"));
    Model island(FileSystem::getPath("resources/objects/island/island.obj"));
    Model lamp(FileSystem::getPath("resources/objects/lamp/streetlamp.obj"));
    Model treasure(FileSystem::getPath("resources/objects/coin/coin.obj"));


    float cubeVertices[] = {
            // positions          // normals           // texture coords
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,      // FRONT FACE
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,      // BACK FACE
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,      // LEFT FACE
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,


            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,      // RIGHT FACE
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,     // BOTTOM FACE
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,      // TOP FACE
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f
    };

    float skyboxVertices[] = {
            // positions
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,

            1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,

            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,

            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f
    };

    float poljeVertices[] = {
            // positions                        // normals                  // texture coords
            -0.75f, 0.0f, -0.75f, 0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
            -0.75f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
            -1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
            -1.0f, 0.0f, -0.75f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f
    };
    unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
    };

    glm::vec3 pointLightPositions[] = {
            glm::vec3( -1.5f, 0.7f, -1.5f),
            glm::vec3( 1.5f, 0.7f,-1.5f),
            glm::vec3(1.5f, 0.7f,  1.5f),
            glm::vec3( -1.5f, 0.7f,  1.5f)
    };

    float quadVertices[] = {
            // positions   // texCoords
            -1.0f, -1.0f,  0.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f,  1.0f,  1.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f
    };

    glFrontFace(GL_CW);
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    // polje VAO
    unsigned int poljeVAO, poljeVBO, poljeEBO;
    glGenVertexArrays(1, &poljeVAO);
    glGenBuffers(1, &poljeVBO);
    glGenBuffers(1, &poljeEBO);
    glBindVertexArray(poljeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, poljeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(poljeVertices), poljeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, poljeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // load textures
    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/container2.png").c_str());
    unsigned int floorSpecular = loadTexture(FileSystem::getPath("resources/textures/container2_specular.png").c_str());
    unsigned int webTexture = loadTexture(FileSystem::getPath("resources/textures/web.jpeg").c_str());
    unsigned int wallTexture = loadTexture(FileSystem::getPath("resources/textures/zid.jpeg").c_str());
    unsigned int wallSpec = loadTexture(FileSystem::getPath("resources/textures/wall_specular.jpg").c_str());
    unsigned int noSpec = loadTexture(FileSystem::getPath("resources/textures/no_specular.jpg").c_str());
    unsigned int grassTexture = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str());

    vector<std::string> faces
    {
        FileSystem::getPath("resources/textures/skybox/right.jpg"),
        FileSystem::getPath("resources/textures/skybox/left.jpg"),
        FileSystem::getPath("resources/textures/skybox/top.jpg"),
        FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
        FileSystem::getPath("resources/textures/skybox/front.jpg"),
        FileSystem::getPath("resources/textures/skybox/back.jpg")
    };

    vector<glm::vec3> grassPos
    {
        glm::vec3(0.5f, -1.7f, 1.35f),
        glm::vec3( 3.2f, -1.7f, 1.35f),
        glm::vec3(0.5f, -1.7f, -1.35f),
        glm::vec3 (3.2f, -1.7f, -1.35f),
        glm::vec3(2.5f, -1.7f, 1.7f),
        glm::vec3( 1.0f, -1.7f, 1.7f),
        glm::vec3(2.5f, -1.7f, -1.7f),
        glm::vec3 (1.0f, -1.7f, -1.7f)
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // shader configuration
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setFloat("material.shininess", 32.0f);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    screenShader.setBool("izgubio", false);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, programState->Width, programState->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, programState->Width, programState->Height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window)){

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noSpec);

        // setting up lighting
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        if(programState->zavrsenGame)
            lightingShader.setBool("upali", true);
        setLights(lightingShader, pointLightPositions);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)programState->Width / (float)programState->Height, 0.1f, 100.0f);

        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);

        // draw lamps and floating island
        for (unsigned int i = 0; i < 4; i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(pointLightPositions[i].x + 0.3f, -0.18f, pointLightPositions[i].z ));
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            lightingShader.setMat4("model", model);
            lamp.Draw(lightingShader);
        }

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.125f, -1.8f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        lightingShader.setMat4("model", model);
        island.Draw(lightingShader);

        // draw knight
        if(programState->game){
            glm::vec3 pos = indexToWorld(programState->table->knight->knightIndex);
            glm::mat4 knightModel = glm::mat4(1.0f);
            knightModel = glm::translate(knightModel, pos);
            knightModel = glm::rotate(knightModel, glm::radians(programState->table->knight->Degrees), glm::vec3(0.0f, 1.0f, 0.0f));
            knightModel = glm::scale(knightModel, glm::vec3(0.6f, 0.6f, 0.6f));
            lightingShader.setMat4("model", knightModel);
            knight.Draw(lightingShader);

            glm::vec3 pos2 = indexToWorld(programState->table->Treasure);
            glm::mat4 treasureModel = glm::mat4(1.0f);
            treasureModel = glm::translate(treasureModel, glm::vec3(pos2.x, 0.1f, pos2.z));
            treasureModel = glm::rotate(treasureModel, glm::radians(currentFrame*25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            treasureModel = glm::scale(treasureModel, glm::vec3(0.06f, 0.05f, 0.06f));
            lightingShader.setMat4("model", treasureModel);
            treasure.Draw(lightingShader);
        }

        // draw labyrinth, floor, treasure and webs
        for(int i=0; i<TABLESIZE*TABLESIZE; i++){
            if(programState->table->Grid[i] == ' '){
                glBindVertexArray(poljeVAO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, floorTexture);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, floorSpecular);
                glm::vec2 coords = indexToCoords(i);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(coords.x*0.25f, 0.0f, coords.y*0.25f));
                lightingShader.setMat4("model", model);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
            else if(programState->table->Grid[i] == '#'){
                glEnable(GL_CULL_FACE);
                glBindVertexArray(cubeVAO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, wallTexture);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, wallSpec);
                glm::vec2 coords = indexToCoords(i);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(coords.x*0.25f-0.875f, 0.0f, coords.y*0.25f-0.875f));
                model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
                glDisable(GL_CULL_FACE);
            }
            else if(programState->table->Grid[i] == 't'){
                glBindVertexArray(poljeVAO);
                if(programState->game && (int)currentFrame%2 == 0){
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, webTexture);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, noSpec);
                }
                else{
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, floorTexture);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, floorSpecular);
                }
                glm::vec2 coords = indexToCoords(i);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(coords.x*0.25f, 0.0f, coords.y*0.25f));
                lightingShader.setMat4("model", model);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }
        // outer walls of labyrinth
        if(programState->lavirintPostavljen){
            glEnable(GL_CULL_FACE);
            for(int i=0; i<TABLESIZE+1; i++){
                glBindVertexArray(cubeVAO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, wallTexture);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, wallSpec);
                glm::vec2 coords = indexToCoords(i*8);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(coords.x*0.25f-0.875f, 0.0f, coords.y*0.25f+1.125f));
                model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            for(int i=0; i<TABLESIZE; i++){
                glBindVertexArray(cubeVAO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, wallTexture);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, wallSpec);
                glm::vec2 coords = indexToCoords(i);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(coords.x*0.25f+1.125f, 0.0f, coords.y*0.25f-0.875f));
                model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glBindVertexArray(0);
            }
            glDisable(GL_CULL_FACE);
        }

        // draw transparent grass
        if(programState->enableGrass){
            glBindVertexArray(poljeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            for (unsigned int i = 0; i < grassPos.size(); i++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, grassPos[i]);
                model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::scale(model, glm::vec3(2.0f));
                lightingShader.setMat4("model", model);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            glBindVertexArray(0);
        }

        updateGame((int)currentFrame);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        DrawImgui(lastFrame);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        if(programState->killedByTrap)
            screenShader.setBool("izgubio", true);
        else
            screenShader.setBool("izgubio", false);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete programState;

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1,&poljeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &poljeVBO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &poljeEBO);

    glfwTerminate();
    return 0;
}

void updateGame(int time) {

    if(programState->game && programState->table->found()){
        programState->treasuresFound++;
        programState->table->generateTreasure();
    }
    if(programState->game && programState->table->nagazio(time)){
        programState->table->knight->HP -= 1;
    }
    if(programState->game && programState->table->mrtav()){
        programState->killedByTrap = true;
        programState->game = false;
        programState->table->ResetKnight();
    }
    if(programState->game && programState->vreme == 0){
        programState->table->ResetKnight();
        programState->game = false;
        programState->zavrsenGame = true;
    }
    else if(programState->game && programState->vreme != 0){
        programState->vreme = 30 - time + programState->startingTime;
    }
}

glm::vec2 indexToCoords(int index){
    glm::vec2 rez;
    rez.x = index/8;
    rez.y = index%8;
    return rez;
}

void processInput(GLFWwindow *window)
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
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod){
    if(key == GLFW_KEY_F && action == GLFW_PRESS){
        camera.unlock();
        if(!camera.LockCamera)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if(key == GLFW_KEY_UP && action == GLFW_PRESS){
        programState->table->move(glm::vec2(1,0));

    }
    if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
        programState->table->move(glm::vec2(0,-1));

    }
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS){
        programState->table->move(glm::vec2(-1,0));

    }
    if(key == GLFW_KEY_LEFT && action == GLFW_PRESS){
        programState->table->move(glm::vec2(0,1));

    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    programState->UpdateRatio(width, height);
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if(!camera.LockCamera){
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.ProcessMouseScroll((float)yoffset);
}

// utility function for loading a 2D texture from file
unsigned int loadTexture(char const * path){
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data){
        GLenum format ;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RED;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else{
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


unsigned int loadCubemap(vector<std::string> faces){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++){
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else{
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void setLights(Shader lightingShader, glm::vec3 pointLightPositions[]) {
    //directional light
    lightingShader.setVec3("dirLight.direction", -0.289f, -0.111f, -0.951f);
    lightingShader.setVec3("dirLight.ambient", 0.15f, 0.005f, 0.005f);
    lightingShader.setVec3("dirLight.diffuse", 0.98f, 0.25f, 0.25f);
    lightingShader.setVec3("dirLight.specular", 0.98f, 0.25f, 0.25f);
    // point light 1
    lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[0].diffuse", 0.94f, 0.98f, 0.78f);
    lightingShader.setVec3("pointLights[0].specular", 0.94f, 0.98f, 0.78f);
    lightingShader.setFloat("pointLights[0].constant", 1.0f);
    lightingShader.setFloat("pointLights[0].linear", 0.2);
    lightingShader.setFloat("pointLights[0].quadratic", 0.5);
    // point light 2
    lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[1].diffuse", 0.94f, 0.98f, 0.78f);
    lightingShader.setVec3("pointLights[1].specular", 0.94f, 0.98f, 0.78f);
    lightingShader.setFloat("pointLights[1].constant", 1.0f);
    lightingShader.setFloat("pointLights[1].linear", 0.2);
    lightingShader.setFloat("pointLights[1].quadratic", 0.05);
    // point light 3
    lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
    lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[2].diffuse", 0.94f, 0.98f, 0.78f);
    lightingShader.setVec3("pointLights[2].specular", 0.94f, 0.98f, 0.78f);
    lightingShader.setFloat("pointLights[2].constant", 1.0f);
    lightingShader.setFloat("pointLights[2].linear", 0.2);
    lightingShader.setFloat("pointLights[2].quadratic", 0.05);
    // point light 4
    lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
    lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[3].diffuse", 0.94f, 0.98f, 0.78f);
    lightingShader.setVec3("pointLights[3].specular", 0.94f, 0.98f, 0.78f);
    lightingShader.setFloat("pointLights[3].constant", 1.0f);
    lightingShader.setFloat("pointLights[3].linear", 0.25);
    lightingShader.setFloat("pointLights[3].quadratic", 0.05);
}

void DrawImgui(float dTime){
    // ImGUi Frame init
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_Always);
        if (!ImGui::Begin("Buttons", &programState->ImGuiEnabled, programState->window_flags)){
            ImGui::End();
            return;
        }

        {
            if (!programState->game && ImGui::Button("Randomise labyrinth")){
                programState->table->RandomiseLabyrinth();
                programState->lavirintPostavljen = true;
                programState->zavrsenGame = false;
            }
            if (programState->lavirintPostavljen && !programState->game && ImGui::Button("Start hunt")){
                programState->killedByTrap = false;
                programState->startingTime = (int)dTime;
                programState->game = !programState->game;
                programState->treasuresFound = 0;
                programState->zavrsenGame = false;
                programState->vreme = 30;
                programState->table->generateTreasure();
            }
            if(programState->game && ImGui::Button("Stop hunt")){
                programState->game = !programState->game;
                programState->table->ResetKnight();
                programState->zavrsenGame = false;
            }
            ImGui::Text("Coins collected: %d", programState->treasuresFound);
            ImGui::Text("Time left: %d", programState->vreme);
            if(programState->killedByTrap){
                ImGui::Text("You stepped on a trap!!!");
            }

        }

        ImGui::End();
    }

    {
        ImGui::SetNextWindowPos(ImVec2(0 , (float)programState->Height-25), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2((float)programState->Width, 25), ImGuiCond_Always);
        if (!ImGui::Begin("Stats", &programState->ImGuiEnabled, programState->window_flags)){
            ImGui::End();
            return;
        }
        ImGui::Text("(FPS: %.1f)", ImGui::GetIO().Framerate);
        ImGui::SameLine();
        ImGui::End();
    }

    {
        ImGui::SetNextWindowPos(ImVec2((float)programState->Width - 250, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250, 600), ImGuiCond_Always);
        if (!ImGui::Begin("Instructions", &programState->ImGuiEnabled, programState->window_flags)){
            ImGui::End();
            return;
        }
        ImGui::Text("Generate labyrinth and start game");
        ImGui::Text("Use arrow keys to move around");
        ImGui::Text("Collect coins before time runs out");
        ImGui::Text("Don`t step on traps!");
        if(camera.LockCamera)
            ImGui::Text("\nPress \"F\" to free camera");
        else
            ImGui::Text("\nPress \"F\" to lock the camera");
        ImGui::Text("Press ESC to exit game");
        if(programState->enableGrass && ImGui::Button("Disable grass")){
            programState->enableGrass = false;
        }
        if(!programState->enableGrass && ImGui::Button("Enable grass")){
            programState->enableGrass = true;
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



