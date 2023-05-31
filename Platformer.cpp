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
#include <stb_image.h>

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "Draw.h"
#include "GLXtras.h"
#include "Sprite.h"

#pragma region player physics
const float VELOCITY_X = 0.2;
const float JUMP = 150;

const float GRAVITY = 1.5; // gravitational acceleration
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
	PhysicsObject(float fx, float fy, float ax, float dx, float m, bool isAcc, float g = GRAVITY) {
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
vec2 initPos = vec2(-0.93f, -0.8754f);
//vec2 endPos = vec2(0.47f, 0.8690f);
//vec2 scale = vec2(0.07f, 0.1246f);
//vec2 p_scale = vec2(scale.x * 0.5f, scale.y * 0.5f);

bool fullscreen = 0;
int	winX = 50, winY = 50, winWidth = 1920, winHeight = 1080;
bool pressed[GLFW_KEY_LAST] = { 0 }; // track pressed keys

//Game state
bool menu = true;
bool options = false;
bool game = false;
bool quit = false;
bool finished = false;

//actor and hazard
Sprite title(vec2(0.0f, 0.5f), vec2(0.7f, 0.7f)),
playbtn(vec2(0.0f, 0.0f), vec2(0.125f, 0.125f)),
optionsbtn(vec2(0.0f, -0.25f), vec2(0.15f, 0.15f)),
quitbtn(vec2(0.0f, -0.47f), vec2(0.11f, 0.11f)),
homebtn(vec2(-0.15f, -0.3f), vec2(0.1f, 0.1f)),
nextbtn(vec2(0.15f, -0.3f), vec2(0.1f, 0.1f)),
actor,
door,
background,
complete(vec2(0.0f, 0.0f), vec2(0.5f, 0.5f));

PhysicsObject player(FRICTION_X, FRICTION_Y, ACCELERATION_X, DECELERATION_X, 5, false);

#pragma endregion

#pragma region level

class level {
public:
	level(std::string levelName) {
		readLevel(levelName);
	}
	std::vector<Sprite> walls;
	std::vector<Sprite> hazards;
	bool readLevel(std::string);
	void clearLevel();
	void loadLevel();
	void restartLevel();
	void nextLevel(std::string);
private:
	struct levelObject {
		std::string file;
		std::string type;
		vec2 pos;
		vec2 endPos;
		vec2 scale;
	};
	//const vec2 sDist = vec2(0.14f, 0.2492f);
	Sprite initSprite(std::string, vec2, vec2);
	void loadObject(std::vector<Sprite>*, levelObject);
	void tessX(std::vector<Sprite>*, levelObject);
	void tessY(std::vector<Sprite>*, levelObject);
	const vector <float> gridy = { -1,-0.8754,-0.6262,-0.377,-0.1278,0.1214,0.3706,0.6198,0.869,1 };
	const vector <float> gridx = { -1,-0.93,-0.79,-0.65,-0.51,-0.37,-0.23,-0.09,0.05,0.12,0.19,0.33,0.47,0.61,0.75,0.89,1 };

	std::vector<levelObject> objs;
	vec2 playerPos;
};
Sprite level::initSprite(std::string file, vec2 pos, vec2 scale) {
	Sprite s(pos, scale);
	s.Initialize(file);
	return s;
}

bool level::readLevel(std::string fileName) {
	std::ifstream infile(fileName);
	if (!infile) return false;
	std::string line;
	std::getline(infile, line);
	while (std::getline(infile, line)) {
		std::cout << line << std::endl;
		std::stringstream ss(line);
		levelObject obj;
		ss >> obj.file >> obj.type >> obj.pos.x >> obj.pos.y >> obj.endPos.x >> obj.endPos.y >> obj.scale.x >> obj.scale.y;
		objs.push_back(obj);
	}
	return true;
}
void level::clearLevel() {
	for (Sprite s : walls) {
		s.Release();
	}
	for (Sprite s : hazards) {
		s.Release();
	}
	walls.clear();
	hazards.clear();
	//actor.Release();
	//door.Release();
}

void level::tessX(std::vector<Sprite>* spriteType, levelObject obj) {
	int direction = (obj.pos.x > obj.endPos.x) ? -1 : 1;
	int index = 0;
	for (float x : gridx) {
		if (x == obj.pos.x) break;
		index++;
	}
	//printf("FILL X  | direction: %d, from: %f, to: %f\n", direction, obj.pos.x, obj.endPos.x);
	while (gridx[index] != obj.endPos.x) {
		//printf("index: %d, val: %f\n", index, gridx[index]);
		spriteType->push_back(initSprite(obj.file, vec2(gridx[index], obj.pos.y), obj.scale));
		index += direction;
	}
	if (gridx[index] == obj.endPos.x) spriteType->push_back(initSprite(obj.file, vec2(gridx[index], obj.pos.y), obj.scale));
}
void level::tessY(std::vector<Sprite>* spriteType, levelObject obj) {
	int direction = (obj.pos.y > obj.endPos.y) ? -1 : 1;
	int index = 0;
	for (float y : gridy) {
		if (y == obj.pos.y) break;
		index++;
	}
	//printf("FILL Y  | direction: %d, from: %f, to: %f\n", direction, obj.pos.y, obj.endPos.y);
	while (gridy[index] != obj.endPos.y) {
		//printf("index: %d, val: %f\n", index, gridy[index]);
		spriteType->push_back(initSprite(obj.file, vec2(obj.pos.x, gridy[index]), obj.scale));
		index += direction;
	}
	if (gridy[index] == obj.endPos.y) spriteType->push_back(initSprite(obj.file, vec2(obj.pos.x, gridy[index]), obj.scale));
}
void level::loadObject(std::vector<Sprite>* spriteType, levelObject obj) {
	if (obj.pos.x != obj.endPos.x) {
		tessX(spriteType, obj);
	}
	if (obj.pos.y != obj.endPos.y) {
		tessY(spriteType, obj);
	}
	//why walls, u mean spriteType?
	walls.push_back(initSprite(obj.file, obj.pos, obj.scale));
}

void level::loadLevel() {
	for (auto o : objs) {
		if (o.type == "wall") {
			loadObject(&walls, o);
		}
		else if (o.type == "hazard") {
			loadObject(&hazards, o);
		}
		else if (o.type == "actor") {
			actor.SetScale(o.scale);
			actor.SetPosition(o.pos);
			initPos = o.pos;
		}
		else if (o.type == "door") {
			door.SetScale(o.scale);
			door.SetPosition(o.pos);
		}

	}
	objs.clear();
}

void level::restartLevel() {
	actor.SetPosition(initPos);
}

void level::nextLevel(std::string fileName) {
	clearLevel();
	readLevel(fileName);
	loadLevel();
}

level* lv;

#pragma endregion

#pragma region glfw

void MouseButton(float x, float y, bool left, bool down) {
	if (down) {
		//On Menu Page
		if (playbtn.Hit(x, y)) {
			playbtn.Down(x, y);
			menu = false;
			game = true;
		}
		else if (optionsbtn.Hit(x, y)) {
			optionsbtn.Down(x, y);
			//menu = false;
			//options = true;
		}
		else if (quitbtn.Hit(x, y)) {
			quitbtn.Down(x, y);
			quit = true;
		}
		//On Level Complete
		else if (homebtn.Hit(x, y)) {
			homebtn.Down(x, y);
			//go back to menu page
			menu = true;
			game = false;
			finished = false;
			lv->restartLevel();
		}
		else if (nextbtn.Hit(x, y)) {
			nextbtn.Down(x, y);
			//load next level
			finished = false;
			lv->nextLevel("Level2.txt");
		}
	}
}

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
	if (menu) {
		//display the startGame sprite 
		title.Display();
		playbtn.Display();
		optionsbtn.Display();
		quitbtn.Display();
		return;
	}
	else if (options) {
		//display settings
		return;
	}
	for (Sprite w : lv->walls) {
		w.Display();
	}
	for (Sprite h : lv->hazards) {
		h.Display();
	}
	door.Display();
	actor.Display();
	//if game is finished
	std::cout << "finished: " << finished << endl;
	if (finished)
	{
		complete.Display();
		homebtn.Display();
		nextbtn.Display();
	}


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
	if (pressed[GLFW_KEY_ESCAPE]) quit = true;

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
	for (Sprite i : lv->walls) {
		//if actor touches any wall sprites
		if (temp.Intersect(i))
		{
			std::cout << "intersect wall_sprites" << std::endl;
			wall_hit = true;
		}
	}
	//Check if actor touches a hazard
	for (Sprite i : lv->hazards) {
		//if actor touches any hazard sprites
		if (actor.Intersect(i))
		{
			std::cout << "intersect hazard" << std::endl;
			//Return to starting point
			lv->restartLevel();
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

	//If reach the door/chest
	if (actor.Intersect(door)) {
		std::cout << "Finish, now what" << std::endl;
		finished = true;
	}
}

#pragma endregion

int main(int ac, char** av) {

	GLFWwindow* w = InitGLFW(winX, winY, winWidth, winHeight, "Platformer");

	GLFWimage icon[1];
	icon[0].pixels = stbi_load("coin.png", &icon[0].width, &icon[0].height, 0, 4);
	glfwSetWindowIcon(w, 1, icon);
	glfwMaximizeWindow(w);
	stbi_image_free(icon[0].pixels);

#pragma region init_sprites
	background.Initialize("Background.png");
	title.Initialize("Title.png");
	playbtn.Initialize("Play.png");
	optionsbtn.Initialize("Options.png");
	quitbtn.Initialize("Quit.png");
	homebtn.Initialize("Home.png");
	nextbtn.Initialize("Next.png");
	complete.Initialize("Complete.png");
	actor.Initialize("Body.png");
	door.Initialize("Door.png");

#pragma endregion

	RegisterMouseButton(MouseButton);
	RegKeyboard(Keystroke, w);
	printf("Move with W, A, and D. Exit with ESC\n");

	lv = new level("level2.txt");
	lv->loadLevel();

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

		if (pressed[GLFW_KEY_C]) lv->clearLevel();
		if (quit) glfwSetWindowShouldClose(w, GLFW_TRUE);
	}
}
