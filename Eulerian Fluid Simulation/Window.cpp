#include "Window.h"

Window::Window() {
    width = 800;
    height = 600;

    for (size_t i = 0; i < 1024; i++) {
        keys[i] = 0;
    }

    mouseFirstMoved = true;

    //This is so that camera does not start at a random direction
    xChange = 0.0f;
    yChange = 0.0f;

    cursorVisible = false; // Start with cursor hidden
}

Window::Window(GLint windowWidth, GLint windowHeight) {
    width = windowWidth;
    height = windowHeight;

    for (size_t i = 0; i < 1024; i++) {
        keys[i] = 0;
    }

    mouseFirstMoved = true;

    //This is so that camera does not start at a random direction
    xChange = 0.0f;
    yChange = 0.0f;

    cursorVisible = false; // Start with cursor hidden
}

int Window::initialize() {
    if (!glfwInit())
    {
        printf("GLFW Initialization Failed");
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    //Core Profile = No backwards compatibility
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    mainWindow = glfwCreateWindow(width, height, "GPU Particle System", NULL, NULL);
    if (!mainWindow) {
        printf("GLFW Window Creation Failed");
        glfwTerminate();
        return 1;
    }

    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

    //Set context for GLEW to use
    glfwMakeContextCurrent(mainWindow);

    //Handle Key and Mouse Inputs
    createCallbacks();
    glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//This is for disabling cursor

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);//For ensuring depth test

    //Setup Viewport
    glViewport(0, 0, bufferWidth, bufferHeight);

    //It associates a user-defined pointer (any void*) with a GLFW window.
    glfwSetWindowUserPointer(mainWindow, this);//(window, user), this sets a pointer which is being used in handleKeys function

    return 0;
}

void Window::createCallbacks() {
    glfwSetKeyCallback(mainWindow, handleKeys);//if key is pressed in mainWindow, call handleKeys function
    glfwSetCursorPosCallback(mainWindow, handleMouse);//for cursor
}

GLfloat Window::getxChange() {//This function is not really that necessary
    GLfloat theChange = xChange;
    xChange = 0.0f;
    return theChange;
}

GLfloat Window::getyChange() {//This function is not really that necessary
    GLfloat theChange = yChange;
    yChange = 0.0f;
    return theChange;
}

void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode) {//No need to use static here
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));//Getting the pointer to the window whose inputs are being handled 

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Toggle cursor on C key pressed (edge only: GLFW_PRESS)
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        theWindow->toggleCursorVisibility();
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            theWindow->keys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            theWindow->keys[key] = false;
        }
    }
}

void Window::handleMouse(GLFWwindow* window, double xPos, double yPos) {
    Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (theWindow->cursorVisible) {
        return; // Ignore mouse movement when cursor is visible
    }

    GLfloat xPosition = static_cast<GLfloat>(xPos);
    GLfloat yPosition = static_cast<GLfloat>(yPos);

    if (theWindow->mouseFirstMoved) {
        theWindow->lastX = xPosition;
        theWindow->lastY = yPosition;
        theWindow->mouseFirstMoved = false;
    }

    theWindow->xChange = xPosition - theWindow->lastX;
    theWindow->yChange = theWindow->lastY - yPosition; //Flip this if you want inverted controls

    theWindow->lastX = xPosition;
    theWindow->lastY = yPosition;
}

void Window::toggleCursorVisibility() {
    if (cursorVisible) {
        glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor
        cursorVisible = false;
        mouseFirstMoved = true; // Reset mouse movement tracking when hiding the cursor
    }
    else {
        glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Show cursor
        cursorVisible = true;
    }
}

Window::~Window() {
    glfwDestroyWindow(mainWindow);
    glfwTerminate();
}