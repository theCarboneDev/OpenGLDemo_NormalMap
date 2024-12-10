#ifndef GAMELEVEL_H
#define GAMELEVEL_H
#include <vector>
#include <fstream>
#include <sstream>
#include "game_object.h"
#include "ResourceManager.h"

class GameLevel {
public:
	std::vector<GameObject> Bricks;

	GameLevel() {}

	void load(const char *file, unsigned int levelWidth, unsigned int levelHeight);

	void draw(SpriteRenderer &renderer);

	bool isCompleted();
private:
	void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

#endif // !GAMELEVEL_H
