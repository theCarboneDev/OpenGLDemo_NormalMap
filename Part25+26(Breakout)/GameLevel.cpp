#include "GameLevel.h"

void GameLevel::load(const char* file, unsigned int levelWidth, unsigned int levelHeight) {
	this->Bricks.clear();

	unsigned int tileCode;
	GameLevel level;
	std::string line;
    std::ifstream fstream(file);
	std::vector<std::vector<unsigned int>> tileData;
	if (fstream) {
		while (std::getline(fstream, line))
		{
			std::istringstream sstream(line);
			std::vector<unsigned int> row;
			while (sstream >> tileCode) {
				row.push_back(tileCode);
			}
			tileData.push_back(row);
		}
		if (tileData.size() > 0) {
			this->init(tileData, levelWidth, levelHeight);
		}
	}
}

void GameLevel::draw(SpriteRenderer& renderer) {
	for (auto &x : Bricks) {
		if (!x.Destroyed) {
			x.Draw(renderer);
		}
	}
}

bool GameLevel::isCompleted() {
	for (auto &x : Bricks) {
		if (!x.IsSolid && !x.Destroyed) {
			return false;
		}
	}
	return true;
}

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight) {
	unsigned int height = tileData.size();
	unsigned int width = tileData[0].size();
	float unitWidth = levelWidth / static_cast<float>(width);
	float unitHeight = levelHeight / static_cast<float>(height);
	for (int y{ 0 }; y < height; ++y) {
		for (int x{ 0 }; x < width; ++x) {
			if (tileData[y][x] == 1) {
				glm::vec2 pos{unitWidth * x, unitHeight * y};
				glm::vec2 size{ unitWidth, unitHeight };
				GameObject obj{ pos, size, ResourceManager::GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f) };
				obj.IsSolid = true;
				Bricks.push_back(obj);
			}
			else if (tileData[y][x] > 1) {
				glm::vec3 color{ 1.f };
				switch (tileData[y][x]) {
				case(2): color = glm::vec3(0.2f, 0.6f, 1.0f); break;
				case(3): color = glm::vec3(0.0f, 0.7f, 0.0f); break;
				case(4): color = glm::vec3(0.8f, 0.8f, 0.4f); break;
				case(5): color = glm::vec3(1.0f, 0.5f, 0.0f); break;
				}
				glm::vec2 pos{ unitWidth * x, unitHeight * y };
				glm::vec2 size{ unitWidth, unitHeight };
				Bricks.push_back(GameObject{ pos, size, ResourceManager::GetTexture("block"), color });
			}
		}
	}
}