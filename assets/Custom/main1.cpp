#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Engine/input.h>
#include <iostream>
#include <fstream>
//#include <windows.h>
#include "stb_image.h"
#include "Utilities.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "model.h"
#include "mesh.h"

struct CustomFrameBuffer
{
    unsigned int ID = 0;
    unsigned int color1 = 0, color2 = 0, depth = 0;

    CustomFrameBuffer() {}
};

CustomFrameBuffer screenBuffer;


//forward declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void RenderModel(Model* model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection);

void CreateFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID);
void CreateSceneBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& colorBufferID2, unsigned int& depthBufferID);
void RenderToBuffer(CustomFrameBuffer to, CustomFrameBuffer from, unsigned int shader);
void RenderQuad();

//unsigned int loadTexture(const char* path);
void RenderTerrain(glm::mat4 view, glm::mat4 projection, int clipDir = 0);
void RenderSkybox(glm::mat4 view, glm::mat4 projection);
//void SetupResources();

//cubemap stuff

// settings
const unsigned int width = 1700;
const unsigned int height = 900;
glm::vec2 resolution(1700, 900);

float lightIntensity = 9.7;
float lightColor[] = { 1, 1, 1 };
int sunSize = 128;
int posX = 0, posY = 200, posZ = 0;
glm::vec3 sunColor(0.85, 0.6, 0.15);

unsigned int plane, planeSize, VAO, VBO, EBO, cubeSize;
unsigned int skyProgram, terrainProgram, modelProgram, blitProgram, chromProgram, postProgram;
unsigned int diffuseTex, heightmapID, normalmapID, dirtID, sandID, grassID, rockID, snowID;
int lightIntensityLoc;
int lightColorLoc;

Model* backpack;

CustomFrameBuffer post1, post2, post3, scene;
unsigned int defaultAttachments[1] = { GL_COLOR_ATTACHMENT0 };
unsigned int sceneAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

const float WATER_HEIGHT = 25.0f;

//cam
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


// lighting
glm::vec3 lightPos(20.2f, 9.0f,5.0f);


bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;


float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
/*
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
*/
int main()
{
    static double previousT = 0;

    std::cout << "Fortnite Big Chungus!\n";
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
    GLFWwindow* window = glfwCreateWindow(width, height, "HEEELPP het werkt niet meer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //'-----------------------------' window stuff done^^

    Shader lightingShader("assets/Custom/3.3.shader.vs", "assets/Custom/3.3.shader.fs");
    Shader lightCubeShader("assets/Custom/lightcube.vs", "assets/Custom/lightcube.fs");
    //Shader lightingShader("assets/Custom/Lightshader.vs", "assets/Custom/Lightshader.fs");
    
    //lock mouse in frame
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    //SETUP RESOURCES
    stbi_set_flip_vertically_on_load(true);
    backpack = new Model("assets/Custom/Resources/backpack/backpack.obj");

    // set up vertex data and buffers for Crate Cube Object
    float cubeVertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };


    unsigned int cubesVBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubesVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubesVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Skybox vertices and indices:
        // need 24 vertices for normal/uv-mapped Cube
    float vertices[] = {
        // positions            //colors            // tex coords   // normals
        0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 0.0f,   1.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 2.f,       0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 2.f,       0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   -1.f, 1.f,      -1.f, 0.f, 0.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   -1.f, 0.f,      -1.f, 0.f, 0.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, -1.f,      0.f, 0.f, -1.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, -1.f,      0.f, 0.f, -1.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   3.f, 0.f,       0.f, 1.f, 0.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   3.f, 1.f,       0.f, 1.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       0.f, 1.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       0.f, 1.f, 0.f
    };

    unsigned int skyIndices[] = {
        // BOTTOm
        0, 1, 2,   // first triangle
        0, 2, 3,    // second triangle
        // BACK
        14, 6, 7,   // first triangle
        14, 7, 15,    // second triangle
        // RIGHT
        20, 4, 5,   // first triangle
        20, 5, 21,    // second triangle
        // LEFT
        16, 8, 9,   // first triangle
        16, 9, 17,    // second triangle
        // FRONT
        18, 10, 11,   // first triangle
        18, 11, 19,    // second triangle
        // TOP
        22, 12, 13,   // first triangle
        22, 13, 23,    // second triangle
    };

    cubeSize = sizeof(skyIndices);

    VAO; // vertex array object
    VBO; // vertex buffer object
    EBO; // element buffer object
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyIndices), skyIndices, GL_STATIC_DRAW);

    int stride = sizeof(float) * 11;

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);
    // normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 8));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //END SETUP OBJECT


    //Load terrain textures and generate plane
    heightmapID;
    plane = GeneratePlane("assets/Custom/Resources/heightmap.png", GL_RGBA, 4, 0.6f, 0.6f, planeSize, heightmapID);

    // terrain textures
    diffuseTex = loadTexture("assets/Custom/Resources/wall.jpg", GL_RGB, 3);
    normalmapID = loadTexture("assets/Custom/Resources/normalmap.png", GL_RGBA, 4);
    grassID = loadTexture("assets/Custom/Resources/grass.png", GL_RGBA, 4);
    dirtID = loadTexture("assets/Custom/Resources/dirt.jpg", GL_RGB, 3);
    sandID = loadTexture("assets/Custom/Resources/sand.jpg", GL_RGB, 3);
    rockID = loadTexture("assets/Custom/Resources/seamlessrock.jpg", GL_RGB, 3);
    snowID = loadTexture("assets/Custom/Resources/seamlesssnow.jpg", GL_RGB, 3);

    //setup Shaders:
    
    // Terrain Shader Programs
    unsigned int vertTerrain, fragTerrain;
    CreateShader("assets/Custom/Terrainshader.vs", GL_VERTEX_SHADER, vertTerrain);
    CreateShader("assets/Custom/Terrainshader.fs", GL_FRAGMENT_SHADER, fragTerrain);

    unsigned int vertModel, fragModel;
    CreateShader("assets/Custom/model.vs", GL_VERTEX_SHADER, vertModel);
    CreateShader("assets/Custom/model.fs", GL_FRAGMENT_SHADER, fragModel);

    //skybox shader programs
    unsigned int vertSky, fragSky;
    CreateShader("assets/Custom/Skybox.vs", GL_VERTEX_SHADER, vertSky);
    CreateShader("assets/Custom/Skybox.fs", GL_FRAGMENT_SHADER, fragSky);

    unsigned int vertImg, fragImg;
    CreateShader("assets/Custom/image.vs", GL_VERTEX_SHADER, vertImg);
    CreateShader("assets/Custom/image.fs", GL_FRAGMENT_SHADER, fragImg);

    unsigned int chrom, postFx;
    CreateShader("assets/Custom/Abberation.fs", GL_FRAGMENT_SHADER, chrom);
    CreateShader("assets/Custom/bloom.fs", GL_FRAGMENT_SHADER, postFx);


    terrainProgram = glCreateProgram();
    glAttachShader(terrainProgram, vertTerrain);
    glAttachShader(terrainProgram, fragTerrain);
    glLinkProgram(terrainProgram);

    skyProgram = glCreateProgram();
    glAttachShader(skyProgram, vertSky);
    glAttachShader(skyProgram, fragSky);
    glLinkProgram(skyProgram);


    modelProgram = glCreateProgram();
    glAttachShader(modelProgram, vertModel);
    glAttachShader(modelProgram, fragModel);
    glLinkProgram(modelProgram);

    chromProgram = glCreateProgram();
    glAttachShader(chromProgram, vertImg);
    glAttachShader(chromProgram, chrom);
    glLinkProgram(chromProgram); 


    blitProgram = glCreateProgram();
    glAttachShader(blitProgram, vertImg);
    glAttachShader(blitProgram, fragImg);
    glLinkProgram(blitProgram);

    postProgram = glCreateProgram();
    glAttachShader(postProgram, vertImg);
    glAttachShader(postProgram, postFx);
    glLinkProgram(postProgram);



    //Deallocate shader memory
    glDeleteShader(vertTerrain);
    glDeleteShader(fragTerrain);

    glDeleteShader(vertSky);
    glDeleteShader(fragSky);

    glDeleteShader(fragModel);
    glDeleteShader(vertModel);

    glDeleteShader(fragImg);
    glDeleteShader(vertImg);
    glDeleteShader(chrom);
    glDeleteShader(postFx);

    // set terrain textures
    glUseProgram(terrainProgram);
    glUniform1i(glGetUniformLocation(terrainProgram, "heightmap"), 0);
    glUniform1i(glGetUniformLocation(terrainProgram, "normalmap"), 1);
    glUniform1i(glGetUniformLocation(terrainProgram, "dirt"), 2);
    glUniform1i(glGetUniformLocation(terrainProgram, "sand"), 3);
    glUniform1i(glGetUniformLocation(terrainProgram, "grass"), 4);
    glUniform1i(glGetUniformLocation(terrainProgram, "rock"), 5);
    glUniform1i(glGetUniformLocation(terrainProgram, "snow"), 6);


    // -------------------------
    // load textures of Crate

    unsigned int diffuseMap = loadTexture("assets/Custom/container2.png", GL_RGB, 3);
    unsigned int specularMap = loadTexture("assets/Custom/container2_specular.png", GL_RGB, 3);


    // Crate shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    //END OF SETUP RESOURCES-----------------------------------------

        // BUFFER SETUP
    CreateFrameBuffer(width, height, post1.ID, post1.color1, post1.depth);
    CreateFrameBuffer(width, height, post2.ID, post2.color1, post2.depth);
    CreateFrameBuffer(width, height, post3.ID, post3.color1, post3.depth);
    CreateSceneBuffer(width, height, scene.ID, scene.color1, scene.color2, scene.depth);



        // MATRIX SETUP
    glUseProgram(terrainProgram);
    int worldLoc = glGetUniformLocation(terrainProgram, "world");
    int viewLoc = glGetUniformLocation(terrainProgram, "view");
    int projLoc = glGetUniformLocation(terrainProgram, "projection");
    lightIntensityLoc = glGetUniformLocation(terrainProgram, "vLightIntensity");
    lightColorLoc = glGetUniformLocation(terrainProgram, "vLightColor");
    // END MATRIX SETUP

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

            // create matrices
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(65.0f), width / (float)height, 0.1f, 1000.0f);


   //glUseProgram(terrainProgram);





    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {


        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        // -----
        processInput(window);

        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(80.0f), (float)width / (float)height, 0.1f, 10000.0f);

        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, scene.ID);
        glDrawBuffers(2, sceneAttachments);
            glClearColor(0.2f, 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //glCheckFramebufferStatus();
            //glEnable(GL_DEPTH_TEST);
            //render terrain!
            RenderSkybox(view, projection);
            RenderTerrain(view, projection);


            for (int x = 0; x < 2; x++)
            {
                for (int z = 0; z < 2; z++)
                {
                    RenderModel(backpack, modelProgram, glm::vec3(50 * x, 50, 50 * z), glm::vec3(0), 10, view, projection);
                }
            }

            //std::cout << glGetError() << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        //SETUP AND RENDER CRATE CUBE------------------------------------------------------------------
        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("light.position", lightPos);
        lightingShader.setVec3("viewPos", cameraPos);

        // light properties
        //lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        //lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        //lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        lightingShader.setVec3("material.specular", 0.7f, 0.7f, 0.7f);
        lightingShader.setFloat("material.shininess", 64.0f);
        //light values for cube
        float lightStrength = 1.5;
 

        // camera/view transformation
        float radius = 15.0f;
        float camX = static_cast<float>(sin(glfwGetTime()) * radius);
        float camZ = static_cast<float>(cos(glfwGetTime()) * radius);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        lightingShader.setMat4("view", view);
        //disco light shit set properties  
        glm::vec3 lightColor;
        lightColor.x = static_cast<float>(sin(1 * (1.0 * lightStrength)));
        lightColor.y = static_cast<float>(sin(1 * (1.0 * lightStrength)));
        lightColor.z = static_cast<float>(sin(1 * (1.0 * lightStrength)));
        glm::vec3 diffuseColor = lightColor * glm::vec3(1.5f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.3f); // low influence
        lightingShader.setVec3("light.ambient", ambientColor);
        lightingShader.setVec3("light.diffuse", diffuseColor);
        lightingShader.setVec3("light.specular", 2.6f, 2.6f, 2.6f);
        lightingShader.setVec3("light.lightOffset", 9, 5, 2.0f);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // activate shader
        //lightingShader.use();

        // retrieve the matrix uniform locations
        unsigned int modelLoc = glGetUniformLocation(lightingShader.ID, "model");
        unsigned int viewLoc = glGetUniformLocation(lightingShader.ID, "view");

        //retrieve uniform location of Post Shader:
        unsigned int postLoc = glGetUniformLocation(postProgram, "screenResolution");
        //glUniform2fv(glGetUniformLocation(bloomProgram, "screenResolution"), 9, resolution);

        // pass them to the shaders
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

        lightingShader.setMat4("projection", projection);
        lightingShader.setVec3("lightPos", lightPos);

        // render box
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // also draw the lamp object
        //lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(7.0f)); // a smaller cube
        lightCubeShader.setMat4("model", model);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //DONE RENDERING CUBE-----------------------------------------------------

        float time = glfwGetTime();
        int timeLocation = glGetUniformLocation(postProgram, "time");
        glUniform1f(timeLocation, time);
        //Post processing
         // Post Processing
        // Post Processing
        RenderToBuffer(post2, scene, chromProgram);
        RenderToBuffer(post3, post2, postProgram);
        RenderToBuffer(post1, post3, blitProgram);
        RenderToBuffer(screenBuffer, post1, blitProgram);

        //holy shit help waarom zie ik de terrain niet??????
        //swap buffers and poll IO events
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubesVBO);
    glDeleteBuffers(1, &EBO);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //cam movement shit
    float cameraSpeed =20.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;



}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void RenderModel(Model* model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection)
{
    // shader
    glUseProgram(shader);

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BACK);

    // world matrix
    glm::mat4 world = glm::mat4(1);
    world = glm::translate(world, position);
    world = glm::scale(world, glm::vec3(scale));
    glm::quat q(rotation);
    world = world * glm::toMat4(q);

    // setup shader
    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(cameraPos));

    // sun
    float t = glfwGetTime();
    glUniform3f(glGetUniformLocation(modelProgram, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    model->Draw(shader);
}

void RenderSkybox(glm::mat4 view, glm::mat4 projection)
{
    //glDepthMask(GL_FALSE);
    glUseProgram(skyProgram);
    glCullFace(GL_FRONT);
    glDisable(GL_DEPTH_TEST);
    glm::mat4 world = glm::mat4(1.0f);

    world = glm::translate(world, cameraPos);

    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPos));
    glUniform3fv(glGetUniformLocation(skyProgram, "sunColor"), 1, glm::value_ptr(sunColor));
    glUniform1i(glGetUniformLocation(skyProgram, "sunSize"), sunSize);
    glUniform1f(lightIntensityLoc, lightIntensity);
    glUniform3fv(lightColorLoc, 3, lightColor);

    // sun
    float t = glfwGetTime();
    glUniform3f(glGetUniformLocation(skyProgram, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, cubeSize, GL_UNSIGNED_INT, 0);
    glDepthMask(GL_TRUE);
}

void RenderTerrain(glm::mat4 view, glm::mat4 projection, int clipDir)
{
    glUseProgram(terrainProgram);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glm::mat4 world = glm::mat4(1.0f);

    world = glm::translate(world, glm::vec3(0, 0, 0));

    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(terrainProgram, "cameraPosition"), 1, glm::value_ptr(cameraPos));

    // sun cycle
    float t = glfwGetTime();
    glUniform3f(glGetUniformLocation(terrainProgram, "lightDirection"), glm::cos(t), -0.9f, glm::sin(t));

    glUniform1f(glGetUniformLocation(terrainProgram, "waterHeight"), 1);
    glUniform1i(glGetUniformLocation(terrainProgram, "clipDir"), clipDir);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalmapID);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dirtID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sandID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grassID);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, rockID);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, snowID);

    glBindVertexArray(plane);
    glDrawElements(GL_TRIANGLES, planeSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}



void CreateFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID)
{
    // frame buffer
    glGenFramebuffers(1, &frameBufferID);

    // color buffer
    glGenTextures(1, &colorBufferID);
    glBindTexture(GL_TEXTURE_2D, colorBufferID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // depth buffer
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_PROXY_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
    glBindTexture(GL_PROXY_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

    glGenRenderbuffers(1, &depthBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CreateSceneBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& colorBufferID2, unsigned int& depthBufferID)
{
    // frame buffer
    glGenFramebuffers(1, &frameBufferID);

    // color buffer
    glGenTextures(1, &colorBufferID);
    glBindTexture(GL_TEXTURE_2D, colorBufferID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // color buffer
    glGenTextures(1, &colorBufferID2);
    glBindTexture(GL_TEXTURE_2D, colorBufferID2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

    // depth buffer 
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_PROXY_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
    glBindTexture(GL_PROXY_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

    glGenRenderbuffers(1, &depthBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach buffers
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, colorBufferID2, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderToBuffer(CustomFrameBuffer to, CustomFrameBuffer from, unsigned int shader)
{
    glBindFramebuffer(GL_FRAMEBUFFER, to.ID);

    glUseProgram(shader);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, from.color1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, from.color2);

    RenderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // uvs
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

//Utility Functions (should move this to seperate header later)

