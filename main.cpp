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

#include "glm/gtx/string_cast.hpp"
#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 windmillRot;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;


// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint directional;
GLint position;
GLint position1;
GLint dens;
GLint caseLoc;
GLuint shadowFbo, depth;
GLuint normalMatrixLoc2;


//constants for fog
int enable = 0, button = 0;
float density = 1.2f;
//int animation = 0;
int animation = 1;


// camera
gps::Camera myCamera(
    glm::vec3(4.748063f, 4.724739f, 3.044693f),
    glm::vec3(4.918767f, 6.798837f, 4.879189f),  // Punctul către care privește camera
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.5f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D horse;
gps::Model3D windmill;
GLfloat angle;
GLfloat angle_animation = 0.1f;

// shaders
gps::Shader myBasicShader;
gps::Shader depthShader;

int glWindowWidth = 1920;
int glWindowHeight = 1017;

float lastX = glWindowWidth ;
float lastY = glWindowHeight ;
bool firstMouse = true;
const float sensitivity = 0.15f;
float yaw = -90.0f;
float pitch = 0.0f;
float red = 1.0f;
float green = 1.0f;
float blue = 1.0f;
glm::vec3 caseForLight = glm::vec3(1.0f, 1.0f, 1.0f);
const unsigned int shadow_w = 7000, shadow_h = 7000;
float initFog = 0.2f;


GLenum glCheckError_(const char *file, int line)
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
    glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;


    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    myCamera.rotate(pitch, yaw);
    myBasicShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_F) == GLFW_PRESS && button == 1)
    {
        enable = !enable;
        button = 0;
        initFog = 0.2f;
    }
    if (pressedKeys[GLFW_KEY_G]) {
        initFog += 0.007f;
    }


    if (pressedKeys[GLFW_KEY_N]) {
        if (red >= 0.0f && green >= 0.0f && blue >= 0.0f) {
            red -= 0.005f;
            green -= 0.005f;
            blue -= 0.005f;
        }
    }

    if (pressedKeys[GLFW_KEY_B]) {
        if (red <= 1.0f && green <= 1.0f && blue <= 1.0f) {
            red += 0.005f;
            green += 0.005f;
            blue += 0.005f;
        }
    }

    if (pressedKeys[GLFW_KEY_U]) {
        // Apăsată tasta 'U', setează modul solid
        myWindow.setSolidMode();
    }

    if (pressedKeys[GLFW_KEY_I]) {
        // Apăsată tasta 'I', setează modul wireframe
        myWindow.setWireframeMode();
    }

    if (pressedKeys[GLFW_KEY_O]) {
        // Apăsată tasta 'P', setează modul poligonal (puncte)
        myWindow.setPointMode();
    }

    if (pressedKeys[GLFW_KEY_L]) {
        caseForLight = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    if (pressedKeys[GLFW_KEY_K]) {
        caseForLight = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_F) == GLFW_RELEASE)
    {
        button = 1;
    }
   /* if (pressedKeys[GLFW_KEY_8]) {
        animation = 1;
    }
    if (animation == 1) {
        glm::vec3 position = myCamera.getPosition();
        float a = position.y;
        position.y = position.z;
        position.z = -a;
        glm::mat4 animationTr = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 animationRot = glm::rotate(glm::mat4(1.0f), glm::radians(angle_animation), glm::vec3(0, 1, 0));
        glm::mat4 animationTrBack = glm::translate(glm::mat4(1.0f), -position);
        glm::mat4 animationMatrix = model * animationTr * animationRot * animationTrBack;
        normalMatrix = glm::mat3(glm::inverseTranspose(view * animationMatrix));
    }*/
}

void initOpenGLWindow() {
    myWindow.Create(1920 , 1017, "OpenGL Project Core");
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
    //teapot.LoadModel("models/teapot/teapot20segUT.obj");
    teapot.LoadModel("models/proiect.obj");
    horse.LoadModel("models/horse.obj");
    windmill.LoadModel("models/windmillEl.obj");
    
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 2000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    lightColor = glm::vec3(red, green, blue); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    caseLoc = glGetUniformLocation(myBasicShader.shaderProgram, "caseLight");
    glUniform3fv(caseLoc,1,glm::value_ptr(caseForLight));

       position = glGetUniformLocation(myBasicShader.shaderProgram, "pos");
       glm::vec3 pos1 = glm::vec3(9.584325f, 8.536662f, -95.028366f);
        glUniform3fv(position, 1, glm::value_ptr(pos1));

        position1 = glGetUniformLocation(myBasicShader.shaderProgram, "pos1");
        glm::vec3 pos2 = glm::vec3(40.584325f, 8.536662f, -95.028366f);
        glUniform3fv(position1, 1, glm::value_ptr(pos2));

}

glm::mat4 computeLightSpaceMatrix() {
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 6.0f;
    glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}


void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    //fog
    shader.setInt("fogEnable", enable);

  

    // draw teapot
    //teapot.Draw(shader);
    teapot.Draw(shader);
}

float horseX = 0.0f, horseZ = 0.0f, horseY = 0.0f;
float horseSpeed = 0.5f;

void moveHorse() {

    if (pressedKeys[GLFW_KEY_UP]) {
        horseZ -= horseSpeed;
    }

    if (pressedKeys[GLFW_KEY_DOWN]) {
        horseZ += horseSpeed;
    }

    if (pressedKeys[GLFW_KEY_LEFT]) {
        horseX -= horseSpeed;
    }

    if (pressedKeys[GLFW_KEY_RIGHT]) {
        horseX += horseSpeed;
    }

    if (pressedKeys[GLFW_KEY_2]) {
        horseY -= horseSpeed;
    }

    if (pressedKeys[GLFW_KEY_5]) {
        horseY += horseSpeed;
    }
}
void renderHorse(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    glm::mat4 horseTr = glm::translate(model, glm::vec3(horseX, horseY, horseZ));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(horseTr));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    horse.Draw(shader);
}

float windmillRotationAngle = 0.0f;

void rotateWindmill() {
    if (pressedKeys[GLFW_KEY_R]) {
        windmillRotationAngle += 2.0f;
    }
}

void renderWindmill(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    windmillRot = translate(glm::mat4(1.0f), glm::vec3(190.519f, 16.7732f, 47.3917f));
    windmillRot = glm::rotate(model, glm::radians(windmillRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    windmillRot = translate(windmillRot, glm::vec3(-190.519f, -16.7732f, -47.3917f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(windmillRot));

    glm::mat3 normalMatrix2 = (glm::inverseTranspose(view * windmillRot));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix2));

    windmill.Draw(shader);
}

void renderShadow() {
    depthShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceTrMatrix"),1,GL_FALSE,glm::value_ptr(computeLightSpaceMatrix()));
    glViewport(0, 0, shadow_w, shadow_h);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//render the scene

	// render the teapot
	//renderTeapot(myBasicShader);
    renderTeapot(myBasicShader);
    renderHorse(myBasicShader);
    renderWindmill(myBasicShader);
    moveHorse();
    rotateWindmill();

    //set light color
    lightColor = glm::vec3(red, green, blue); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    caseLoc = glGetUniformLocation(myBasicShader.shaderProgram, "caseLight");
    glUniform3fv(caseLoc, 1, glm::value_ptr(caseForLight));

    float fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "densityC");
    glUniform1f(fogLoc, initFog);

    /*std::cout << glm::to_string(myCamera.getPosition())<< std::endl;
    std::cout << "yaw :" << yaw << std::endl;
    std::cout << "pitch: " << pitch << std::endl;*/


}

void initShadowsForScene() {
    glGenFramebuffers(1, &shadowFbo);

    glGenTextures(1, &depth);
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_w, shadow_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //unbind until ready to use
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
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
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}



	cleanup();

    return EXIT_SUCCESS;
}


