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

enum render_type {
    Render1 = 0,
    Render2,
    Render3
};

enum culling_type {
    Culling1 = 0,
    Culling2
};

enum rotate_about_axis {
    Axis1 = 0,
    Axis2,
    Axis3
};

enum shading_type {
    Shading1 = 0,
    Shading2
};

enum depth_type {
    Depth1 = 0,
    Depth2
};

// Window dimensions
const GLuint SCR_WIDTH = 1024, SCR_HEIGHT = 768;

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
culling_type cullingType = Culling1;
Color colval(1.0f, 0.5f, 0.3f, 1.f);

float shininess = 32.0f;
bool pointLightFlag = true;
bool dirLightFlag = false;
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

// rotate attributes
rotate_about_axis rotateAxis = Axis1;
rotate_about_axis lastRotateAxis = Axis1;
float rotateDeg = 0.0;

shading_type shadingType = Shading1;
depth_type depthType = Depth1;

// light position
glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 defaultLightPos = lightPos;

// turn on/off diffuse mapping and normal mapping
unsigned int diffuseMap;
unsigned int normalMap;
bool diffuseMapFlag = true;
bool normalMapFlag = true;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 3", nullptr, nullptr);
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
    gui->addVariable("z near", zNear)->setSpinnable(true);
    gui->addVariable("z far", zFar)->setSpinnable(true);
    gui->addVariable("Culling type", cullingType, enabled)->setItems({"CCW", "CW"});
    gui->addVariable("Shading type", shadingType, enabled)->setItems({"SMOOTH", "FLAT"});
    gui->addVariable("Depth type", depthType, enabled)->setItems({"LESS", "ALWAYS"});
    gui->addVariable("Model name", modelName);
    gui->addButton("Reload model", []() {
        resetValues(); setupDraw();
        //gui->refresh();
        //gui2->refresh();
    });
    gui->addButton("Reset light", []() {
        lightPos = defaultLightPos;
        rotateDeg = 0.0;
    });
    
    // create another Selector Bar
    FormHelper *gui2 = new FormHelper(screen);
    ref<Window> nanoguiWindow2 = gui2->addWindow(Eigen::Vector2i(230, 10), "Selectors 2");
    
    gui2->addGroup("Lighting");
    gui2->addVariable("Object color", colval);
    gui2->addVariable("Object shininess", shininess)->setSpinnable(true);
    
    gui2->addVariable("Dir light status", dirLightFlag);
    gui2->addVariable("Dir light X", dirLightX)->setSpinnable(true);
    gui2->addVariable("Dir light Y", dirLightY)->setSpinnable(true);
    gui2->addVariable("Dir light Z", dirLightZ)->setSpinnable(true);
    gui2->addVariable("Dir light ambient color", dirLightAmbient);
    gui2->addVariable("Dir light diffuse color", dirLightDiffuse);
    gui2->addVariable("Dir light specular color", dirLightSpecular);
    
    gui2->addVariable("Point light status", pointLightFlag);
    gui2->addVariable("Point light ambient color", pointLightAmbient);
    gui2->addVariable("Point light diffuse color", pointLightDiffuse);
    gui2->addVariable("Point light specular color", pointLightSpecular);
    
    gui2->addVariable("Rotate camera", rotateCam);
    gui2->addVariable("Point light rotate about", rotateAxis, enabled)->setItems({"X", "Y", "Z"});
    
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
    Shader objectShader("shader/object.vs", "shader/object.fs");
    
    // setup the vertices and buffers
    setupDraw();
    
    // shader configuration
    // --------------------
    objectShader.Use();
    objectShader.setInt("material.diffuse", 0);
    objectShader.setInt("normalMap", 1);
    
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // enable depth test
        glEnable(GL_DEPTH_TEST);
        // set depth type
        switch(depthType)
        {
            case Depth1:
                glDepthFunc(GL_LESS);
                break;
            case Depth2:
                glDepthFunc(GL_ALWAYS);
                break;
        }
        
        // update model path (if changed)
        if (lastModelName.compare(modelName) != 0)
        {
            resetValues();
            setupDraw();
            // refresh the gui
            gui->refresh();
            gui2->refresh();
        }
        //lastModelPath = modelPath;
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
        
        // get matrix's uniform location and set matrices
        // set model, view and projection matrices
        objectShader.Use();
        
        objectShader.setMat4("model", model);
        objectShader.setMat4("view", view);
        objectShader.setMat4("projection", projection);
        
        // set lighting attributes for object shader
        // set light position based on current rotation
        float radius = glm::abs(defaultLightPos.z);
        switch(rotateAxis)
        {
            case Axis1:
                lightPos = glm::vec3(defaultLightPos.x, defaultLightPos.y+radius*glm::sin(rotateDeg), radius*glm::cos(rotateDeg));
                break;
            case Axis2:
                lightPos = glm::vec3(defaultLightPos.x+radius*glm::sin(rotateDeg), defaultLightPos.y, radius*glm::cos(rotateDeg));
                break;
            case Axis3:
                // compute the radius for the rotation, set it at max(stretchX, stretchY)
                float aveStretch = glm::max(stretchX, stretchY);
                lightPos = glm::vec3(defaultLightPos.x+aveStretch*glm::sin(rotateDeg), defaultLightPos.y+aveStretch*glm::cos(rotateDeg), defaultLightPos.z/2);
                break;
        }
        // reset rotate degree if we change the rotation axis.
        if (lastRotateAxis != rotateAxis) { rotateDeg = 0.0; }
        lastRotateAxis = rotateAxis;
        // increase the rotation degree, set it at 0.005, roughly the real-time duration (compared to glfwGetTime() method).
        if(rotateCam)
        {
            rotateDeg += 0.005;
        }
        
        // set shading type: smooth vs flat shading.
        bool flatFlag = false;
        switch(shadingType)
        {
            case Shading1:
                flatFlag = false;
                break;
            case Shading2:
                flatFlag = true;
                break;
        }
        
        // send data to the object shader
        objectShader.setBool("flatFlag", flatFlag);
        objectShader.setBool("dirLightFlag", dirLightFlag);
        objectShader.setBool("pointLightFlag", pointLightFlag);
        objectShader.setVec3("pointLight.position", lightPos);
        objectShader.setVec3("lightPos", lightPos);
        objectShader.setVec3("viewPos", viewPos);
        
        // set point light attributes
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 pointDiffuseColor = glm::vec3(pointLightDiffuse[0], pointLightDiffuse[1], pointLightDiffuse[2]);
        glm::vec3 pointAmbientColor = glm::vec3(pointLightAmbient[0], pointLightAmbient[1], pointLightAmbient[2]);
        glm::vec3 pointSpecularColor = glm::vec3(pointLightSpecular[0], pointLightSpecular[1], pointLightSpecular[2]);
        objectShader.setVec3("pointLight.ambient", pointAmbientColor);
        objectShader.setVec3("pointLight.diffuse", pointDiffuseColor);
        objectShader.setVec3("pointLight.specular", pointSpecularColor);
        
        // set Directional Light attributes
        dirLightDirection = glm::vec3(dirLightX, dirLightY, dirLightZ);
        glm::vec3 lightDir = camToWorld(dirLightDirection, cameraOrigin, cameraRight, cameraUp, cameraFront);
        glm::vec3 dirDiffuseColor = glm::vec3(dirLightDiffuse[0], dirLightDiffuse[1], dirLightDiffuse[2]);
        glm::vec3 dirAmbientColor = glm::vec3(dirLightAmbient[0], dirLightAmbient[1], dirLightAmbient[2]);
        glm::vec3 dirSpecularColor = glm::vec3(dirLightSpecular[0], dirLightSpecular[1], dirLightSpecular[2]);
        objectShader.setVec3("dirLight.direction", lightDir);
        objectShader.setVec3("dirLight.ambient", dirAmbientColor);
        objectShader.setVec3("dirLight.diffuse", dirDiffuseColor);
        objectShader.setVec3("dirLight.specular", dirSpecularColor);
        
        // object color
        // glm::vec4 myColor = glm::vec4(colval[0], colval[1], colval[2], colval[3]);
        // material properties
        objectShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
        objectShader.setFloat("material.shininess", shininess);
        
        // set culling face
        glEnable( GL_CULL_FACE );
        // set culling type
        switch(cullingType)
        {
            case Culling1:
                glFrontFace(GL_CCW);
                break;
            case Culling2:
                glFrontFace(GL_CW);
                break;
        }

        // bind texture
        objectShader.setBool("diffuseMapFlag", diffuseMapFlag);
        objectShader.setBool("normalMapFlag", normalMapFlag);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        
        // render object
        glBindBuffer(GL_ARRAY_BUFFER, objectVBO);
        glBindVertexArray(objectVAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLint)vertices.size());

        // draw the GUI
        screen->drawWidgets();
        
        // Swap the screen buffers
        glfwSwapBuffers(window);

    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &objectVAO);
    glDeleteVertexArrays(1, &lampVAO);
    glDeleteBuffers(1, &objectVBO);
    glDeleteBuffers(1, &lampVBO);
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
