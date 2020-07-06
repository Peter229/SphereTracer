#pragma once

#include "windowGL.h"

#include "shader.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <memory>
#include <fstream>
#include <strstream>

class Game {

public:
	Game();
	~Game();

	void start();

	void loop();

	void update(GLboolean* Keys, double* mousePos, float deltaTime);
	void logic();

	void tick();

	void render();

	void cleanUp();

	void createComputeShader();

private:
	Camera* camera;
	Shader* shader;
	
	GLuint computeShader;

	bool firstMouse;
	double lastX, lastY;
	int windowWidth, windowHeight;
	glm::vec3 startMouse, endMouse;

	float deltaTime;
	float lastFrame;

	float deltaTimeT;
	float deltaTime2;
	double lastTimeA;
	double lastTimeB;
	int nbFrames;

	std::vector<float> windowVerts;
	std::vector<float> windowUVs;
	unsigned int windowVAO, windowVBO, uvVBO;

	int tex_w = 800, tex_h = 600;
	GLuint tex_output;
};