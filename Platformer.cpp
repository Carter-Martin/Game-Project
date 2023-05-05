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

Sprite background, actor, hazard, wall;
//actor and hazard
vector<Sprite*> sprites = { &actor, &hazard, &wall};
vector<Sprite*> hazard_sprites = { &hazard};
vector<Sprite*> wall_sprites = { &wall };
//Initial position
vec2 initPos = vec2(-0.5, 0.2);

bool fullscreen = 0;
int window_width = 800, window_height = 800;
bool pressed[GLFW_KEY_LAST] = {0}; // track pressed keys
bool Quit = false;
const float MAX_VELOCITY = 0.1;
const float GRAVITY = 0.001;
const float X_ACCELERATION = 0.001;
const float Y_ACCELERATION = 0.01;
vec2 velocity = vec2(0.0, 0.0);

void MouseButton(float x, float y, bool left, bool down) { if (left && down) actor.Down(x, y); }

void MouseMove(float x, float y, bool leftDown, bool rightDown) { if (leftDown) actor.Drag(x, y); }


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

//Display sprite outline
void Outline(Sprite& s, float width = 2, vec3 color = vec3(1, 1, 0)) {// if(true)return;
	UseDrawShader(mat4());
	vec2 pts[] = { s.PtTransform({-1,-1}), s.PtTransform({-1,1}), s.PtTransform({1,1}), s.PtTransform({1,-1}) };
	for (int i = 0; i < 4; i++)
		Line(pts[i], pts[(i + 1) % 4], width, color);
}

void Display() {
	background.Display();
	wall.Display();
	hazard.Display();
	actor.Display();
	//Show outline around all sprites
	for (Sprite* s : sprites) {
		Outline(*s, 2, vec3(1, 1, 1));
	}
}

void toggleFullscreen(GLFWwindow* window) {
	if (pressed[GLFW_KEY_F]) {
		fullscreen = !fullscreen;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, fullscreen ? monitor : NULL, fullscreen ? 0 : 50, fullscreen ? 0 : 50, fullscreen ? mode->width : window_width, fullscreen ? mode->height : window_height, GLFW_DONT_CARE);
	}
}

//Checks if the sprite postion is within bounds of the game
bool CheckBound(Sprite s)
{
	vec2 sPos = s.GetPosition();
	// outside x-axis bound?
	bool collisionX = sPos.x <= -0.9 || sPos.x>= 0.9;
	// outside y-axis bound?
	bool collisionY = sPos.y <= -0.9 || sPos.y >= 0.9;
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
	actorPos.x += velocity.x;

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
}

// Application
void Resize(int width, int height) { glViewport(0, 0, width, height); }

int main(int ac, char **av) {
	GLFWwindow *w = InitGLFW(50, 50, 800, 800, "Sprite Demo");

	// read background, foreground, and mat textures
	background.Initialize("Background.png");
	actor.Initialize("Pink_Monster.png");
	hazard.Initialize("Mace.png");
	wall.Initialize("Grass.png");

	//Position and Scale
	hazard.SetPosition(vec2(0.6, 0.2));
	hazard.SetScale(vec2(0.1,0.1));
	wall.SetPosition(vec2(-0.5, -0.5));
	wall.SetScale(vec2(0.1, 0.1));
	actor.SetPosition(initPos);
	actor.SetScale(vec2(0.1, 0.1));

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
