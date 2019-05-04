#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
class Camera {
public:
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
	{
		Position = position;
		WorldUp = up;
		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		updates();
	}
	void moveForward(float const distance)
	{
		
		Position += Front * distance;
	}
	void moveBack(float const distance)
	{
		Position -= Front *  distance;
	}
	void moveRight(float const distance)
	{
		Position += Right * distance;
	}
	void moveLeft(float const distance)
	{
		Position -= Right * distance;
	}
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, (Position + Front), Up);
	}
	void rotate(float xpos, float ypos)
	{	
		float xoffset = xpos - prexPos;
		float yoffset = preyPos - ypos;
		xoffset *= SENSITIVITY;
		yoffset *= SENSITIVITY;

		prexPos = xpos;
		preyPos = ypos;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		

		updates();
	}

	glm::vec3 getPosition()
	{
		return Position;
	}

private:
	float pitch = 0.f, yaw = -90.f;
	bool first = true;
	float prexPos = 500, preyPos = 500;
	float SENSITIVITY = 0.1f;
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	float speed = 0.05;

	void updates()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Front = glm::normalize(front);
		
		Right = glm::normalize(glm::cross(Front, WorldUp));  
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
