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
void renderScene(const Shader &shader);

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

	//先判断直线的方向，然后用算法的时候就由简单的加1，改成加方向的单位移动
	int directX = (dx > 0) ? 1 : -1;
	int directY = (dy > 0) ? 1 : -1;
	if (dx < 0) 
		dx = -dx;
	if (dy < 0) 
		dy = -dy;
	if (dx == 0)
		dx = 0.01;
	double m = dy / dx; //斜率

	if (m < 1)	//小于1就是上课讲的那种
	{
		int ddy = 2 * dy; //两倍dy
		int ddiffer = ddy - 2 * dx; //两倍dy-两倍dx
		int p = ddy - dx; //p0

		while (x0 != x1) //直接用x0作为循环变量xi
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
	else //斜率大于1的情况就是坐标系反转
	{
		int ddx = 2 * dx; //两倍dx
		int ddiffer = ddx - 2 * dy; //两倍dx-两倍dy
		int p = ddx - dy; //p0

		while (y0 != y1)		//这里要判断y,因为斜率大于1是向y方向的
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
	float *vertices = new float[point_num * 6]; //除了x,y坐标还有深度，然后还有颜色
	for (int i = 0; i< point_num; i++) {
		// 位置
		vertices[i * 6 + 0] = (float)v[i * 2 + 0] / (float)WIDTH;		//归一化坐标
		vertices[i * 6 + 1] = (float)v[i * 2 + 1] / (float)HEIGHT;
		vertices[i * 6 + 2] = 0.0f;
		// 颜色
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

	//创建窗口
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "firWindow", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	//判断是否加载glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}

	//设定那个什么视口
	glViewport(0, 0, WIDTH, HEIGHT);

	//注册缓冲更改时的回调函数
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

	//根据源码编译两个着色器
	unsigned int myVertexShader;
	myVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(myVertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(myVertexShader);

	unsigned int myFragmentShader;
	myFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(myFragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(myFragmentShader);

	//创建程序对象，将着色器附加上去，然后将程序链接,激活，就可以把着色器对象删掉了
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

	//看第三个点带进去是不是小于0，因为第三个点肯定是inside要大于0
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
	
	//测试整个bounding box的点
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


unsigned int planeVAO;
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

void renderLoop(GLFWwindow* window)
{
	// 画出一个平面
	float planeVertices[] = {
		// 位置信息            // 法向量信息
		25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 
		-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f,
		-25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 

		25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f,
		25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f,
		-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f
	};
	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	// 告诉opengl怎么看顶点数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindVertexArray(0);

	//产生深度贴图缓冲对象
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	//创建2d纹理
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//把纹理作为帧缓冲的附件
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//告诉OpenGL我们不适用任何颜色数据进行渲染
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 定义shader
	Shader shader("shadow_mapping.vs", "shadow_mapping.fs");
	Shader simpleDepthShader("shadow_mapping_depth.vs", "shadow_mapping_depth.fs");
	shader.use();
	//设置shadowmap的编号
	shader.setInt("shadowMap", 0);
	
	

	float diffuseStrength = 0.85;
	float ambientStrength = 0.1;
	float specularStrength = 0.5;
	float shininess = 32;
	bool perspective = false;
	bool moveLight = false;
	
	Camera cam(glm::vec3(0.0f, 0.0f, 3.0f));
	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	while (!glfwWindowShouldClose(window))
	{
		
		processInput(window, &cam);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓存也要清除
		glEnable(GL_DEPTH_TEST);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("hw7");
			ImGui::SliderFloat("diffuseStrength", &diffuseStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("ambientStrength", &ambientStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("specularStrength", &specularStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("shininess", &shininess, 2.0f, 256.0f);
			ImGui::Checkbox("perspective", &perspective);
			ImGui::Checkbox("moveLight", &moveLight);
			ImGui::ColorEdit3("lightcolor", (float*)&lightColor);
			ImGui::ColorEdit3("objectcolor", (float*)&objectColor);

			ImGui::End();
		}
		//根据GUI中选择的进行调整参数
		shader.setVec3("objectColor", objectColor);
		shader.setVec3("lightColor", lightColor);
		//设置ambient因子
		shader.setFloat("ambientStrength", ambientStrength);
		//设置diffuse因子
		shader.setFloat("diffuseStrength", diffuseStrength);
		//设置specularStreng因子
		shader.setFloat("specularStrength", specularStrength);
		//设置反光度shininess
		shader.setFloat("Shininess", shininess);
		if (moveLight)
		{
			lightPos.x = sin(glfwGetTime()) * 3.0f;
			lightPos.z = cos(glfwGetTime()) * 2.0f;
			lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;
		}
		

		//从光源视角进行渲染，得到深度贴图
		//先获取对应的view和projection矩阵
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		GLfloat near_plane = 1.0f, far_plane = 7.5f;
		if(perspective)
			lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
		else
			lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		//从光源的视角进行渲染
		simpleDepthShader.use();
		simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		//改变此时渲染的viewport，作为深度贴图的分辨率？
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(simpleDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//正常的渲染场景
		//先把视口设置回来，清楚缓存
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		//view矩阵和投影矩阵
		glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		//获取对应的lookat矩阵，使得照相机可以更新方向那些
		glm::mat4 view = cam.GetViewMatrix();
		//给shader传递坐标转换矩阵
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		// 给shader传递变量
		shader.setVec3("viewPos", cam.getPosition());
		shader.setVec3("lightPos", lightPos);
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderScene(shader);

		//渲染
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

void renderCube()
{
	//先初始化VAO
	if (cubeVAO == 0)
	{
		float vertices[] = {
			//六个面，每个面两个三角形，六个点
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, // Bottom-left
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, // bottom-right         
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  // top-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  // bottom-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,// top-left
			
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, // bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,  // top-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, // top-right
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,  // top-left
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,  // bottom-left
			
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, // top-right
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, // top-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,  // bottom-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,  // bottom-right
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, // top-right
			
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // top-right         
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,  // top-left
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // bottom-left     
			
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, // top-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,// bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, // top-right
			
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,// top-left
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // top-right     
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // bottom-right
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,// top-left
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f // bottom-left            
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		//通过buffer传数据
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// 告诉opengl怎么看数据
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// 渲染
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void renderScene(const Shader &shader)
{
	// 先渲染地板
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(1.0f, 1.5f, 1.0));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(3.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.3f, 2.0f, 3.0));
	model = glm::rotate(model, glm::radians(80.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.35));
	shader.setMat4("model", model);
	renderCube();
}

