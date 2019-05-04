#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include<algorithm>
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
void processInput(GLFWwindow *window);
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

const float WIDTH = 800;
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
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



void renderLoop(GLFWwindow* window)
{
	//顶点矩阵
	float vertices[] = {
		-2.f, -2.f, -2.f,  0.0f, 0.0f,
		2.f, -2.f, -2.f,  1.0f, 0.0f,
		2.f,  2.f, -2.f,  1.0f, 1.0f,
		2.f,  2.f, -2.f,  1.0f, 1.0f,
		-2.f,  2.f, -2.f,  0.0f, 1.0f,
		-2.f, -2.f, -2.f,  0.0f, 0.0f,

		-2.f, -2.f,  2.f,  0.0f, 0.0f,
		2.f, -2.f,  2.f,  1.0f, 0.0f,
		2.f,  2.f,  2.f,  1.0f, 1.0f,
		2.f,  2.f,  2.f,  1.0f, 1.0f,
		-2.f,  2.f,  2.f,  0.0f, 1.0f,
		-2.f, -2.f,  2.f,  0.0f, 0.0f,

		-2.f,  2.f,  2.f,  1.0f, 0.0f,
		-2.f,  2.f, -2.f,  1.0f, 1.0f,
		-2.f, -2.f, -2.f,  0.0f, 1.0f,
		-2.f, -2.f, -2.f,  0.0f, 1.0f,
		-2.f, -2.f,  2.f,  0.0f, 0.0f,
		-2.f,  2.f,  2.f,  1.0f, 0.0f,

		2.f,  2.f,  2.f,  1.0f, 0.0f,
		2.f,  2.f, -2.f,  1.0f, 1.0f,
		2.f, -2.f, -2.f,  0.0f, 1.0f,
		2.f, -2.f, -2.f,  0.0f, 1.0f,
		2.f, -2.f,  2.f,  0.0f, 0.0f,
		2.f,  2.f,  2.f,  1.0f, 0.0f,

		-2.f, -2.f, -2.f,  0.0f, 1.0f,
		2.f, -2.f, -2.f,  1.0f, 1.0f,
		2.f, -2.f,  2.f,  1.0f, 0.0f,
		2.f, -2.f,  2.f,  1.0f, 0.0f,
		-2.f, -2.f,  2.f,  0.0f, 0.0f,
		-2.f, -2.f, -2.f,  0.0f, 1.0f,

		-2.f,  2.f, -2.f,  0.0f, 1.0f,
		2.f,  2.f, -2.f,  1.0f, 1.0f,
		2.f,  2.f,  2.f,  1.0f, 0.0f,
		2.f,  2.f,  2.f,  1.0f, 0.0f,
		-2.f,  2.f,  2.f,  0.0f, 0.0f,
		-2.f,  2.f, -2.f,  0.0f, 1.0f
	};




	//创建一个VAO
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//新建一个顶点缓冲对象，绑定类型，传输数据
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	//openGL顶点数据要怎么看
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture坐标属性
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	Shader myshader = Shader();


	unsigned int texture1, texture2;
	// 纹理1
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// 设定textture参数环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// 过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载图片
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // 反转图片
	cout << "A";
	unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
	cout << "A";
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	// 纹理2
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// 环绕方式参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// 过滤方式参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载图片，生成纹理
	data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
	cout << "b";
	if (data)
	{
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	cout << "D";
	stbi_image_free(data);

	// 告诉采样器对应的纹理
	// -------------------------------------------------------------------------------------------
	myshader.use();
	myshader.setInt("texture1", 0);
	myshader.setInt("texture2", 1);

	bool depthTest = false;
	bool move = false;
	bool rot = false;
	bool scale = false;
	float x = 0;
	float s = 1;
	bool sindex = false;
	bool bonus = false;
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓存也要清除

		// imgui帧初始化
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("hw4");
			ImGui::Checkbox("depth test", &depthTest);
			ImGui::Checkbox("move", &move);
			ImGui::Checkbox("rotate", &rot);
			ImGui::Checkbox("scale", &scale);
			ImGui::Checkbox("BONUS", &bonus);
			ImGui::End();
		}
		//深度测试
		if(depthTest)
			glEnable(GL_DEPTH_TEST);

		//绑定textture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		// 使用shader
		myshader.use();


		// 坐标变换的矩阵
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		//沿x轴旋转55度,方便看到多几个面
		model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		if (bonus)
		{
			
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.f, 1.f));
			model = glm::translate(model, glm::vec3((float)-2.5, 2.5f, 0.0f));
			model = glm::scale(model, glm::vec3((float)0.3, (float)0.3, (float)0.3));

		}
		if (move)
		{
			//给一个平移的方向
			x = x + 0.1;
			if (x >= 8)
				x = -8;
			model = glm::translate(model, glm::vec3((float)x, 0.0f, 0.0f));
		}
		if (rot)
		{
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 0.0f, 0.5f));
		}
		if (scale)
		{
			if (s <= 0.1)
				sindex = true;
			else if(s > 0.9)
				sindex = false;
			if (sindex)
				s += 0.01;
			else
				s -= 0.01;
			model = glm::scale(model, glm::vec3((float)s, (float)s, (float)s));
		}

		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -16.0f));
		projection = glm::perspective(glm::radians(45.0f), (float)600 / (float)800, 0.1f, 100.0f);
		// retrieve the matrix uniform locations
		unsigned int modelLoc = glGetUniformLocation(myshader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(myshader.ID, "view");
		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		
		myshader.setMat4("projection", projection);


		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		if (bonus)
		{
			glm::mat4 model2 = glm::mat4(1.0f);

			model2 = glm::rotate(model2, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model2 = glm::scale(model2, glm::vec3((float)0.3, (float)0.3, (float)0.3));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

			myshader.setMat4("projection", projection);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

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


