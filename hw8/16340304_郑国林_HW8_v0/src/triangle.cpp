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
float *convert2vertice(vector<float> v);
vector<int> getCirclePoints(int x0, int y0, int r);
vector<int> getEightSymetricPoint(int originx, int originy, int offsetx, int offsety);
vector<int> getRasterizeTripoints(int x1, int y1, int x2, int y2, int x3, int y3);
vector<int> getLineEquation(int x1, int y1, int x2, int y2, int x3, int y3);

typedef struct data {
	GLfloat x;
	GLfloat y;
}Point;

void drawDynamicProcedure(unsigned int shader);
void drawCurve(unsigned int shader);
void drawLine(Point fir, Point sec, unsigned int shader);
vector<Point> Bezier(vector<Point>& points);
int factorial(int num);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
double Bernstein(int i, int n, double t);

GLFWwindow* openGLallInit();
unsigned int initShaders();
void initImGui(GLFWwindow* window);
void cleanAll();

void renderLoop(GLFWwindow* window);

const float WIDTH = 1000;
const float HEIGHT = 1000;



vector<Point> controlPoints;
double dynamicT = 0;


int main()
{
	GLFWwindow* window = openGLallInit();
	if (window == NULL)
		return -1;



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


float* convert2vertice(vector<float> v) {
	int point_num = v.size() / 2;
	float *vertices = new float[point_num * 3]; //除了x,y坐标还有深度，然后还有颜色
	for (int i = 0; i< point_num; i++) {
		// 位置
		vertices[i * 3 + 0] = (float)v[i * 2 + 0];
		vertices[i * 3 + 1] = (float)v[i * 2 + 1];
		vertices[i * 3 + 2] = 0.0f;
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
	//glfwSetCursorPosCallback(window, mouse_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	return window;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		controlPoints.pop_back();

		dynamicT = 0;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double tempX = 0, tempY = 0;
		glfwGetCursorPos(window, &tempX, &tempY);
		
		tempX = (-1) + (tempX / WIDTH) * 2;
		tempY = (-1)*((-1) + (tempY / HEIGHT) * 2);
		Point p;
		p.x = tempX;
		p.y = tempY;
		controlPoints.push_back(p);

		dynamicT = 0;
	}
}

unsigned int initShaders()
{
	const char *vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char *fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"uniform vec4 drawColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = drawColor;\n"
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

vector<Point> Bezier(vector<Point>& points)
{
	vector<Point> resultPoints;

	int n = points.size() - 1;
	for (double t = 0; t <= 1; t += 0.00005)
	{
		Point targetPoint;
		targetPoint.x = 0;
		targetPoint.y = 0;

		for (int i = 0; i <= n; i++)
		{
			double B = Bernstein(i, n, t);
			Point curPoint = points[i];

			targetPoint.x += curPoint.x * B;
			targetPoint.y += curPoint.y * B;
		}

		resultPoints.push_back(targetPoint);
	}

	return resultPoints;

}

double Bernstein(int i, int n, double t)
{
	double B = factorial(n) / (factorial(i) * factorial(n - i));
	B = B * pow(t, i) * pow((1 - t), (n - i));
	return B;
}

int factorial(int num)
{
	int ans = 1;
	for (int i = 1; i <= num; i++)
		ans *= i;
	return ans;
}

void renderLoop(GLFWwindow* window)
{
	unsigned int shaderProgram = initShaders();

	
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	
	bool out = true;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (controlPoints.size() >= 2)
		{	
			glUseProgram(shaderProgram);

			for (int i = 0; i < controlPoints.size() - 1; i++)
			{
				drawLine(controlPoints[i], controlPoints[i + 1], shaderProgram);
			}
			
			drawCurve(shaderProgram);
			drawDynamicProcedure(shaderProgram);
		}
		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void drawDynamicProcedure(unsigned int shader)
{
	double t = dynamicT;
	vector<Point> prePoints(controlPoints);
	while (prePoints.size() != 0)
	{
		vector<Point> temp(prePoints);
		prePoints.clear();
		
		for (int j = 0; j < temp.size() - 1; j++)
		{
			Point p;
			Point fir = temp[j];
			Point sec = temp[j + 1];
			p.x = t * sec.x + (1 - t)*fir.x;
			p.y = t * sec.y + (1 - t)*fir.y;
			prePoints.push_back(p);
		}
		//draw line
		for (int j = 0; prePoints.size() >= 1 &&  j < prePoints.size() - 1; j++)
		{
			drawLine(prePoints[j], prePoints[j + 1], shader);
		}
	}
	if (dynamicT + 0.005 > 1)
		dynamicT = 1;
	else
		dynamicT += 0.005;
}

void drawLine(Point fir, Point sec, unsigned int shader)
{
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glm::vec4 color(1, 1, 1, 1);
	glUniform4fv(glGetUniformLocation(shader, "drawColor"), 1, &color[0]);

	float vertices[] = { fir.x, fir.y, 0.0, sec.x, sec.y, 0.0 };
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_LINE_STRIP, 0, 2);
	glBindVertexArray(0); // no need to unbind it every time 
}

void drawCurve(unsigned int shader)
{
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	vector<float> points;
	vector<Point> bezierPoints = Bezier(controlPoints);
	glm::vec4 color(1.0f, 0.5f, 0.2f, 1);
	
	glUniform4fv(glGetUniformLocation(shader, "drawColor"), 1, &color[0]);
	for (int i = 0; i < bezierPoints.size(); i++)
	{
		Point P = bezierPoints[i];
		points.push_back(P.x);
		points.push_back(P.y);
	}
	int point_num = points.size() / 2;
	float* vertices = convert2vertice(points);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, point_num * 3 * sizeof(float), vertices, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//glDrawArrays(GL_LINE_STRIP, 0, 2);
	glDrawArrays(GL_POINTS, 0, point_num);
	glBindVertexArray(0); // no need to unbind it every time 
}

