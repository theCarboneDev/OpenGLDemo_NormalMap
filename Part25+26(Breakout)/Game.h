#ifndef GAME_H
#define GAME_H
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "GameLevel.h"
#include "ResourceManager.h"
#include "Ball.h"
#include <irrKlang/irrKlang.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tuple>

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT,
};
typedef std::tuple<bool, Direction, glm::vec2> Collision;

class Game {
public:
	GameState state;
	bool keys[1024];
	unsigned int width, height;

	std::vector<GameLevel> Levels;
	unsigned int level;
	unsigned int lives;
	float speedMod;

	Game(unsigned int Width, unsigned int Height);
	~Game();

	void Init();

	void doCollision();
	Collision checkCollision(GameObject& one, GameObject& two);
	Collision checkCollision(BallObject& one, GameObject& two);
	Direction VectorDirection(glm::vec2 target);

	void resetPlayer();
	void resetLevel();

	void processInput(float dt);
	void update(float dt);
	void render();
};

#endif