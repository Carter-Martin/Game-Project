/**
* Group 13
* DangerDash.cpp
* Move with W, A, and D. Exit with ESC. Change volume with - and =.
* 
* The goal of the game is to finish all the levels with the least amount
* of resets.
*/

#include <math.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include <glad.h>
#include <GLFW/glfw3.h>
#include <SFML/Audio.hpp>
#include <stdio.h>
#include <stb_image.h>

#include "Draw.h"
#include "GLXtras.h"
#include "Sprite.h"
#include "Text.h"

#pragma region declarations
const float VELOCITY_X = 0.3,
	JUMP = 275,
	GRAVITY = 1.5,
	FRICTION_X = 0.001,
	FRICTION_Y = 0.0001,
	ACCELERATION_X = 0.01,
	DECELERATION_X = 0.02,
	MASS = 0.02,
	X_DAMPER = 0.02,
	Y_DAMPER = 0.00008,
	MAX_VELOCITY = 0.3;

const vec2 DEFAULT_GRID = vec2(0.14, 0.2492);
const vec2 DEFAULT_SCALE = vec2(0.07, 0.1246);

std::vector<std::string> soundFiles({ "jump.wav", "death.wav", "fall_death.wav", "next_level.wav", "click-button.wav" });
std::vector<std::string> soundNames({ "jump", "death", "fall_death", "next_level", "click_button" });

bool pressed[GLFW_KEY_LAST] = { 0 }; // track pressed keys

vec2 initPos, endPos, scale = vec2(0.07f, 0.1246f);;
int deaths = 0, currentLevel = 0, winX = 50, winY = 50, winWidth = 1920, winHeight = 1080;
bool fullscreen = false, 
	playMusic = false, 
	menu = true, 
	options = false, 
	game = false, 
	quit = false, 
	finished = false,
	win = false;

Sprite* actor, * door, background, 
title(vec2(0.0f, 0.5f), vec2(0.7f, 0.7f)),
playbtn(vec2(0.0f, 0.0f), vec2(0.125f, 0.125f)),
optionsbtn(vec2(0.0f, -0.25f), vec2(0.15f, 0.15f)),
quitbtn(vec2(0.0f, -0.47f), vec2(0.11f, 0.11f)),
homebtn(vec2(-0.15f, -0.3f), vec2(0.1f, 0.1f)),
nextbtn(vec2(0.15f, -0.3f), vec2(0.1f, 0.1f)),
complete(vec2(0.0f, 0.0f), vec2(0.5f, 0.5f)),
increase(vec2(0.2f, 0.1f), vec2(0.08f, 0.08f)),
decrease(vec2(0.2f, -0.1f), vec2(0.08f, 0.08f)),
frame(vec2(0.0f, 0.0f), vec2(0.5f, 0.5f));


vector<Sprite*> spritesCollide;

sf::Music music;

#pragma endregion

/*
	PhysicsObject simulates the physics of a 2d rectangle with the 
	provided parameters.
*/
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
		if(!grounded) velocity.y += mass * gravity;
		//friction 
		float f_force = friction.x * mass * gravity;
		float f_direction = (velocity.x > 0) ? -1.0 : 1.0;
		if (velocity.x == 0) f_direction = 0;

		velocity.x += f_direction * f_force;

		//acceleration or deceleration
		if (isAccelerating) {
			velocity.x += acceleration;
		}
		else {
			float dec_direction = (velocity.x > 0) ? -1.0 : 1.0;
			if (velocity.x == 0) dec_direction = 0;
			velocity.x += dec_direction * deceleration;
			if ((dec_direction == -1 && velocity.x < 0) || (dec_direction == 1 && velocity.x > 0)) {
				velocity.x = 0.0;
			}
		}
		if (abs(velocity.x) > MAX_VELOCITY) velocity.x = MAX_VELOCITY;
		pos.x += velocity.x * X_DAMPER;
		pos.y += velocity.y * Y_DAMPER;

	}

	bool grounded = true,
		obstructed = false;

	void move_x(float vel) { if(!obstructed) velocity.x = vel; }
	void accelerate() { isAccelerating = true; }
	void decelerate() { isAccelerating = false; }
	void jump(float vel) { if(grounded) velocity.y = vel; }

	void setPos(vec2 p) { pos = p; }
	void setVel(vec2 v) { velocity = v; }

	//getters 

	vec2 getPos() { return pos; }
	vec2 getFriction() { return friction; }
	vec2 getVel() { return velocity; }
	float getAcc() { return acceleration; }
	float getDec() { return deceleration; }
	float getMass() { return mass; }

};

#pragma region sounds

/*
	Audio handles playing sound effects through SFML.
*/
class Audio {

private:
	sf::SoundBuffer buffer;
	sf::Sound sound;
	std::string file;
public:
	Audio(std::string f) {
		file = f;
		buffer.loadFromFile(file);
		sound.setBuffer(buffer);
	}
	void play() {
		sound.play();
	}
	void stop() {
		sound.stop();
	}
};
std::map<std::string, Audio*> audio;

// load a sound into the audio map
void loadsfx(std::vector<std::string> sounds, std::vector<std::string> names) {
	int i = 0;
	for (std::string s : sounds) {
		audio[names[i++]] = new Audio(s);
	}
}

// initialize the music 
int initMusic(std::string file) {
	if (!music.openFromFile(file))
		return -1; // error
	music.setLoop(true);

}

// stop the current music, and play a new file
void changeMusic(std::string file) {
	music.stop();
	initMusic(file);
	music.play();
}

// adjust the global volume by an integer. Global volume can range from 0 to 100.
void musicVolume(int add) {
	int currentVol = sf::Listener::getGlobalVolume();
	if(currentVol + add >= 0 && currentVol + add <= 100) sf::Listener::setGlobalVolume(currentVol + add);
}
#pragma endregion

#pragma region level

/*
	level handles all level sprites and objectives
*/
class level {
public:
	level(std::string levelName) {
		readLevel(levelName);
	}
	std::vector<Sprite> walls, floors, hazards;
	bool readLevel(std::string);
	void clearLevel();
	void loadLevel();
	void restartLevel(std::string);
	void nextLevel(std::string);
private:
	struct levelObject {
		std::string file, type;
		vec2 pos, endPos, scale;
	};
	Sprite* initSprite(std::string, vec2, vec2);
	void loadObject(std::vector<Sprite>*, levelObject);
	void tessX(std::vector<Sprite>*, levelObject);
	void tessY(std::vector<Sprite>*, levelObject);

	std::vector<levelObject> objs;
	vec2 playerPos;
};

// initialize the given file as a sprite and return it 
Sprite* level::initSprite(std::string file, vec2 pos, vec2 scale) {
	Sprite* s = new Sprite(pos, scale);
	s->Initialize(file);
	return s;
}

// read the level file and push the sprites to the objs vector
bool level::readLevel(std::string fileName) {
	std::ifstream infile(fileName);
	if (!infile) return false;
	std::string line;
	std::getline(infile, line);
	while (std::getline(infile, line)) {
		//std::cout << line << std::endl;
		std::stringstream ss(line);
		levelObject obj;
		ss >> obj.file >> obj.type >> obj.pos.x >> obj.pos.y >> obj.endPos.x >> obj.endPos.y >> obj.scale.x >> obj.scale.y;
		objs.push_back(obj);
	}
	return true;
}

// remove all level related sprites
void level::clearLevel() {
	for (Sprite s : walls) {
		s.Release();
	}
	for (Sprite s : floors) {
		s.Release();
	}
	for (Sprite s : hazards) {
		s.Release();
	}
	walls.clear();
	floors.clear();
	hazards.clear();
	spritesCollide.clear();
	actor->Release();
	door->Release();
}

// reset the position of the player
void level::restartLevel(std::string type = "") {
	actor->SetPosition(initPos);
	if (type != "") {
		audio[type]->play();
	}
}

// tesselate a sprite in the X direction. A positive value will add that many sprites 
// in the positive X direction. A negative value will add that many sprites in the 
// negative X direction. 
void level::tessX(std::vector<Sprite>* spriteType, levelObject obj) {
	int direction = (obj.endPos.x < 0) ? -1 : 1;
	float current = obj.pos.x;
	for (int i = 0; i < abs(obj.endPos.x); i++) {
		Sprite* s = initSprite(obj.file, vec2(current * DEFAULT_GRID.x, obj.pos.y * DEFAULT_GRID.y), (obj.scale* DEFAULT_SCALE));
		spriteType->push_back(*s);
		if (obj.type == "hazard") {
			spritesCollide.push_back(s);
		}
		current += direction;
	}
}

// tesselate a sprite in the Y direction. A positive value will add that many sprites 
// in the positive Y direction. A negative value will add that many sprites in the 
// negative Y direction. 
void level::tessY(std::vector<Sprite>* spriteType, levelObject obj) {
	int direction = (obj.endPos.y < 0) ? -1 : 1;
	float current = obj.pos.y;
	for (int i = 0; i < abs(obj.endPos.y); i++) {
		Sprite* s = initSprite(obj.file, vec2(obj.pos.x * DEFAULT_GRID.x, current * DEFAULT_GRID.y), (obj.scale*DEFAULT_SCALE));
		spriteType->push_back(*s);
		if (obj.type == "hazard") {
			spritesCollide.push_back(s);
		}
		current += direction;
	}
}

// loads the given object to the given sprite type vector.
void level::loadObject(std::vector<Sprite>* spriteType, levelObject obj) {
	if (obj.pos.x != obj.endPos.x) {
		tessX(spriteType, obj);
	}
	if (obj.pos.y != obj.endPos.y) {
		tessY(spriteType, obj);
	}
	Sprite* s = initSprite(obj.file, obj.pos* DEFAULT_GRID, obj.scale*DEFAULT_SCALE);
	spriteType->push_back(*s);
	if (obj.type == "hazard") {
		spritesCollide.push_back(s);
	}
}

// loads all objects from the objs vector into their respective sprite type vectors (walls, floors, hazards)
void level::loadLevel() {
	for (auto o : objs) {
		if (o.type == "wall") {
			loadObject(&walls, o);
		}
		if (o.type == "floor") {
			loadObject(&floors, o);
		}
		else if (o.type == "hazard") {
			loadObject(&hazards, o);
		}
		else if (o.type == "actor") {
			initPos = o.pos * DEFAULT_GRID;
			actor = initSprite(o.file, o.pos * DEFAULT_GRID, o.scale * DEFAULT_SCALE);
			spritesCollide.push_back(actor);
			
		}
		else if (o.type == "door") {
			door = initSprite(o.file, o.pos * DEFAULT_GRID, o.scale * DEFAULT_SCALE);
		}
	}
	objs.clear();
}

// loads the next level. If there is no next level, the game is finished.
void level::nextLevel(std::string fileName) {
	audio["next_level"]->play();
	clearLevel();
	if (!readLevel(fileName)) finished = true;
	loadLevel();
}

#pragma endregion

#pragma region game variables

PhysicsObject player(FRICTION_X, FRICTION_Y, ACCELERATION_X, DECELERATION_X, 5, false);
level* lv;

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
		pressed[key] = GLFW_PRESS;
	}
	else if (press == GLFW_RELEASE)
	{
		pressed[key] = GLFW_RELEASE;
	}
	
	if (pressed[GLFW_KEY_EQUAL]) {
		musicVolume(5);
		audio["click_button"]->play();
	}
	if (pressed[GLFW_KEY_MINUS]) {
		musicVolume(-5);
		audio["click_button"]->play();
	}
	if (key == GLFW_KEY_ESCAPE) quit = true;

}

void MouseButton(float x, float y, bool left, bool down) {
	if (down && !game) {
		//On Menu Page
		if (playbtn.Hit(x, y)) {
			playbtn.Down(x, y);
			menu = false;
			game = true;
			options = false;
			audio["click_button"]->play();
		}
		else if (optionsbtn.Hit(x, y)) {
			optionsbtn.Down(x, y);
			menu = false;
			game = false;
			options = true;
			audio["click_button"]->play();
		}
		else if (quitbtn.Hit(x, y)) {
			quitbtn.Down(x, y);
			quit = true;
			audio["click_button"]->play();
		}
		else if (homebtn.Hit(x, y)) {
			homebtn.Down(x, y);
			//go back to menu page
			menu = true;
			game = false;
			finished = false;
			options = false;
			lv->restartLevel();
		}
		else if (nextbtn.Hit(x, y)) {
			nextbtn.Down(x, y);
			//load next level
			finished = false;
			lv->nextLevel("Level2.txt");
		}
		//On Option Page
		else if (increase.Hit(x, y)) {
			increase.Down(x, y);
			audio["click_button"]->play();
			musicVolume(5);
			//Increase volume
		}
		else if (decrease.Hit(x, y)) {
			decrease.Down(x, y);
			audio["click_button"]->play();
			musicVolume(-5);
			//Decrease volume
		}
	}
}

#pragma endregion

#pragma region sprites

void Display() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(.5f, .5f, .5f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		frame.Display();
		increase.Display();
		decrease.Display();
		homebtn.Display();
		return;
	}

	for (Sprite w : lv->walls) {
		w.Display();
	}
	for (Sprite w : lv->floors) {
		w.Display();
	}
	for (Sprite h : lv->hazards) {
		h.Display();
	}
	door->Display();
	actor->Display();

	if (finished) {
		lv->clearLevel();
		lv->readLevel("win.txt");
		lv->loadLevel();
		finished = false;
		win = true;
	}
	if (win) complete.Display();
		

	glFlush();
}
// Checks if the sprite postion is within bounds of the game
bool CheckBound(Sprite s)
{
	vec2 sPos = s.GetPosition();
	bool collisionX = sPos.x <= -0.98 || sPos.x >= 0.98;
	return collisionX;
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
		if(player.grounded) audio["jump"]->play();
	}
}

#pragma endregion

#pragma region gameplay loop

void Update() {
	vec2 actorPos = actor->GetPosition();
	bool wall_hit = false, floor_hit = false;
	
	// calculate movement
	player.setPos(actorPos);
	movement();
	player.update();
	actorPos = player.getPos();

	// store new position in temp sprite to test if its a valid move
	Sprite temp = *actor;
	temp.SetPosition(actorPos);

	// check if actor touches wall
	for (Sprite i : lv->walls) {
		if (temp.Intersect(i))
		{
			wall_hit = true;
		}
	}
	// check if actor touched floor
	for (Sprite i : lv->floors) {
		if (temp.Intersect(i))
		{
			floor_hit = true;
		}
	}
	// stop x movement if a wall has been hit
	if (wall_hit) {
		player.setVel(vec2(0.0f, player.getVel().y));
		player.obstructed = true;
	}
	else {
		player.obstructed = false;
	}
	// stop y movement if a floor has been hit
	if (floor_hit) {
		player.grounded = true;
		player.setVel(vec2(player.getVel().x, 0.0f));
	}
	else {
		player.grounded = false;
	}
	// check if player is in bounds
	if (!CheckBound(temp)) {
		actor->SetPosition(actorPos);
	}
	// kill player if they fall out of bounds
	if (actorPos.y <= -1) {
		lv->restartLevel("fall_death");
		deaths++;
	}
	
	// level end reached
	if (actor->Intersect(*door)) {
		std::string file = "level" + std::to_string(++currentLevel) + ".txt";
		lv->nextLevel(file);
	}

	// check for hazard collisinos
	int nHitPixels = TestCollisions(spritesCollide);
	for (Sprite* s : spritesCollide) {
		bool hit = false;
		int w = 0;
		for (int i = 0; i < (int)s->collided.size(); i++)
			if (s->collided[i] > -1) {
				hit = true;
				w = i;
			}
		if (hit && s->id != actor->id && w == 0 && actor->position.x != initPos.x && actor->position.y != initPos.y) {
			lv->restartLevel("death");
			deaths++;
		}
			
	}

	// change the backgound music
	if (game && !playMusic) {
		changeMusic("background.wav");
		playMusic = true;
	}
	else if(!game && !options){ 
		music.pause();
		playMusic = false;
	}
	if (options && !playMusic) {
		changeMusic("options.wav");
		playMusic = true;
	}
	else if (!options && !game) {
		music.pause();
		playMusic = false;
	}
}

#pragma endregion

int main(int ac, char** av) {

	// init window
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* w = InitGLFW(winX, winY, winWidth, winHeight, "Platformer");
	
	// set icon
	GLFWimage icon[1];
	icon[0].pixels = stbi_load("coin.png", &icon[0].width, &icon[0].height, 0, 4);
	glfwSetWindowIcon(w, 1, icon);
	stbi_image_free(icon[0].pixels);
	
	// maximize window
	glfwMaximizeWindow(w);
	
	// glfw callbacks
	RegisterMouseButton(MouseButton);
	RegKeyboard(Keystroke, w);

	// init sprites
#pragma region init_sprites

	// big sprites
	background.Initialize("Background.png");
	title.Initialize("Title.png");
	complete.Initialize("Complete.png");

	// buttons
	playbtn.Initialize("Play.png");
	optionsbtn.Initialize("Options.png");
	quitbtn.Initialize("Quit.png");
	homebtn.Initialize("Home.png");
	nextbtn.Initialize("Next.png");
	
	// options
	increase.Initialize("Increase.png");
	decrease.Initialize("Decrease.png");
	frame.Initialize("Frame.png");

#pragma endregion
	
	// load sound effects
	loadsfx(soundFiles, soundNames);
	
	// initialize level
	lv = new level("level0.txt");
	lv->loadLevel();
	
	printf("Move with W, A, and D.\nExit with ESC. Change volume with - and =.\n");
	
	// game loop
	while (!glfwWindowShouldClose(w)) {
		Display();
		// show the score
		Text(10, 10, vec3(0,0,0), 20, "Resets: %i", deaths);
		glfwSwapBuffers(w);
		glfwPollEvents();
		Update();
		if (quit) glfwSetWindowShouldClose(w, GLFW_TRUE);
	}
}