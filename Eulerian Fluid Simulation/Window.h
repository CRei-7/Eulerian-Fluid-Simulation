#pragma once

#include <iostream>

#include<glad/glad.h>
#include<GLFW/glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);

	int initialize();

	GLint getBufferWidth() { return bufferWidth; }
	GLint getBufferHeight() { return bufferHeight; }

	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }

	bool* getKeys() { return keys; }
	GLfloat getxChange();
	GLfloat getyChange();

	void swapBuffers() { glfwSwapBuffers(mainWindow); }

	GLFWwindow* getGLFWwindow() { return mainWindow; }

	void toggleCursorVisibility();

	~Window();

private:
	GLFWwindow* mainWindow;

	GLint width, height;
	GLint bufferWidth, bufferHeight;//Get Buffer Size Information

	//For checking if key is pressed
	bool keys[1024]; //1024 for ASCII

	GLfloat lastX, lastY, xChange, yChange; //to compare coordinates to calculate changes
	bool mouseFirstMoved; //for first mouse movement

	bool cursorVisible; // Track cursor visibility state

	void createCallbacks();//for key callbacks

	//static is to be used for callback, using just void means the function is only applicable for a single object
	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);//these variables are required for the callback
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);
};

