#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include<algorithm>
#include "camera.h"
#include"stb_image.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"



#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include"shader.h"

using namespace std;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera* cam);
vector<int> Bresenham(int x0, int y0, int x1, int y1);
vector<int> getTriPoints(int x1, int y1, int x2, int y2, int x3, int y3);
float *convert2vertice(vector<int> v);
vector<int> getCirclePoints(int x0, int y0, int r);
vector<int> getEightSymetricPoint(int originx, int originy, int offsetx, int offsety);
vector<int> getRasterizeTripoints(int x1, int y1, int x2, int y2, int x3, int y3);
vector<int> getLineEquation(int x1, int y1, int x2, int y2, int x3, int y3);

GLFWwindow* openGLallInit();
unsigned int initShaders();
void initImGui(GLFWwindow* window);
void cleanAll();

void renderLoop(GLFWwindow* window);

const float WIDTH = 1000;
const float HEIGHT = 1000;



int main()
{
	GLFWwindow* window = openGLallInit();
	if (window == NULL)
		return -1;

	unsigned int myShaderProgram = initShaders();

	initImGui(window);


	renderLoop(window);

	cleanAll();
	
	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, Camera* cam)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cam->moveForward(0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cam->moveBack(0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cam->moveLeft(0.1);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cam->moveRight(0.1);
	}

	double xpos = 0, ypos = 0;
	glfwGetCursorPos(window, &xpos, &ypos);
	cam->rotate(xpos, ypos);
}

vector<int> Bresenham(int x0, int y0, int x1, int y1)
{
	vector<int> points;
	points.push_back(x0);
	points.push_back(y0);

	int dx = x1 - x0, dy = y1 - y0;

	//���ж�ֱ�ߵķ���Ȼ�����㷨��ʱ����ɼ򵥵ļ�1���ĳɼӷ���ĵ�λ�ƶ�
	int directX = (dx > 0) ? 1 : -1;
	int directY = (dy > 0) ? 1 : -1;
	if (dx < 0) 
		dx = -dx;
	if (dy < 0) 
		dy = -dy;
	if (dx == 0)
		dx = 0.01;
	double m = dy / dx; //б��

	if (m < 1)	//С��1�����Ͽν�������
	{
		int ddy = 2 * dy; //����dy
		int ddiffer = ddy - 2 * dx; //����dy-����dx
		int p = ddy - dx; //p0

		while (x0 != x1) //ֱ����x0��Ϊѭ������xi
		{
			if (p <= 0)
			{
				x0 += directX;
				p += ddy;
			}
			else
			{
				x0 += directX;
				y0 += directY;
				p += ddiffer;
			}
			points.push_back(x0);
			points.push_back(y0);
		}
	}
	else //б�ʴ���1�������������ϵ��ת
	{
		int ddx = 2 * dx; //����dx
		int ddiffer = ddx - 2 * dy; //����dx-����dy
		int p = ddx - dy; //p0

		while (y0 != y1)		//����Ҫ�ж�y,��Ϊб�ʴ���1����y�����
		{
			if (p <= 0)
			{
				y0 += directY;
				p += ddx;
			}
			else
			{
				x0 += directX;
				y0 += directY;
				p += ddiffer;
			}
			points.push_back(x0);
			points.push_back(y0);
		}
	}
	return points;

}

vector<int> getTriPoints(int x1, int y1, int x2, int y2, int x3, int y3) {
	vector<int> totalPoints;
	vector< vector<int> > edgesPoints;
	edgesPoints.push_back(Bresenham(x1, y1, x2, y2));
	edgesPoints.push_back(Bresenham(x1, y1, x3, y3));
	edgesPoints.push_back(Bresenham(x2, y2, x3, y3));
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < edgesPoints[i].size(); j++)
			totalPoints.push_back(edgesPoints[i][j]);
	return totalPoints;
}

vector<int> getEightSymetricPoint(int originx,  int originy, int offsetx, int offsety)
{
	vector<int> temp;
	temp.push_back(offsetx + originx);
	temp.push_back(offsety + originy);
	temp.push_back(-offsetx + originx);
	temp.push_back(offsety + originy);
	temp.push_back(offsetx + originx);
	temp.push_back(-offsety + originy);
	temp.push_back(-offsetx + originx);
	temp.push_back(-offsety + originy);

	temp.push_back(offsety + originx);
	temp.push_back(offsetx + originy);
	temp.push_back(offsety + originx);
	temp.push_back(-offsetx + originy);
	temp.push_back(-offsety + originx);
	temp.push_back(offsetx + originy);
	temp.push_back(-offsety + originx);
	temp.push_back(-offsetx + originy);
	return temp;
}

vector<int> getCirclePoints(int x0, int y0, int r) {
	vector<int> points;
	int x = 0, y = r, d = 3 - 2 * r;
	vector<int> temp = getEightSymetricPoint(x0, y0, x, y);
	points.insert(points.begin(), temp.begin(), temp.end());
	while (x< y) {
		if (d< 0) {
			d = d + 4 * x + 6;
		}
		else {
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
		temp = getEightSymetricPoint(x0, y0, x, y);
		points.insert(points.begin(), temp.begin(), temp.end());
	}
	return points;
}


float* convert2vertice(vector<int> v) {
	int point_num = v.size() / 2;
	float *vertices = new float[point_num * 6]; //����x,y���껹����ȣ�Ȼ������ɫ
	for (int i = 0; i< point_num; i++) {
		// λ��
		vertices[i * 6 + 0] = (float)v[i * 2 + 0] / (float)WIDTH;		//��һ������
		vertices[i * 6 + 1] = (float)v[i * 2 + 1] / (float)HEIGHT;
		vertices[i * 6 + 2] = 0.0f;
		// ��ɫ
		vertices[i * 6 + 3] = 1.0f;
		vertices[i * 6 + 4] = 1.0f;
		vertices[i * 6 + 5] = 1.0f;
	}
	return vertices;
}

GLFWwindow* openGLallInit()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//��������
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "firWindow", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	//�ж��Ƿ����glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}

	//�趨�Ǹ�ʲô�ӿ�
	glViewport(0, 0, WIDTH, HEIGHT);

	//ע�Ỻ�����ʱ�Ļص�����
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	return window;
}

unsigned int initShaders()
{
	const char *vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
		"   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
		"}\0";

	const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D texture1;\n"
		"uniform sampler2D texture2;\n"

		"void main()\n"
		"{\n"
		"   FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);\n"
		"}\n\0";

	//����Դ�����������ɫ��
	unsigned int myVertexShader;
	myVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(myVertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(myVertexShader);

	unsigned int myFragmentShader;
	myFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(myFragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(myFragmentShader);

	//����������󣬽���ɫ��������ȥ��Ȼ�󽫳�������,����Ϳ��԰���ɫ������ɾ����
	unsigned int myShaderProgram;
	myShaderProgram = glCreateProgram();
	glAttachShader(myShaderProgram, myVertexShader);
	glAttachShader(myShaderProgram, myFragmentShader);
	glLinkProgram(myShaderProgram);

	glDeleteShader(myVertexShader);
	glDeleteShader(myFragmentShader);

	return myShaderProgram;
}

void initImGui(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	const char* glsl_version = "#version 130";
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void cleanAll()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

vector<int> getLineEquation(int x1, int y1, int x2, int y2, int x3, int y3) {
	// line equation: Ax+ By+ C= 0
	vector<int> res;
	int A = y2 - y1;
	int B = x1 - x2;
	int C = x2 * y1 - x1 * y2;

	//�������������ȥ�ǲ���С��0����Ϊ��������϶���insideҪ����0
	if (A*x3 + B * y3 + C < 0)
	{
		A *= -1;
		B *= -1;
		C *= -1;
	}
	res.push_back(A);
	res.push_back(B);
	res.push_back(C);
	return res;
}

vector<int> getRasterizeTripoints(int x1, int y1, int x2, int y2, int x3, int y3) {
	vector< vector<int> > linesParm(3);
	linesParm[0] = getLineEquation(x1, y1, x2, y2, x3, y3);
	linesParm[1] = getLineEquation(x1, y1, x3, y3, x2, y2);
	linesParm[2] = getLineEquation(x3, y3, x2, y2, x1, y1);
	
	//��������bounding box�ĵ�
	int maxX = max(x1, max(x2, x3));
	int minX = min(x1, min(x2, x3));
	int maxY = max(y1, max(y2, y3));
	int minY = min(y1, min(y2, y3));
	vector<int> points;
	for (int x = minX; x <= maxX; x++) {
		for (int y = minY; y <= maxY; y++) {
			bool inside = true;
			for (int i = 0; i< 3; i++) {
				if (linesParm[i][0] * x + linesParm[i][1] * y + linesParm[i][2]< 0) {
					inside = false;
					break;
				}
			}
			if (inside) {
				points.push_back(x);
				points.push_back(y);
			}
		}
	}
	return points;
}



void renderLoop(GLFWwindow* window)
{
	//�������ǰ����λ����Ϣ�������Ƕ����Ӧ��������ķ�����
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};




	//����һ��VAO
	unsigned int cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	//�½�һ�����㻺����󣬰����ͣ���������
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	//openGL��������Ҫ��ô��
	//λ����Ϣ
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//��������Ϣ
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//����Ҫע��������������Ҳ�����ɫ���Ĵ����ļ�
	

	bool depthTest = true;
	float diffuseStrength = 0.85;
	float ambientStrength = 0.1;
	float specularStrength = 0.5;
	float shininess = 32;
	bool GouraudOrPhong = false;
	bool moveLight = false;
	
	Camera cam(glm::vec3(0.0f, 0.0f, 3.0f));
	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	while (!glfwWindowShouldClose(window))
	{

		processInput(window, &cam);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ��Ȼ���ҲҪ���

															// imgui֡��ʼ��
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("hw6");
			ImGui::Checkbox("depth test", &depthTest);
			ImGui::SliderFloat("diffuseStrength", &diffuseStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("ambientStrength", &ambientStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("specularStrength", &specularStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("shininess", &shininess, 2.0f, 256.0f);
			ImGui::Checkbox("GouraudOrPhong", &GouraudOrPhong);
			ImGui::Checkbox("moveLight", &moveLight);
			ImGui::ColorEdit3("lightcolor", (float*)&lightColor);
			ImGui::ColorEdit3("objectcolor", (float*)&objectColor);
			
			ImGui::End();
		}

		//��Ȳ���
		if (depthTest)
			glEnable(GL_DEPTH_TEST);

		if (moveLight)
		{
			lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
			lightPos.z = sin(glfwGetTime() / 2) * 2.0f;
		}

		//�ж��ò�ͬ��shading����
		Shader lightingShader("color1.vs", "color1.fs");
		Shader lampShader("lamp1.vs", "lamp1.fs");
		if (GouraudOrPhong)
		{
			lightingShader = Shader("GouraudColor.vs", "GouraudColor.fs");
		}

		
		//���ù�����ɫ���������ɫ
		lightingShader.use();
		lightingShader.setVec3("objectColor", objectColor);
		lightingShader.setVec3("lightColor", lightColor);
		//����ambient����
		lightingShader.setFloat("ambientStrength", ambientStrength);
		//����diffuse����
		lightingShader.setFloat("diffuseStrength", diffuseStrength);
		//����specularStreng����
		lightingShader.setFloat("specularStrength", specularStrength);
		//���÷����shininess
		lightingShader.setFloat("Shininess", shininess);
		//���ù���λ�ã�phoneģ���еļ��㶼����Ƭ����ɫ������ɵ�
		lightingShader.setVec3("lightPos", lightPos);
		//���ù۲���λ�ã����ڼ��㾵�淴��,�۲���λ�þ��������λ��
		lightingShader.setVec3("viewPos", cam.getPosition());

		glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		//��ȡ��Ӧ��lookat����ʹ����������Ը��·�����Щ
		glm::mat4 view = cam.GetViewMatrix();
		//��������ת������
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);
		//���÷��������������������ٴ��붥����ɫ���ܼ��ټ�����
		lightingShader.setMat3("NormalMatrix", glm::mat3(transpose(inverse(model))));
		
		//��cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//���Ƶ�����
		lampShader.use();
		//project��view������һ���ģ�ͨ���ı�module����������λ�úʹ�С
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lampShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		//��Ⱦ
		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
}


