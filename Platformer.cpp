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
#include <stdio.h>
#include "Draw.h"
#include "GLXtras.h"
#include "Sprite.h"
#include <cmath>
#include<vector>

//Initial position
vec2 initPos = vec2(-0.93, -0.8754);
vec2 endPos = vec2(0.47, 0.8690);
vec2 scale = vec2(0.07, 0.1246);

Sprite actor(initPos, scale), door(endPos, scale), background;

Sprite wall1(vec2(-0.93, -0.0900), scale);
Sprite wall2(vec2(-0.65, -0.6262), scale);
Sprite wall3(vec2(-0.51, 0.1214), scale);
Sprite wall4(vec2(-0.51, -0.1278), scale);
Sprite wall5(vec2(-0.51, -0.3770), scale);
Sprite wall6(vec2(-0.51, -0.6262), scale);
Sprite wall7(vec2(-0.51, -0.8754), scale);
Sprite wall8(vec2(-0.09, -0.8754), scale);
Sprite wall9(vec2(-0.09, -0.6262), scale);
Sprite wall10(vec2(-0.09, -0.3770), scale);
Sprite wall11(vec2(0.05, -0.8754), scale);
Sprite wall12(vec2(0.05, -0.6262), scale);
Sprite wall13(vec2(0.05, -0.3770), scale);
Sprite wall14(vec2(0.12, -0.8754), scale);
Sprite wall15(vec2(0.12, -0.6262), scale);
Sprite wall16(vec2(0.12, -0.3770), scale);
Sprite wall17(vec2(0.19, -0.8754), scale);
Sprite wall18(vec2(0.19, -0.6262), scale);
Sprite wall19(vec2(0.19, -0.3770), scale);
Sprite wall20(vec2(0.33, -0.8754), scale);
Sprite wall21(vec2(0.33, -0.6262), scale);
Sprite wall22(vec2(0.33, -0.3770), scale);
Sprite wall23(vec2(0.47, 0.6198), scale);
Sprite wall24(vec2(0.61, -0.1278), scale);
Sprite wall25(vec2(0.89, 0.3706), scale);

Sprite hazard1(vec2(-0.65, 0.8690), scale);
Sprite hazard2(vec2(-0.37, -0.8754), scale);
Sprite hazard3(vec2(-0.23, -0.8754), scale);
Sprite hazard4(vec2(0.47, -0.8754), scale);
Sprite hazard5(vec2(0.61, -0.8754), scale);
Sprite hazard6(vec2(0.75, -0.8754), scale);
Sprite hazard7(vec2(0.89, -0.8754), scale);

vector<Sprite*> wall_sprites = {
	&wall1,
	&wall2,
	&wall3,
	&wall4,
	&wall5,
	&wall6,
	&wall7,
	&wall8,
	&wall9,
	&wall10,
	&wall11,
	&wall12,
	&wall13,
	&wall14,
	&wall15,
	&wall16,
	&wall17,
	&wall18,
	&wall19,
	&wall20,
	&wall21,
	&wall22,
	&wall23,
	&wall24,
	&wall25,
};
vector<Sprite*> hazard_sprites = {
	&hazard1,
	&hazard2,
	&hazard3,
	&hazard4,
	&hazard5,
	&hazard6,
	&hazard7,
};

//display
bool fullscreen = 0;
int	winX = 100, winY = 100, winWidth = 1920, winHeight = 1080;

bool pressed[GLFW_KEY_LAST] = { 0 }; // track pressed keys
bool Quit = false;
const float MAX_VELOCITY = 0.1;
const float GRAVITY = 0.001;
const float X_ACCELERATION = 0.001;
const float Y_ACCELERATION = 0.01;
vec2 velocity = vec2(0.0, 0.0);

void MouseButton(float x, float y, bool left, bool down) {
	if (left && down)
		actor.Down(x, y);
}

void MouseMove(float x, float y, bool leftDown, bool rightDown) {
	if (leftDown)
		actor.Drag(x, y);
}


void Keystroke(int key, bool press, bool shift, bool control) {
	if (press == GLFW_PRESS)
	{
		std::cout << "Key pressed: " << key << std::endl;
		pressed[key] = GLFW_PRESS;
	}
	else if (press == GLFW_RELEASE)
	{
		std::cout << "Key released: " << key << std::endl;
		pressed[key] = GLFW_RELEASE;
	}
}

void Display() {
	background.Display();
	for (Sprite* w : wall_sprites) {
		w->Display();
	}
	for (Sprite* h : hazard_sprites) {
		h->Display();
	}
	door.Display();
	actor.Display();
}

void toggleFullscreen(GLFWwindow* window) {
	if (pressed[GLFW_KEY_F]) {
		fullscreen = !fullscreen;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, fullscreen ? monitor : NULL, fullscreen ? 0 : winX, fullscreen ? 0 : winY, fullscreen ? mode->width : winWidth, fullscreen ? mode->height : winHeight, GLFW_DONT_CARE);
	}
}

//Checks if the sprite postion is within bounds of the game
bool CheckBound(Sprite s)
{
	vec2 sPos = s.GetPosition();
	// outside x-axis bound?
	bool collisionX = sPos.x <= -0.94 || sPos.x >= 0.94;
	// outside y-axis bound?
	bool collisionY = sPos.y <= -0.8932 || sPos.y >= 0.8932;
	return collisionX || collisionY;
}

void Update() {
	if (pressed[GLFW_KEY_ESCAPE]) Quit = true;

	vec2 actorPos = actor.GetPosition();
	bool wall_hit = false;

	/*if (pressed[GLFW_KEY_W]) {
		velocity.y = 0;
		velocity.y += Y_ACCELERATION;
	}
	if (velocity.y - GRAVITY > -MAX_VELOCITY)velocity.y += -GRAVITY;*/

	//static movement for testing purposes
	if (pressed[GLFW_KEY_A] && !pressed[GLFW_KEY_D]) {
		actorPos.x -= 0.01;

		//if (velocity.x - X_ACCELERATION > -MAX_VELOCITY) velocity.x += -X_ACCELERATION;
	}
	if (pressed[GLFW_KEY_D] && !pressed[GLFW_KEY_A]) {
		actorPos.x += 0.01;

		//if (velocity.x + X_ACCELERATION < MAX_VELOCITY) velocity.x += X_ACCELERATION;
	}
	if (pressed[GLFW_KEY_W] && !pressed[GLFW_KEY_S]) {
		actorPos.y += 0.01;
		//if (velocity.x - X_ACCELERATION > -MAX_VELOCITY) velocity.x += -X_ACCELERATION;
	}
	if (pressed[GLFW_KEY_S] && !pressed[GLFW_KEY_W]) {
		actorPos.y -= 0.01;
		//if (velocity.x + X_ACCELERATION < MAX_VELOCITY) velocity.x += X_ACCELERATION;
	}
	/*
	if (velocity.x != 0) {
		if ((!pressed[GLFW_KEY_D] && !pressed[GLFW_KEY_A]) || (pressed[GLFW_KEY_D] && pressed[GLFW_KEY_A])) {
			if (velocity.x > 0) velocity.x += -X_ACCELERATION;
			else if (velocity.x < 0) velocity.x += X_ACCELERATION;
		}
	}
	*/
	//pos.y += velocity.y;
	//actorPos.x += velocity.x;

	//store new position in temp sprite to test if its a valid move
	Sprite temp = actor;
	temp.SetPosition(actorPos);

	//Check if actor touches wall
	for (Sprite* i : wall_sprites) {
		//if actor touches any wall sprites
		if (temp.Intersect(*i))
		{
			std::cout << "intersect wall_sprites" << std::endl;
			wall_hit = true;
		}
	}

	//set new position if actor move and hit a wall or out of bound
	if (!wall_hit && !CheckBound(temp))
		actor.SetPosition(actorPos);

	//Check if actor touches a hazard
	for (Sprite* i : hazard_sprites) {
		//if actor touches any hazard sprites
		if (actor.Intersect(*i))
		{
			std::cout << "intersect hazard" << std::endl;
			//Return to starting point
			actor.SetPosition(initPos);
		}
	}
	//If reach the door/chest
	if (actor.Intersect(door)) {
		std::cout << "Finish, now what" << std::endl;
	}
}

// Application
void Resize(int width, int height) { glViewport(0, 0, width, height); }

int main(int ac, char** av) {
	GLFWwindow* w = InitGLFW(winX, winY, winWidth, winHeight, "Sprite Demo");

	background.Initialize("Background.png");
	actor.Initialize("Body.png");
	door.Initialize("chest.png");

	wall1.Initialize("Grass.png");
	wall2.Initialize("Grass.png");
	wall3.Initialize("Grass.png");
	wall4.Initialize("Dirt.png");
	wall5.Initialize("Dirt.png");
	wall6.Initialize("Dirt.png");
	wall7.Initialize("Dirt.png");
	wall8.Initialize("Dirt.png");
	wall9.Initialize("Dirt.png");
	wall10.Initialize("Grass.png");
	wall11.Initialize("Dirt.png");
	wall12.Initialize("Dirt.png");
	wall13.Initialize("Grass.png");
	wall14.Initialize("Dirt.png");
	wall15.Initialize("Dirt.png");
	wall16.Initialize("Grass.png");
	wall17.Initialize("Dirt.png");
	wall18.Initialize("Dirt.png");
	wall19.Initialize("Grass.png");
	wall20.Initialize("Dirt.png");
	wall21.Initialize("Dirt.png");
	wall22.Initialize("Grass.png");
	wall23.Initialize("Grass.png");
	wall24.Initialize("Grass.png");
	wall25.Initialize("Grass.png");

	hazard1.Initialize("Mace.png");
	hazard2.Initialize("Spike_Up.png");
	hazard3.Initialize("Spike_Up.png");
	hazard4.Initialize("Spike_Up.png");
	hazard5.Initialize("Spike_Up.png");
	hazard6.Initialize("Spike_Up.png");
	hazard7.Initialize("Spike_Up.png");

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
		Display();
		glFlush();
		glfwSwapBuffers(w);
		glfwPollEvents();
		toggleFullscreen(w);
		Update();
		if (Quit) glfwSetWindowShouldClose(w, GLFW_TRUE);
	}
}
