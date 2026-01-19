#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <glm/vec4.hpp>



class GameObject {
    float vertices[36] = {
        -0.5f , -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f  
    };

public:
	unsigned int VBO = 0, VAO = 0, EBO = 0;
	glm::mat4 model = glm::mat4(1.f);
    GLint modelLoc;
    unsigned int shaderProgram;
    GameObject(unsigned int shaderProgram) : shaderProgram{shaderProgram} {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // �������� VAO
        glBindVertexArray(VAO);

        // ����������� ������ � �����
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // ��������� ��������� ������
        // ������� 0: ������� (3 float)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // ������� 1: ����� (3 float) - �������� �� 3 float
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // �������
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        modelLoc = glGetUniformLocation(shaderProgram, "model");
	}

	void virtual render() {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
	}

};