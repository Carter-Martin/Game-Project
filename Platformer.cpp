/**
* Group 13
* Platformer.cpp
* Use W, A, S, and D to move. Exit the game with ESC
* 
* NOTE : y position updates are commented out of Update() to make it easier 
*	to understand the x position updates. Uncomment that line if you want to
*	experience gravity.
*/

#include <glad.h>
#include <GLFW/glfw3.h>
#include "GLXtras.h"
#include "Sprite.h"
#include <cmath>

Sprite background, actor;

bool pressed[GLFW_KEY_LAST] = {0}; // track pressed keys
bool Quit = false;
const float MAX_VELOCITY = 0.1;
const float GRAVITY = 0.001;
const float X_ACCELERATION = 0.01;
const float Y_ACCELERATION = 0.01;
vec2 velocity = vec2(0.0, 0.0);


void MouseButton(float x, float y, bool left, bool down) { if (left && down) actor.Down(x, y); }

void MouseMove(float x, float y, bool leftDown, bool rightDown) { if (leftDown) actor.Drag(x, y); }


void Keystroke(int key, int press, bool shift, bool control) {
	if (press == GLFW_PRESS)
	{
		//std::cout << "Key pressed: " << key << std::endl;
		pressed[key] = GLFW_PRESS;
	}
	else if (press == GLFW_RELEASE)
	{
		//std::cout << "Key released: " << key << std::endl;
		pressed[key] = GLFW_RELEASE;
	}
}

void Update() {
	if (pressed[GLFW_KEY_ESCAPE]) Quit = true;
	
	vec2 pos = actor.GetPosition();

	if (pressed[GLFW_KEY_W]) {
		velocity.y = 0;
		velocity.y += Y_ACCELERATION;
	}
	if (velocity.y - GRAVITY > -MAX_VELOCITY)velocity.y += -GRAVITY;


	if (pressed[GLFW_KEY_A] && !pressed[GLFW_KEY_D]) {
		if (velocity.x - X_ACCELERATION > -MAX_VELOCITY) velocity.x += -X_ACCELERATION;
	}
	if (pressed[GLFW_KEY_D] && !pressed[GLFW_KEY_A]) {
		if (velocity.x + X_ACCELERATION < MAX_VELOCITY) velocity.x += X_ACCELERATION;
	}
	if (velocity.x != 0) {
		if ((!pressed[GLFW_KEY_D] && !pressed[GLFW_KEY_A]) || (pressed[GLFW_KEY_D] && pressed[GLFW_KEY_A])) {
			if (velocity.x > 0) velocity.x += -X_ACCELERATION;
			else if (velocity.x < 0) velocity.x += X_ACCELERATION;
		}
	}
	//pos.y += velocity.y;
	pos.x += velocity.x;
	

	actor.SetPosition(pos);
}


// Application

void Resize(int width, int height) { glViewport(0, 0, width, height); }

int main(int ac, char **av) {
	GLFWwindow *w = InitGLFW(100, 100, 600, 600, "Sprite Demo");
	// read background, foreground, and mat textures
	background.Initialize("C:/Users/shiny/Code/Graphics/Apps/blur-background.png");
	actor.Initialize("C:/Users/shiny/Code/Graphics/Apps/128x128 player.PNG");
	actor.SetPosition(vec2(0.2,0.2));
	actor.SetScale(vec2(0.5,0.5));
	// callbacks
	RegisterMouseButton(MouseButton);
	RegisterMouseMove(MouseMove);
	RegisterResize(Resize);

	RegisterKeyboard(Keystroke);
	printf("Move with W, A, S, and D. Exit with ESC\n");
	// event loop
	while (!glfwWindowShouldClose(w)) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		background.Display();
		actor.Display();
		glFlush();
		glfwSwapBuffers(w);
		glfwPollEvents();

		Update();
		if (Quit) glfwSetWindowShouldClose(w, GLFW_TRUE);
	}
}
