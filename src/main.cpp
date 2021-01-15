#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
Camera camera(glm::vec3(0.0f, 2.0f, -1.4f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct ProgramState{
    bool ImGuiEnabled = true;
    bool ShowButtons = true;
    bool enableButton = true;
    bool placeManually = false;
    bool battle = false;
    int Width;
    int Height;
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

void DrawImgui(GLFWwindow*);

void setLights(Shader shader, glm::vec3 pVec[4]);

void updateGame(float time);


glm::vec3 indexToWorld(int ind){
    vec2 dvodimenzioni = indexToCoords(ind);
    return glm::vec3(dvodimenzioni.y * 0.25f - 0.875f, -0.05f, dvodimenzioni.x * 0.25f - 0.875f);
}


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    int count;
    GLFWmonitor **monitor = glfwGetMonitors(&count);
    const GLFWvidmode *return_struct = glfwGetVideoMode(monitor[count-1]);
    programState->Width = return_struct->width;
    programState->Height = return_struct->height;

    GLFWwindow* window = glfwCreateWindow(programState->Width, programState->Height, "Labyrinth Escape", monitor[count-1], nullptr);
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

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //stbi_set_flip_vertically_on_load(true);

    //ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    // build and compile shaders
    // -------------------------
    Shader screenShader("resources/shaders/framebuffers.vs", "resources/shaders/framebuffers.fs");
    Shader lightingShader("resources/shaders/lighting/lights.vs", "resources/shaders/lighting/lights.fs");
    Shader skyboxShader("resources/shaders/skyboxShader.vs", "resources/shaders/skyboxShader.fs");

    Model knight(FileSystem::getPath("resources/objects/knight/knight.obj"));
    Model island(FileSystem::getPath("resources/objects/island/island.obj"));
    Model beast(FileSystem::getPath("resources/objects/beast/beast.obj"));
    Model lamp(FileSystem::getPath("resources/objects/lamp/streetlamp.obj"));

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
            // coords                     normals         texCoords
            -1.0f, 0.001f, -1.0f,     0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
            -0.75f, 0.001f, -1.0f,    0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
            -0.75f, 0.001f, -0.75f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,

            -0.75f, 0.001f, -0.75f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
            -1.0f, 0.001f, -0.75f,    0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
            -1.0f, 0.001f, -1.0f,     0.0f, -1.0f, 0.0f,   0.0f, 0.0f
    };

    glm::vec3 pointLightPositions[] = {
            glm::vec3( -1.5f, 0.7f, -1.5f),
            glm::vec3( 1.5f, 0.7f,-1.5f),
            glm::vec3(1.5f, 0.7f,  1.5f),
            glm::vec3( -1.5f, 0.7f,  1.5f)
    };

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f, -1.0f,  0.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f,  1.0f,  1.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f
    };

    //glFrontFace(GL_CW);
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glBindVertexArray(skyboxVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    // polje VAO
    unsigned int poljeVAO, poljeVBO;
    glGenVertexArrays(1, &poljeVAO);
    glGenBuffers(1, &poljeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, poljeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(poljeVertices), &poljeVertices, GL_STATIC_DRAW);

    glBindVertexArray(poljeVAO);
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
    // -------------
    unsigned int cubeTexture = loadTexture(FileSystem::getPath("resources/textures/container2.png").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/container2_specular.png").c_str());

    vector<std::string> faces{
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg")
            };

    unsigned int cubemapTexture = loadCubemap(faces);

    // shader configuration
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // framebuffer configuration
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, programState->Width, programState->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, programState->Width, programState->Height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // render loop
    while (!glfwWindowShouldClose(window)){

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // bind to framebuffer and draw scene as we normally would to color texture
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //postavljamo svetla
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);
        setLights(lightingShader, pointLightPositions);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)programState->Width / (float)programState->Height, 0.1f, 100.0f);

        lightingShader.use();
        lightingShader.setMat4("model", model);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);

        glBindVertexArray(poljeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        for(int i=0; i<8; i++){
            for(int j=0; j<8; j++){
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((float)j*0.25f, 0.0f, (float)i*0.25f));
                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        glBindVertexArray(0);

        for (unsigned int i = 0; i < 4; i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(pointLightPositions[i].x + 0.3f, -0.18f, pointLightPositions[i].z ));
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            lightingShader.setMat4("model", model);
            lamp.Draw(lightingShader);
        }

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.8f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        lightingShader.setMat4("model", model);
        island.Draw(lightingShader);

        updateGame(deltaTime);

        if(!programState->table->team1Units.empty()){
            for(int i=0; i<5; i++){
                if(programState->table->team1Units[i]->Type == 1){
                    glm::vec3 pos = indexToWorld(programState->table->team1Units[i]->PositionIndex);
                    glm::mat4 knightModel = glm::mat4(1.0f);
                    knightModel = glm::translate(knightModel, pos);
                    knightModel = glm::rotate(knightModel, glm::radians(180.0f), glm::vec3(0.0f, pos.x, 0.0f));
                    knightModel = glm::scale(knightModel, glm::vec3(0.6f, 0.6f, 0.6f));
                    lightingShader.setMat4("model", knightModel);
                    knight.Draw(lightingShader);
                }
            }
        }
        if(!programState->table->team2Units.empty()){
            for(int i=5; i<10; i++){
                if(programState->table->team2Units[i]->Type == 1){
                    glm::vec3 pos = indexToWorld(programState->table->team2Units[i]->PositionIndex);
                    glm::mat4 beastModel = glm::mat4(1.0f);
                    beastModel = glm::translate(beastModel, glm::vec3(pos.x, pos.y+0.05, pos.z));
                    beastModel = glm::rotate(beastModel, glm::radians(180.0f), glm::vec3(0.0f, pos.x, 0.0f));
                    beastModel = glm::scale(beastModel, glm::vec3(0.07f, 0.07f, 0.055f));
                    lightingShader.setMat4("model", beastModel);
                    beast.Draw(lightingShader);
                }
            }
        }



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

        DrawImgui(window);

        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        //glEnable(GL_CULL_FACE);
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete programState;

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1,&poljeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1,&poljeVBO);;
    glDeleteBuffers(1, &quadVBO);

    glfwTerminate();
    return 0;
}

void updateGame(float time) {


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
    if(key == GLFW_KEY_1 && action == GLFW_PRESS && programState->placeManually){
        //table->placeWarrior(getCursorWorldLocation());
        //updejtuje matricu i stavi lika na to polje pomocu fje koja vraca centar polja na koje smeramo
    }
    if(key == GLFW_KEY_2 && action == GLFW_PRESS && programState->placeManually){
        //table->placeMage(getCursorWorldLocation());
    }
    if(key == GLFW_KEY_3 && action == GLFW_PRESS && programState->placeManually){
        //table->placeAssassin(getCursorWorldLocation());
    }
    if(key == GLFW_KEY_E && action == GLFW_PRESS && programState->placeManually){
        //table->removeUnit(getCursorWorldLocation());
        //postavlja na to polje na 0 i zaustavlja render modela
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera.ProcessMouseScroll((float)yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
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
    // pointLightsPositions
    lightingShader.setVec3("pointLightPositions[0].position", pointLightPositions[0]);
    lightingShader.setVec3("pointLightPositions[1].position", pointLightPositions[1]);
    lightingShader.setVec3("pointLightPositions[2].position", pointLightPositions[2]);
    lightingShader.setVec3("pointLightPositions[3].position", pointLightPositions[3]);
    // pointLight
    lightingShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLight.diffuse", 0.94f, 0.98f, 0.78f);
    lightingShader.setVec3("pointLight.specular", 0.94f, 0.98f, 0.78f);
    lightingShader.setFloat("pointLight.constant", 2.0f);
    lightingShader.setFloat("pointLight.linear", 0.09);
    lightingShader.setFloat("pointLight.quadratic", 0.032);
    // spotLight
    lightingShader.setVec3("spotLight.position", 0.0f, 2.3f, 0.0f);
    lightingShader.setVec3("spotLight.direction", 0.0f, -1.0f, 0.0f);
    lightingShader.setVec3("spotLight.ambient", 0.15f, 0.15f, 0.15f);
    lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    lightingShader.setVec3("spotLight.specular", 0.7f, 0.7f, 0.7f);
    lightingShader.setFloat("spotLight.constant", 2.0f);
    lightingShader.setFloat("spotLight.linear", 0.3);
    lightingShader.setFloat("spotLight.quadratic", 0.032);
    lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(21.371f)));//21.371f
    lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(23.0f)));
}

void DrawImgui(GLFWwindow *window){
    // ImGUi Frame init
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_Always);
        if (!ImGui::Begin("Buttons", &programState->ImGuiEnabled, programState->window_flags)){
            ImGui::End();
            return;
        }

        {
            if(camera.LockCamera)
                ImGui::Text("Press \"F\" to free camera");
            else
                ImGui::Text("Press \"F\" to lock the camera");
            if (programState->enableButton && ImGui::Button("Randomise Enemy")){
                programState->table->randomiseEnemy();
            }
            if (programState->enableButton && ImGui::Button("Randomise Self")){
                programState->table->randomiseSelf();
            }
            if (programState->enableButton && ImGui::Button("Reset Board")){
                programState->table->clearTable();
            }
            if (ImGui::Button("Battle/Stop battle")){
                programState->enableButton = !programState->enableButton;
                programState->battle = !programState->enableButton;
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
        ImGui::Text("| Camera pos: %.2f, %.2f, %.2f", camera.Position.x, camera.Position.y ,camera.Position.z);
        ImGui::End();
    }

    if(camera.LockCamera && programState->enableButton){
        ImGui::SetNextWindowPos(ImVec2((float)programState->Width - 165, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(165, 600), ImGuiCond_Always);
        if (!ImGui::Begin("Unit input", &programState->ImGuiEnabled, programState->window_flags)){
            ImGui::End();
            return;
        }
        if(!programState->placeManually && ImGui::Button("Input units manually")){
            programState->placeManually = !programState->placeManually;

        }
        else if(programState->placeManually && ImGui::Button("Stop unit input")){
            programState->placeManually = !programState->placeManually;

        }
        ImGui::Text("Press to add: ");
        ImGui::Text("    1-knight");
        ImGui::Text("    2-mage");
        ImGui::Text("    3-assassin");
        ImGui::Text("Press \"E\" to remove.");
    /*    ImGui::Text("Matrix of positions:");
        for(int i=0; i<4; i++){
            ImGui::Text(" "); ImGui::SameLine();
            for(int j=0; j<8; j++){
                ImGui::Text("%d",UnitsTable[i][j]);
                ImGui::SameLine();
            }
            ImGui::Text(" ");
        }*/

        ImGui::End();
    }

    //ImGui render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



