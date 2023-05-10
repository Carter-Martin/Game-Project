/**
* Group 13
* Platformer.cpp
* Use W, A, and D to move. Exit the game with ESC. Fullscren with F.
*
*/
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <stb_image.h>

#include "Draw.h"
#include "GLXtras.h"
#include "Sprite.h"

#pragma region player physics
const float VELOCITY_X = 0.2;
const float JUMP = 150;

const float GRAVITY = 0.0001; // gravitational acceleration
const float FRICTION_X = 0.001;
const float FRICTION_Y = 0.0001;
const float ACCELERATION_X = 0.01;
const float DECELERATION_X = 0.02;
const float MASS = 0.02;

const float X_DAMPER = 0.02;
const float Y_DAMPER = 0.00008;
const float MAX_VELOCITY = 0.2;


class PhysicsObject {
private:
	vec2 pos;
	vec2 velocity;
	vec2 friction;

	float acceleration;
	float deceleration;

	float mass;

	bool isAccelerating;

	float gravity;

public:
	PhysicsObject(float fx, float fy, float ax, float dx, float m, bool isAcc, bool g = GRAVITY) {
		pos = vec2(0.0, 0.0);
		velocity = vec2(0.0, 0.0);
		friction = vec2(fx, fy);

		acceleration = ax;
		deceleration = dx;

		mass = m;

		isAccelerating = isAcc;

		gravity = -g;
	}

	void update() {
		//gravity
		velocity.y += mass * gravity;

		//friction 
		float f_force = friction.x * mass * gravity;
		float f_direction = (velocity.x > 0) ? -1.0 : 1.0;
		if (velocity.x == 0) f_direction = 0;

		velocity.x += f_direction * f_force;

		//acceleration or deceleration
		if (isAccelerating) {
			//printf("accel\n");
			velocity.x += acceleration;
		}
		else {
			//printf("decel\n");
			float dec_direction = (velocity.x > 0) ? -1.0 : 1.0;
			if (velocity.x == 0) dec_direction = 0;
			velocity.x += dec_direction * deceleration;
			if ((dec_direction == -1 && velocity.x < 0) || (dec_direction == 1 && velocity.x > 0)) {
				velocity.x = 0.0;
			}
		}
		if (abs(velocity.x) > MAX_VELOCITY) velocity.x = MAX_VELOCITY;
		//update positions
		pos.x += velocity.x * X_DAMPER;
		if (!grounded) {
			pos.y += velocity.y * Y_DAMPER;
			can_jump = false;
		}
		else {
			if (can_jump) {
				pos.y += velocity.y * Y_DAMPER;
				can_jump = false;
			}
		}
	}

	bool grounded = false;
	bool can_jump = false;

	void move_x(float vel) { velocity.x = vel; }
	void accelerate() { isAccelerating = true; }
	void decelerate() { isAccelerating = false; }
	void jump(float vel) {
		velocity.y = vel;
		can_jump = true;
	}

	void setPos(vec2 p) { pos = p; }

	//getters 

	vec2 getPos() { return pos; }
	vec2 getFriction() { return friction; }
	vec2 getVel() { return velocity; }
	float getAcc() { return acceleration; }
	float getDec() { return deceleration; }
	float getMass() { return mass; }

};


#pragma endregion 

#pragma region variables
vec2 initPos = vec2(-0.93, -0.8754);
vec2 endPos = vec2(0.47, 0.8690);
vec2 scale = vec2(0.07, 0.1246);
vec2 p_scale = vec2(scale.x * 0.5, scale.y * 0.5);

bool fullscreen = 0;
int	winX = 50, winY = 50, winWidth = 1920, winHeight = 1080;
bool pressed[GLFW_KEY_LAST] = { 0 }; // track pressed keys
bool Quit = false;
bool finished = false;
//actor and hazard

Sprite actor(initPos, p_scale), door(endPos, scale), background, complete(vec2(0.0, 0.0), vec2(0.9, 1.0));

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

Sprite wallbot1(vec2(-0.37, -1.0), scale);
Sprite wallbot2(vec2(-0.23, -1.0), scale);
Sprite wallbot3(vec2(0.47, -1.0), scale);
Sprite wallbot4(vec2(0.61, -1.0), scale);
Sprite wallbot5(vec2(0.75, -1.0), scale);
Sprite wallbot6(vec2(0.89, -1.0), scale);
Sprite wallbot7(vec2(1.0, -1.0), scale);

Sprite hazard1(vec2(-0.65, 0.8690), scale * 2);
Sprite hazard2(vec2(-0.37, -0.8754), scale);
Sprite hazard3(vec2(-0.23, -0.8754), scale);
Sprite hazard4(vec2(0.12, -0.2524), scale);
Sprite hazard5(vec2(0.47, -0.8754), scale);
Sprite hazard6(vec2(0.61, -0.8754), scale);
Sprite hazard7(vec2(0.75, -0.8754), scale);
Sprite hazard8(vec2(0.89, -0.8754), scale);

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

	&wallbot1,
	&wallbot2,
	&wallbot3,
	&wallbot4,
	&wallbot5,
	&wallbot6,
	&wallbot7,
};
vector<Sprite*> hazard_sprites = {
	&hazard1,
	&hazard2,
	&hazard3,
	&hazard4,
	&hazard5,
	&hazard6,
	&hazard7,
	&hazard8,
};


PhysicsObject player(FRICTION_X, FRICTION_Y, ACCELERATION_X, DECELERATION_X, 5, false);

#pragma endregion

#pragma region glfw

typedef void(*KeyCallback)(int key, int press, bool shift, bool control);
KeyCallback kcb = NULL;
void RegKeyboard(KeyCallback, GLFWwindow*);
void Key(GLFWwindow* w, int key, int scancode, int action, int mods) {
	kcb(key, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL);
}

void RegKeyboard(KeyCallback cb, GLFWwindow* w) {
	kcb = cb;
	glfwSetKeyCallback(w, Key);
}

void Resize(int width, int height) { glViewport(0, 0, width, height); }

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

#pragma endregion

#pragma region sprites

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
	//if game is finished
	if (finished)
		complete.Display();
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

void basic_movement(vec2& actorPos) {

	//static movement for testing purposes
	if (pressed[GLFW_KEY_A] && !pressed[GLFW_KEY_D]) {
		actorPos.x -= 0.01;
	}
	if (pressed[GLFW_KEY_D] && !pressed[GLFW_KEY_A]) {
		actorPos.x += 0.01;
	}
	if (pressed[GLFW_KEY_W] && !pressed[GLFW_KEY_S]) {
		actorPos.y += 0.01;
	}
	if (pressed[GLFW_KEY_S] && !pressed[GLFW_KEY_W]) {
		actorPos.y -= 0.01;
	}

}

void movement() {
	if ((pressed[GLFW_KEY_A] && !pressed[GLFW_KEY_D]) || (pressed[GLFW_KEY_D] && !pressed[GLFW_KEY_A])) {
		if (pressed[GLFW_KEY_A] && !pressed[GLFW_KEY_D]) {
			player.move_x(-VELOCITY_X);
			player.accelerate();
		}
		if (pressed[GLFW_KEY_D] && !pressed[GLFW_KEY_A]) {
			player.move_x(VELOCITY_X);
			player.accelerate();
		}
	}
	else player.decelerate();

	if (pressed[GLFW_KEY_W] && !pressed[GLFW_KEY_S]) {
		player.jump(JUMP);
	}

}

#pragma endregion

#pragma region gameplay loop
void Update() {
	if (pressed[GLFW_KEY_ESCAPE]) Quit = true;

	vec2 actorPos = actor.GetPosition();
	bool wall_hit = false;
	player.setPos(actorPos);
	movement();
	player.update();
	actorPos = player.getPos();

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
	if (wall_hit) {
		player.grounded = true;
	}
	else {
		player.grounded = false;
	}
	if (!CheckBound(temp)) {
		actor.SetPosition(actorPos);
	}
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
		finished = true;
	}
}

#pragma endregion

int main(int ac, char** av) {

	GLFWwindow* w = InitGLFW(winX, winY, winWidth, winHeight, "Sprite Demo");

	GLFWimage icon[1];
	icon[0].pixels = stbi_load("coin.png", &icon[0].width, &icon[0].height, 0, 4);
	glfwSetWindowIcon(w, 1, icon);
	stbi_image_free(icon[0].pixels);

#pragma region init_sprites
	background.Initialize("Background.png");
	complete.Initialize("levelComplete.png");
	actor.Initialize("Body.png");
	door.Initialize("Door.png");

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

	wallbot1.Initialize("Grass.png");
	wallbot2.Initialize("Grass.png");
	wallbot3.Initialize("Grass.png");
	wallbot4.Initialize("Grass.png");
	wallbot5.Initialize("Grass.png");
	wallbot6.Initialize("Grass.png");
	wallbot7.Initialize("Grass.png");

	hazard1.Initialize("Mace.png");
	hazard2.Initialize("Spike_Up.png");
	hazard3.Initialize("Spike_Up.png");
	hazard4.Initialize("Mace-ball.png");
	hazard5.Initialize("Spike_Up.png");
	hazard6.Initialize("Spike_Up.png");
	hazard7.Initialize("Spike_Up.png");
	hazard8.Initialize("Spike_Up.png");
#pragma endregion

	RegKeyboard(Keystroke, w);
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