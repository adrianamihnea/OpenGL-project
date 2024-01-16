#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
glm::mat4 modelBlenderScene;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;
glm::mat4 sunLightRotation;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 sunLight;

// shader uniform locations
GLint modelLoc;
GLint modelLocTeapot;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLuint sunLightLocation;


// camera
gps::Camera myCamera(
    glm::vec3(-38.381f, -0.05413f, 19.596f), 
    glm::vec3(-38.399f, -0.56639f, 16.3f), 
    glm::vec3(0.0f, 1.0f, 0.0f)
);

GLfloat mouseSpeed = 0.1f;
GLfloat cameraSpeed = 0.01f;
//GLfloat cameraSpeed = 5.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D scene;
GLfloat angle;
GLfloat angleY;

bool startVisualisation = false;
float sceneAngle = 0.0f;

// lights
float angleDirectionalLight = 0.0f;
float anglePointLight = 0.0f;

// shaders
gps::Shader myBasicShader;
//gps::Shader depthMapShader;
float previousX = 400, previousY = 300;
float yaw = -90.0f, pitch;

GLuint shadowMapFBO;

bool directionalLightEnabled = true;
bool pointLightEnabled = false;
bool fogEnabled = false;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    GLfloat X = xpos - previousX;
    GLfloat Y = previousY - ypos;

    previousX = xpos;
    previousY = ypos;

    X *= mouseSpeed;
    Y *= mouseSpeed;

    yaw += X;
    pitch += Y;

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angleY -= 0.2f;
        // update model matrix
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0, 1, 0));
        // update normal matrix
        //normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angleY += 0.2f;
        // update model matrix
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0, 1, 0));
        // update normal matrix
       // normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_J]) { // MOVE DOWN
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_U]) { // MOVE UP
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    // -- VIEWS

    if (pressedKeys[GLFW_KEY_Y]) { // SOLID VIEW
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_T]) { // WIREFRAME VIEW
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_G]) { // POLYGONAL VIEW
        glDisable(GL_POINT_SMOOTH);
        glEnable(GL_MULTISAMPLE);
    }

    if (pressedKeys[GLFW_KEY_H]) { // SMOOTH VIEW
        glDisable(GL_MULTISAMPLE);
        glEnable(GL_POINT_SMOOTH);
    }
   
    // -- end VIEWS

    // SCENE VIZUALIZATION

     // UNIVERSE VISUALISATION

    if (pressedKeys[GLFW_KEY_P]) { // START VISUALISATION
        startVisualisation = true;
    }

    if (pressedKeys[GLFW_KEY_O]) { // STOP VISUALIZATION
        startVisualisation = false;
    }

    // end SCENE VIZUALIZATION

    // LIGHTS

    if (pressedKeys[GLFW_KEY_K]) { // directional light
        directionalLightEnabled = !directionalLightEnabled;
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "directionalLightEnabled"), directionalLightEnabled);
    }

    if (pressedKeys[GLFW_KEY_L]) { // point light
        pointLightEnabled = !pointLightEnabled;
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "pointLightEnabled"), pointLightEnabled);
    }

    if (pressedKeys[GLFW_KEY_F]) { // fog
        fogEnabled = !fogEnabled;
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fogEnabled"), fogEnabled);
    }

    // end LIGHTS
}

void viewSceneAnimation() {

    if (startVisualisation) {
        sceneAngle += 0.2f;
        myCamera.startVisualization(sceneAngle);

    }
}

void processRotation() {
    myCamera.rotate(pitch, yaw);
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    scene.LoadModel("models/SCENE/scene.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    /*depthMapShader.loadShader(
        "shaders/depthMap.vert",
        "shaders/depthMap.frag"
    );*/
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    //initialize the model matrix
    //model = glm::mat4(1.0f);
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(55.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(1.0f, 0.0f, 0.0f); // originally on Y
    //lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angleDirectionalLight), glm::vec3(0.0f, 0.0f, 1.0f));
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // point light, on the sun
    sunLight = glm::vec3(10.0, 10.0, 10.0);
    sunLightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angleDirectionalLight), glm::vec3(0.0f, 0.0f, 1.0f));
    sunLightLocation = glGetUniformLocation(myBasicShader.shaderProgram, "lightPosEye");
    glUniform3fv(sunLightLocation, 1, glm::value_ptr(sunLight));

   /* lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));*/
}


void renderBlenderScene(gps::Shader shader) {

    // select active shader program
    shader.useShaderProgram();

   /* model = glm::translate(model, glm::vec3(0, 1.9, 0));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));*/
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send scene normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    scene.Draw(shader);

}

void renderScene() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render the scene
    //model = glm::mat4(1.0f);
    //modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    //render the scene to the depth buffer
   /* depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);*/

    // render the elements of the scene
    /*renderGrass(myBasicShader);
    renderTable(myBasicShader);
    renderTeapot(myBasicShader);
    renderSakuraTree(myBasicShader);
    renderTree(myBasicShader);*/
    renderBlenderScene(myBasicShader);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        //processRotation();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
