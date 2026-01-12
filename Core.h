#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);


class Core
{
	unsigned int SCR_WIDTH = 800;
	unsigned int SCR_HEIGHT = 600;
public:
	const static Core* _instance;
	GLFWwindow* window;
	glm::vec4 colorBackGround = glm::vec4(0.f,0.f,0.f,1);
private:
	Core() {
		initOpenGL();
		mainLoop();
	}

	void initOpenGL();
	void mainLoop();
public:

	void processInput(GLFWwindow* window);
};

// glfw: всякий раз, когда изменяются размеры окна (пользователем или оперионной системой), вызывается данная callback-функция
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// Убеждаемся, что окно просмотра соответствует новым размерам окна.
	// Обратите внимание, ширина и высота будут значительно больше, чем указано, на Retina-дисплеях
	glViewport(0, 0, width, height);
}