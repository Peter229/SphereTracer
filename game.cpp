#include "game.h"

Game::Game() {

}
 
Game::~Game() {

}

void Game::start() {

	if (WindowGL::start() == -1) {
		std::cout << "Failed to create window\n";
	}

	deltaTime = 0.0f;
	lastFrame = 0.0f;

	deltaTimeT = 0;
	deltaTime2 = 0;
	lastTimeA = glfwGetTime();
	lastTimeB = glfwGetTime();
	nbFrames = 0;

	firstMouse = true;

	this->windowWidth = SCR_WIDTH;
	this->windowHeight = SCR_HEIGHT;

	endMouse = startMouse = glm::vec3(0.0f, 0.0f, 0.0f);

	//Load Shaders
	shader = new Shader("Shaders/Vertex/vertexShader.txt", "Shaders/Fragment/fragmentShader.txt");

	//Create Camera
	camera = new Camera(glm::vec3(0.0f, 1.0f, 0.0f));

	//Set up window verts
	windowVerts.push_back(-1.0f);
	windowVerts.push_back(-1.0f);
	windowVerts.push_back(0.0f);
	windowVerts.push_back(0.0f);

	windowVerts.push_back(1.0f);
	windowVerts.push_back(-1.0f);
	windowVerts.push_back(1.0f);
	windowVerts.push_back(0.0f);

	windowVerts.push_back(-1.0f);
	windowVerts.push_back(1.0f);
	windowVerts.push_back(0.0f);
	windowVerts.push_back(1.0f);

	windowVerts.push_back(1.0f);
	windowVerts.push_back(1.0f);
	windowVerts.push_back(1.0f);
	windowVerts.push_back(1.0f);

	windowVerts.push_back(-1.0f);
	windowVerts.push_back(1.0f);
	windowVerts.push_back(0.0f);
	windowVerts.push_back(1.0f);

	windowVerts.push_back(1.0f);
	windowVerts.push_back(-1.0f);
	windowVerts.push_back(1.0f);
	windowVerts.push_back(0.0f);

	//Set up window VAO
	glGenVertexArrays(1, &windowVAO);
	glGenBuffers(1, &windowVBO);

	glBindVertexArray(windowVAO);

	glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * windowVerts.size(), &windowVerts[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//https://antongerdelan.net/opengl/compute.html

	//Set up render image
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max local work group invocations %i\n", work_grp_inv);

	createComputeShader();
}

void Game::loop() {

	while (!glfwWindowShouldClose(WindowGL::getWindow())) {

		double currentTimeA = glfwGetTime();
		nbFrames++;
		if (currentTimeA - lastTimeA >= 1.0) {
			printf("%f ms/frame\n%d Frames Per Second\n", 1000.0 / double(nbFrames), nbFrames);
			nbFrames = 0;
			lastTimeA += 1.0;
		}

		//Delta Time To Get How Fast The Game Should Run
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Get Inputs
		WindowGL::update();
		
		update(WindowGL::getKeys(), WindowGL::getMousePos(), deltaTime);

		if (currentTimeA - lastTimeB >= 0.0166) { //Call physics 60 times a second

			lastTimeB += 0.0166;

			tick();
		}

		render();

		//Go To Next Frame
		glfwSwapBuffers(WindowGL::getWindow());
	}
}

void Game::update(GLboolean* Keys, double* mousePos, float deltaTime) {

	//Mouse Movement
	double xpos = mousePos[0];
	double ypos = mousePos[1];

	if (glfwGetInputMode(WindowGL::getWindow(), GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {

		firstMouse = true;
	}
	else {

		if (firstMouse) {

			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera->ProcessMouseMovement(xoffset, yoffset);
	}
	
	if (Keys[GLFW_KEY_W] == GLFW_PRESS) {
		camera->ProcessKeyboard(FORWARD, deltaTime);
	}
	if (Keys[GLFW_KEY_S] == GLFW_PRESS) {
		camera->ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (Keys[GLFW_KEY_A] == GLFW_PRESS) {
		camera->ProcessKeyboard(LEFT, deltaTime);
	}
	if (Keys[GLFW_KEY_D] == GLFW_PRESS) {
		camera->ProcessKeyboard(RIGHT, deltaTime);
	}

	logic();
}

void Game::logic() {

}

void Game::tick() {

}

void Game::render() {

	glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 400.0f);
	glm::mat4 view = camera->GetViewMatrix();
	glm::mat4 invProjVeiw = glm::inverse(projection * view);
	
	glUseProgram(computeShader);

	glUniform3fv(glGetUniformLocation(computeShader, "pos"), 1, glm::value_ptr(camera->Position));
	glUniform3fv(glGetUniformLocation(computeShader, "dir"), 1, glm::value_ptr(camera->Front));
	glUniform3fv(glGetUniformLocation(computeShader, "up"), 1, glm::value_ptr(camera->Up));
	glUniform3fv(glGetUniformLocation(computeShader, "right"), 1, glm::value_ptr(camera->Right));
	glUniform1f(glGetUniformLocation(computeShader, "time"), glfwGetTime());

	glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.2f, 0.2f, 0.9f, 1.0f);

	glm::mat4 model = glm::mat4(1.0f);

	//Render With Basic Shader
	shader->use();

	glBindVertexArray(windowVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Game::cleanUp() {

	WindowGL::end();
}

void Game::createComputeShader() {

	std::string computeShaderCode;
	std::ifstream computeShaderFile;

	computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try { //Try And Load Shader Code
		computeShaderFile.open("Shaders/computeShader.txt");
		std::stringstream computeShaderStream;

		computeShaderStream << computeShaderFile.rdbuf();

		computeShaderFile.close();

		computeShaderCode = computeShaderStream.str();
	} catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ \n";
	}

	const char* csComputeShaderCode = computeShaderCode.c_str();

	//Compile And Check For Errors
	unsigned int compute;
	int success;
	char infoLog[512];

	//Compile compute
	compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &csComputeShaderCode, NULL);
	glCompileShader(compute);

	//Check compute
	glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
	if (!success) {

		glGetShaderInfoLog(compute, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << "\n";
	};

	//Compile Shader
	computeShader = glCreateProgram();
	glAttachShader(computeShader, compute);
	glLinkProgram(computeShader);

	//Check Shader
	glGetProgramiv(computeShader, GL_LINK_STATUS, &success);
	if (!success) {

		glGetProgramInfoLog(computeShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
	}

	glDeleteShader(compute);
}