/*
 CS 6366 - Computer Graphics
 Assignment 4
 Student: Tri M. Cao
 Date: April 7, 2018
 */

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>
//#include <glm/gtx/string_cast.hpp>

// STB IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// Other includes
#include "shader.h"
#include "nanogui/nanogui.h"
#include "load_obj.h"

using namespace nanogui;

// declare methods
std::vector<float> computeAverageXYZ(std::vector<Vertex> vertices);
glm::vec3 camToWorld(glm::vec3 point, glm::vec3 camPos, glm::vec3 camRight, glm::vec3 camUp, glm::vec3 camFront);
unsigned int loadTexture(const char *path);
void computeTangent(std::vector<Vertex>&vertices);
void setupDraw();
void resetValues();
std::vector<glm::vec3> vertex_transform_view(GLfloat vertices[], int size, glm::mat4 modelView, float &min_depth, float &max_depth);
glm::vec3 plane_line_intersect(glm::vec3 edge0, glm::vec3 edge1, glm::vec3 plane0, glm::vec3 normal);
std::vector<glm::vec3> intersect_points_per_plane(glm::vec3 plane0, glm::vec3 normal, std::vector<glm::vec3> vertices, GLuint edges[], int num_edges);
glm::vec3 find_centroid(std::vector<glm::vec3> points);
bool less(glm::vec3 a, glm::vec3 b, glm::vec3 center);
void insertion_sort(std::vector<glm::vec3> &points, glm::vec3 center);
void gen_pos_indices(std::vector<glm::vec3> &original_pos, std::vector<float> &positions, std::vector<unsigned int> &indices, glm::mat4 &matrix);
GLubyte * load_3d_raw_data(std::string texture_path, glm::vec3 dimension);

enum render_type {
    Render1 = 0,
    Render2,
    Render3
};


// Window dimensions
const GLuint SCR_WIDTH = 1024, SCR_HEIGHT = 768;

// sampling rate
int sampling_rate = 10;
int reference_sampling_rate = 10;
// slider to show the line triangles
float drawPercent = 1.0;

// turn on/off texture map
bool textureMapFlag = true;

float camU = 0.0f; // in camera coordinate
float camV = 0.0f;
float camN = 0.0f;

float targetU = 0.0f; // in camera coordinate
float targetV = 0.0f;
float targetN = 1.0f;

// camera coordinate system
glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraOrigin = glm::vec3(0.0f, 0.0f, 0.0f); // this is in world coordinate

glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

// camera view
glm::vec3 viewPos    = glm::vec3(camU, camV, camN);
glm::vec3 viewTarget = glm::vec3(targetU, targetV, targetN);
glm::vec3 viewUp     = glm::vec3(0.0f, 1.0f, 0.0f);

// initialize variables for the GUI
float rotateStep = 4.0f;
float zNear = 1.0f;
float zFar = 100.0f;
float viewDegree = 45.0f;
std::string modelName = "cube";
std::string modelPath = "models/" + modelName + ".obj";
std::string lastModelPath = modelPath;
std::string lastModelName = modelName;
render_type renderType = Render2;
Color colval(0.4f, 0.6f, 1.0f, 1.0f);

float shininess = 32.0f;
bool pointLightFlag = false;
bool dirLightFlag = true;
bool rotateCam = false;

// directional light attributes
float dirLightX = 0.0f; // in camera coordinate
float dirLightY = -1.0f;
float dirLightZ = -1.0f;
glm::vec3 dirLightDirection(dirLightX, dirLightY, dirLightZ);
Color dirLightAmbient(0.05f, 0.1f, 0.2f, 1.0f);
Color dirLightDiffuse(0.3f, 0.5f, 1.0f, 1.0f);
Color dirLightSpecular(1.0f, 1.0f, 1.0f, 1.0f);

// point light attributes
Color pointLightAmbient(0.2f, 0.1f, 0.05f, 1.0f);
Color pointLightDiffuse(1.0f, 0.5f, 0.3f, 1.0f);
Color pointLightSpecular(1.0f, 1.0f, 1.0f, 1.0f);

// light position
glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 defaultLightPos = lightPos;

// turn on/off diffuse mapping and normal mapping
unsigned int diffuseMap;
unsigned int normalMap;
bool diffuseMapFlag = false;
bool normalMapFlag = false;

Screen *screen = nullptr;

// Set up vertex data (and buffer(s)) and attribute pointers
std::vector<Vertex> vertices;
std::vector<float> aveXY;
float stretchY, stretchX, stretchZ;
unsigned int objectVAO, objectVBO;
unsigned int lampVAO, lampVBO;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Create a GLFWwindow object
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 4", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
#if defined(NANOGUI_GLAD)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Could not initialize GLAD!");
    glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
#endif
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Create a nanogui screen and pass the glfw pointer to initialize
    screen = new Screen();
    screen->initialize(window, true);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(0);
    glfwSwapBuffers(window);
    
    // Create nanogui gui
    bool enabled = true;
    FormHelper *gui = new FormHelper(screen);
    ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(0, 10), "Selectors 1");
    
    gui->addGroup("Position");
    gui->addVariable("X", camU)->setSpinnable(true);
    gui->addVariable("Y", camV)->setSpinnable(true);
    gui->addVariable("Z", camN)->setSpinnable(true);
    
    gui->addGroup("Rotate");
    gui->addVariable("Rotate step", rotateStep)->setSpinnable(true);
    gui->addButton("Rotate right+", [](){
        cameraUp = glm::normalize(glm::rotate(cameraUp, glm::radians(rotateStep), cameraUp));
        cameraFront = glm::normalize(glm::rotate(cameraFront, glm::radians(rotateStep), cameraRight));
    });
    gui->addButton("Rotate right-", []() {
        cameraUp = glm::normalize(glm::rotate(cameraUp, glm::radians(-rotateStep), cameraUp));
        cameraFront = glm::normalize(glm::rotate(cameraFront, glm::radians(-rotateStep), cameraRight));
    });;
    gui->addButton("Rotate up+", []() {
        cameraRight = glm::normalize(glm::rotate(cameraRight, glm::radians(-rotateStep), cameraUp));
        cameraFront = glm::normalize(glm::rotate(cameraFront, glm::radians(-rotateStep), cameraUp));
    });
    gui->addButton("Rotate up-", []() {
        cameraRight = glm::normalize(glm::rotate(cameraRight, glm::radians(rotateStep), cameraUp));
        cameraFront = glm::normalize(glm::rotate(cameraFront, glm::radians(rotateStep), cameraUp));
    });
    gui->addButton("Rotate front+", []() {
        viewUp = glm::normalize(glm::rotate(viewUp, glm::radians(-rotateStep), cameraFront));
    });
    gui->addButton("Rotate front-", []() {
        viewUp = glm::normalize(glm::rotate(viewUp, glm::radians(rotateStep), cameraFront));
    });
    
    gui->addGroup("Configuration");
    //gui->addVariable("z near", zNear)->setSpinnable(true);
    //gui->addVariable("z far", zFar)->setSpinnable(true);
    gui->addVariable("Render type", renderType, enabled)->setItems({"Line", "Fill", "Point"});
    gui->addVariable("Sampling rate", sampling_rate)->setSpinnable(true);
    gui->addVariable("Texture map", textureMapFlag);
    gui->addVariable("Model name", modelName);
    gui->addButton("Reload model", []() {
        resetValues(); setupDraw();
        //gui->refresh();
        //gui2->refresh();
    });
    
    // create another Selector Bar
    //window = new Window(this, "Basic widgets");
    //window->setPosition(Vector2i(230, 10));
    //window->setLayout(new GroupLayout());
    
    
    FormHelper *gui2 = new FormHelper(screen);
    ref<Window> nanoguiWindow2 = gui2->addWindow(Eigen::Vector2i(230, 10), "Selectors 2");
    
    
    //Widget *panel = new Widget(nanoguiWindow2);
    //panel->setLayout(new BoxLayout(Orientation::Horizontal,
    //                               Alignment::Middle, 0, 20));

    
    gui2->addVariable("Draw Percentage", drawPercent);
    
    gui2->addGroup("Lighting");
    gui2->addVariable("Object color", colval);
    gui2->addVariable("Object shininess", shininess)->setSpinnable(true);
    
    gui2->addVariable("Texture map status", diffuseMapFlag);
    gui2->addVariable("Normal map status", normalMapFlag);

    
    screen->setVisible(true);
    screen->performLayout();
    
    glfwSetCursorPosCallback(window,
                             [](GLFWwindow *, double x, double y) {
                                 screen->cursorPosCallbackEvent(x, y);
                             }
                             );
    
    glfwSetMouseButtonCallback(window,
                               [](GLFWwindow *, int button, int action, int modifiers) {
                                   screen->mouseButtonCallbackEvent(button, action, modifiers);
                               }
                               );
    
    glfwSetKeyCallback(window,
                       [](GLFWwindow *, int key, int scancode, int action, int mods) {
                           screen->keyCallbackEvent(key, scancode, action, mods);
                       }
                       );
    
    glfwSetCharCallback(window,
                        [](GLFWwindow *, unsigned int codepoint) {
                            screen->charCallbackEvent(codepoint);
                        }
                        );
    
    glfwSetDropCallback(window,
                        [](GLFWwindow *, int count, const char **filenames) {
                            screen->dropCallbackEvent(count, filenames);
                        }
                        );
    
    glfwSetScrollCallback(window,
                          [](GLFWwindow *, double x, double y) {
                              screen->scrollCallbackEvent(x, y);
                          }
                          );
    
    glfwSetFramebufferSizeCallback(window,
                                   [](GLFWwindow *, int width, int height) {
                                       screen->resizeCallbackEvent(width, height);
                                   }
                                   );
    
    
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();
    
    // Build and compile our shader program
    Shader objectShader("shader/vol.vs", "shader/vol.fs");
    
    // setup the vertices and buffers
    //setupDraw();
    
    GLfloat cube_vertices[24] = {
        0.0, 0.0, 0.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 1.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 1.0
    };
    
    GLuint cube_indices[36] = {
        1,5,7,
        7,3,1,
        0,2,6,
        6,4,0,
        0,1,3,
        3,2,0,
        7,5,4,
        4,6,7,
        2,3,7,
        7,6,2,
        1,0,4,
        4,5,1
    };
    
    GLuint cube_edges[24]{
        1,5,
        5,7,
        7,3,
        3,1,
        0,4,
        4,6,
        6,2,
        2,0,
        0,1,
        2,3,
        4,5,
        6,7
    };
    
    // setup camera
    cameraOrigin = glm::vec3(0.5, 0.5, 3.0);
    
    // setup 3D texture
    std::string texture_path = "./geometry/Bucky_32_32_32.raw";
    GLubyte *texture_data = load_3d_raw_data(texture_path, glm::vec3(32,32,32));
    
    unsigned int textureID;
    int tex_width = 32;
    int tex_height = 32;
    int tex_depth = 32;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, tex_width, tex_height, tex_depth, 0, GL_RED,
                 GL_UNSIGNED_BYTE, texture_data);
    
    
    
    
    // shader configuration
    // --------------------
    objectShader.Use();
    
    // refresh the gui
    gui->refresh();
    gui2->refresh();
    
    
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        
        // Clear the colorbuffer and depth buffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // update model path (if changed)
        if (lastModelName.compare(modelName) != 0)
        {
            resetValues();
            //setupDraw();
            // refresh the gui
            gui->refresh();
            gui2->refresh();
        }
        lastModelName = modelName;

        // update camera position and target
        viewPos = camToWorld(glm::vec3(camU, camV, camN), cameraOrigin, cameraRight, cameraUp, cameraFront);
        viewTarget = camToWorld(glm::vec3(camU+targetU, camV+targetV, camN+targetN), cameraOrigin, cameraRight, cameraUp, cameraFront);
        // model matrix
        glm::mat4 model = glm::mat4(1.0f);
        // view matrix
        glm::mat4 view = glm::lookAt(viewPos, viewTarget, viewUp);
        // projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(viewDegree), (float)SCR_WIDTH / (float)SCR_HEIGHT, zNear, zFar);
        
    
        // test transforming vertices
        glm::mat4 modelView = view*model;
        glm::mat4 modelViewInverse = glm::inverse(modelView);
        
        float min_depth = 0.0f;
        float max_depth = 0.0f;
        
        std::vector<glm::vec3> transform = vertex_transform_view(cube_vertices, 24, modelView, min_depth, max_depth);
        
        // generate sampling planes
        float plane_dist = 1 / float(sampling_rate);
        glm::vec3 normalVec = glm::vec3(0.0, 0.0, -1.0);
        int num_planes = (max_depth - min_depth) / plane_dist - 1;
        //std::cout << num_planes << "\n";
        // set num_planes based on the percentage of planes we draw
        num_planes = int(drawPercent * num_planes);
        
        std::vector<float> vertices_draw;
        std::vector<unsigned int> indices_draw;
        std::vector<int> indices_size;
        float d = max_depth; // denote the current depth of the sampling plane
        unsigned int VBOs[num_planes], VAOs[num_planes], EBOs[num_planes];
        glGenVertexArrays(num_planes, VAOs); // we can also generate multiple VAOs or buffers at the same time
        glGenBuffers(num_planes, VBOs);
        glGenBuffers(num_planes, EBOs);
        // setup the data for all the planes
        for (int i = 0; i < num_planes; i++) {
            d = d - plane_dist;
            //std::cout << "d: " << d << "\n";
            std::vector<glm::vec3> intersects = intersect_points_per_plane(glm::vec3(0.0,0.0,d), normalVec, transform, cube_edges, 24);
            
            // generate vertices positions (in object space) and triangle indices
            vertices_draw.clear();
            indices_draw.clear();
            gen_pos_indices(intersects, vertices_draw, indices_draw, modelViewInverse);
            indices_size.push_back(indices_draw.size());
            // setup triangles for the current sampling plane
            glBindVertexArray(VAOs[i]);
            glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
            glBufferData(GL_ARRAY_BUFFER, vertices_draw.size() * sizeof(float), vertices_draw.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_draw.size() * sizeof(unsigned int), indices_draw.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }
        
        
        // get matrix's uniform location and set matrices
        // set model, view and projection matrices
        objectShader.Use();
        
        objectShader.setMat4("model", model);
        objectShader.setMat4("view", view);
        objectShader.setMat4("projection", projection);
        
        glm::vec4 myColor = glm::vec4(colval[0], colval[1], colval[2], colval[3]);
        objectShader.setVec4("ourColor", myColor);
        objectShader.setBool("textureMapFlag", textureMapFlag);
        
        // enable culling face
        //glEnable(GL_CULL_FACE);
        //glFrontFace(GL_CCW);
        
        // blending
        glEnable(GL_BLEND);
        // TODO: check the glBlendFunc
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
        
        // render object, choosing render type
        switch(renderType)
        {
            case Render1:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case Render2:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            case Render3:
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                break;
        }
        
        // bind textures on corresponding texture units
        if (textureMapFlag) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, textureID);
        }
        
        for (int i = 0; i < num_planes; i++) {
            glBindVertexArray(VAOs[i]);
            //glDrawArrays(GL_TRIANGLES, 0, 6);
            glDrawElements(GL_TRIANGLES, indices_size[i], GL_UNSIGNED_INT, 0);
        }
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         
        // draw the GUI
        screen->drawWidgets();
        
        // Swap the screen buffers
        glfwSwapBuffers(window);
        
        // de-allocate resources
        //glDeleteVertexArrays(num_planes, &VAOs);
        //glDeleteBuffers(num_planes, &VBOs);
        //glDeleteBuffers(num_planes, &EBOs);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    //glDeleteVertexArrays(1, &objectVAO);
    //glDeleteVertexArrays(1, &lampVAO);
    //glDeleteBuffers(1, &objectVBO);
    //glDeleteBuffers(1, &lampVBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// computer average x, y of the vertices
// -------------------------------------
std::vector<float> computeAverageXYZ(std::vector<Vertex> vertices)
{
    std::vector<float> aveXY = std::vector<float>();
    float minX = vertices[0].Position.x;
    float maxX = vertices[0].Position.x;
    float minY = vertices[0].Position.y;
    float maxY = vertices[0].Position.y;
    float minZ = vertices[0].Position.z;
    float maxZ = vertices[0].Position.z;
    // find max and min X (and Y and Z)
    for (int i = 0; i < vertices.size(); i++)
    {
        if (vertices[i].Position.x > maxX)
        {
            maxX = vertices[i].Position.x;
        }
        if (vertices[i].Position.x < minX)
        {
            minX = vertices[i].Position.x;
        }
        if (vertices[i].Position.y > maxY)
        {
            maxY = vertices[i].Position.y;
        }
        if (vertices[i].Position.y < minY)
        {
            minY = vertices[i].Position.y;
        }
        if (vertices[i].Position.z < minZ)
        {
            minZ = vertices[i].Position.z;
        }
        if (vertices[i].Position.z > maxZ)
        {
            maxZ = vertices[i].Position.z;
        }
    }
    float aveX = (maxX + minX) / 2;
    float aveY = (maxY + minY) / 2;
    float aveZ = (maxZ + minZ) / 2;
    aveXY.push_back(aveX);
    aveXY.push_back(aveY);
    aveXY.push_back(aveZ);
    aveXY.push_back(minX);
    aveXY.push_back(maxX);
    aveXY.push_back(minY);
    aveXY.push_back(maxY);
    aveXY.push_back(minZ);
    aveXY.push_back(maxZ);
    return aveXY;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int imgWidth, imgHeight, nrComponents;
    unsigned char *data = stbi_load(path, &imgWidth, &imgHeight, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // free memory
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Textures failed to load at path: " << path << "\n";
        stbi_image_free(data);
    }
    return textureID;
}

// compute tangent and bitangents for each vertex
// ---------------------------------------------------
void computeTangent(std::vector<Vertex>&vertices)
{
    for (int i = 0; i < vertices.size(); i+=3)
    {
        glm::vec3 tangent, bitangent;
        glm::vec3 edge1 = vertices[i+1].Position - vertices[i].Position;
        glm::vec3 edge2 = vertices[i+2].Position - vertices[i].Position;
        glm::vec2 deltaUV1 = vertices[i+1].TexCoords - vertices[i].TexCoords;
        glm::vec2 deltaUV2 = vertices[i+2].TexCoords - vertices[i].TexCoords;
        
        GLfloat f = 1.0f / (deltaUV1.x*deltaUV2.y - deltaUV1.y*deltaUV2.x);
        
        tangent.x = f * (deltaUV2.y*edge1.x - deltaUV1.y*edge2.x);
        tangent.y = f * (deltaUV2.y*edge1.y - deltaUV1.y*edge2.y);
        tangent.z = f * (deltaUV2.y*edge1.z - deltaUV1.y*edge2.z);
        tangent = glm::normalize(tangent);
        
        bitangent.x = f * (-deltaUV2.x*edge1.x + deltaUV1.x*edge2.x);
        bitangent.y = f * (-deltaUV2.x*edge1.y + deltaUV1.x*edge2.y);
        bitangent.z = f * (-deltaUV2.x*edge1.z + deltaUV1.x*edge2.z);
        bitangent = glm::normalize(bitangent);
        
        // set tangent and bitangent vectors to the vertices
        vertices[i].Tangent = tangent;
        vertices[i+1].Tangent = tangent;
        vertices[i+2].Tangent = tangent;
        vertices[i].Bitangent = bitangent;
        vertices[i+1].Bitangent = bitangent;
        vertices[i+2].Bitangent = bitangent;
    }
}

// method to convert position on camera coordinates to world coordinates
// ---------------------------------------------------------------------
glm::vec3 camToWorld(glm::vec3 point, glm::vec3 camPos, glm::vec3 camRight, glm::vec3 camUp, glm::vec3 camFront)
{
    glm::mat4 transform_mat = glm::mat4(1.0f);
    transform_mat[0] = glm::vec4(camRight, 0.0f);
    transform_mat[1] = glm::vec4(camUp, 0.0f);
    transform_mat[2] = glm::vec4(camFront, 0.0f);
    transform_mat[3] = glm::vec4(camPos, 1.0f);
    glm::vec3 result = glm::vec3(transform_mat * glm::vec4(point, 1.0f));
    return result;
}

// setup the model: read vertex data, create vertex buffer, etc.
// --------------------------------------------------------------
void setupDraw()
{
    vertices.clear();
    
    // load model
    modelPath = "models/" + modelName + ".obj";
    load_obj(modelPath, vertices);
    // load textures
    std::string texturePath = "textures/" + modelName + "_diffuse.png";
    std::string normalMapPath = "textures/" + modelName + "_normal.png";
    diffuseMap = loadTexture(texturePath.c_str());
    normalMap = loadTexture(normalMapPath.c_str());
    
    // compute tangent and bitangent vectors
    computeTangent(vertices);
    
    // computer average X and Y
    aveXY = computeAverageXYZ(vertices);
    std::cout << "Ave X: " << aveXY[0] << "\n";
    std::cout << "Ave Y: " << aveXY[1] << "\n";
    std::cout << "Ave Z: " << aveXY[2] << "\n";
    // set original camera position and target
    // find the range X and Y of the object
    stretchZ = aveXY[8] - aveXY[7];
    stretchY = aveXY[6] - aveXY[5];
    stretchX = aveXY[4] - aveXY[3];
    std::cout << "Stretch Y: " << stretchY << "\n";
    std::cout << "Stretch X: " << stretchX << "\n";
    std::cout << "Stretch Z: " << stretchZ << "\n";
    // set the starting position (origin) for the camera
    float originZ = glm::max(stretchX, stretchY, stretchZ)*2.0;
    cameraOrigin = glm::vec3(aveXY[0], aveXY[1], originZ);
    
    // create the vertex buffer
    glGenVertexArrays(1, &objectVAO);
    glGenBuffers(1, &objectVBO);
    glBindVertexArray(objectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, objectVBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    
    glBindVertexArray(0);
    
    // set point light original position
    defaultLightPos = cameraOrigin;
    lightPos = cameraOrigin;
}

// reset the values when we load a new model
// ---------------------------------------------
void resetValues()
{
    camU = 0.0f;
    camV = 0.0f;
    camN = 0.0f;
    targetU = 0.0f;
    targetV = 0.0f;
    targetN = 1.0f;
    viewPos    = glm::vec3(camU, camV, camN);
    viewTarget = glm::vec3(targetU, targetV, targetN);
    viewUp     = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
    dirLightDirection = glm::vec3(0.0f, -1.0f, -1.0f);
}


// transform vertices into view coordinates
std::vector<glm::vec3> vertex_transform_view(GLfloat vertices[], int size, glm::mat4 modelView, float &min_depth, float &max_depth)
{
    float minZ;
    float maxZ;
    std::vector<glm::vec3> transformed;
    // transform each vertex from the vertices list.
    for (int i = 0; i < size; i += 3)
    {
        glm::vec4 v = glm::vec4(vertices[i], vertices[i+1], vertices[i+2], 1.0f);
        glm::vec3 transformed_v = glm::vec3(modelView*v);
        transformed.push_back(transformed_v);
        if (i == 0) {
            minZ = transformed_v.z;
            maxZ = transformed_v.z;
        }
        else {
            if (transformed_v.z < minZ) {
                minZ = transformed_v.z;
            }
            if (transformed_v.z > maxZ) {
                maxZ = transformed_v.z;
            }
        }
    }
    // find the min and max depth for the sampling planes
    min_depth = minZ;
    max_depth = maxZ;
    return transformed;
}


// compute the intersect point for plane vs edge.
// ----------------------------------------------
glm::vec3 plane_line_intersect(glm::vec3 edge0, glm::vec3 edge1, glm::vec3 plane0, glm::vec3 normal)
{
    //std::cout << "z of edge 0: " << edge0.z << "\n";
    //std::cout << "z of edge 1: " << edge1.z << "\n";
    //std::cout << "\n";
    float s_i = -1.0f;
    if ((plane0.z <= edge0.z) and (plane0.z >= edge1.z)) {
        float denominator = glm::dot(normal, edge1 - edge0);
        if (glm::abs(denominator) > 0.0001) {
            s_i =  glm::dot(normal, plane0 - edge0) / denominator;
            //std::cout << "s_i: " << s_i << "\n";
        }
    }
    if ((s_i > 0.0001) and (s_i < 0.9999)) {
        return (edge0 + s_i * (edge1 - edge0));
    }
    else {
        return glm::vec3(-99, -99, -99);
    }
}

// find the intersection points between the plane and all the edges
// ------------------------------------------------------------
std::vector<glm::vec3> intersect_points_per_plane(glm::vec3 plane0, glm::vec3 normal, std::vector<glm::vec3> vertices, GLuint edges[], int num_edges)
{
    std::vector<glm::vec3> points;
    glm::vec3 test_intersect(0, 0, 0);
    for (int i = 0; i < num_edges; i+=2) {
        if (vertices[edges[i]].z >= vertices[edges[i+1]].z) {
            test_intersect = plane_line_intersect(vertices[edges[i]], vertices[edges[i+1]], plane0, normal);
        }
        else {
            test_intersect = plane_line_intersect(vertices[edges[i+1]], vertices[edges[i]], plane0, normal);
        }
        if (glm::all(glm::notEqual(test_intersect, glm::vec3(-99,-99,-99)))) {
            points.push_back(glm::vec3(test_intersect));
            //std::cout << "test intersect for edge " << i/2 << ": " << test_intersect.x << " " << test_intersect.y << " " << test_intersect.z << "\n";
        }
    }
    // find the centroid of the intersections
    glm::vec3 centroid = find_centroid(points);
    // sort points
    insertion_sort(points, centroid);
    // insert the centroid point
    points.insert(points.begin(), centroid);
    /*
    std::cout << "after sorting" << "\n";
    for (int i = 0; i < points.size(); i++) {
        std::cout << "points " << i << ": " << points[i].x << " " << points[i].y << " " << points[i].z << "\n";
    }
    std::cout << "\n\n";
    */
    return points;
}


// compute the 2D centroid of the points
glm::vec3 find_centroid(std::vector<glm::vec3> points)
{
    float center_x = 0.0f;
    float center_y = 0.0f;
    float center_z = points[0].z;
    for (int i = 0; i < points.size(); i++) {
        center_x += points[i].x;
        center_y += points[i].y;
    }
    center_x = center_x / points.size();
    center_y = center_y / points.size();
    return glm::vec3(center_x, center_y, center_z);
}


// helper method to sort points clockwise/counter-clockwise on a polygon
bool less(glm::vec3 a, glm::vec3 b, glm::vec3 center)
{
    if ((a.x - center.x >= 0) && (b.x - center.x < 0))
        return true;
    if ((a.x - center.x < 0) && (b.x - center.x >= 0))
        return false;
    if ((a.x - center.x == 0) && (b.x - center.x == 0)) {
        if ((a.y - center.y >= 0) || (b.y - center.y >= 0))
            return a.y > b.y;
        return b.y > a.y;
    }
    // compute the cross product of vectors (center -> a) x (center -> b)
    float det = (a.x - center.x) * (b.y - center.y) - (b.x - center.x) * (a.y - center.y);
    if (det < 0)
        return true;
    if (det > 0)
        return false;
    // points a and b are on the same line from the center
    // check which point is closer to the center
    float d1 = (a.x - center.x) * (a.x - center.x) + (a.y - center.y) * (a.y - center.y);
    float d2 = (b.x - center.x) * (b.x - center.x) + (b.y - center.y) * (b.y - center.y);
    return d1 > d2;
}

// insertion sort algorithm
void insertion_sort(std::vector<glm::vec3> &points, glm::vec3 center) {
    int i, j;
    for (i = 1; i < points.size(); i++) {
        glm::vec3 temp(points[i]);
        for (j = i; j > 0 && !less(points[j-1], temp, center); j--) {
            points[j] = points[j-1];
        }
        points[j] = temp;
    }
}

// generate the vertices positions in object space, and indices for EBO
void gen_pos_indices(std::vector<glm::vec3> &original_pos, std::vector<float> &positions, std::vector<unsigned int> &indices, glm::mat4 &matrix)
{
    // positions
    for (int i = 0; i < original_pos.size(); i++) {
        original_pos[i] = glm::vec3(matrix * glm::vec4(original_pos[i], 1.0));
        positions.push_back(original_pos[i].x);
        positions.push_back(original_pos[i].y);
        positions.push_back(original_pos[i].z);
    }
    // indices
    for (int i = 1; i < original_pos.size(); i++) {
        indices.push_back(0);
        if (i == original_pos.size() - 1) {
            indices.push_back(original_pos.size() - 1);
            indices.push_back(1);
        }
        else {
            indices.push_back(i);
            indices.push_back(i + 1);
        }
    }
}

// load 3D texture data
GLubyte * load_3d_raw_data(std::string texture_path, glm::vec3 dimension) {
    size_t size = dimension[0] * dimension[1] * dimension[2];
    
    FILE *fp;
    GLubyte *data = new GLubyte[size];              // 8bit
    if (!(fp = fopen(texture_path.c_str(), "rb"))) {
        std::cout << "Error: opening .raw file failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "OK: open .raw file successed" << std::endl;
    }
    if (fread(data, sizeof(char), size, fp) != size) {
        std::cout << "Error: read .raw file failed" << std::endl;
        exit(1);
    }
    else {
        std::cout << "OK: read .raw file successed" << std::endl;
    }
    fclose(fp);
    return data;
}

