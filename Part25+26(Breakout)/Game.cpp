#include "Game.h"

SpriteRenderer* renderer;

const glm::vec2 PLAYER_SIZE{ 100.f, 20.f };
const float PLAYER_VELOCITY{ 400.f };

const glm::vec2 INIT_BALL_VELOCITY{ 100.f,-350.f };
const float BALL_RADIUS{ 12.5f };

GameObject* Player;
BallObject* Ball;
TextRenderer* Text;

using namespace irrklang;

ISoundEngine* SoundEngine = createIrrKlangDevice();

Game::Game(unsigned int Width, unsigned int Height)
	: state(GAME_MENU), keys(), width(Width), height(Height)
{
	
}

Game::~Game() {
	delete renderer;
}
void Game::Init() {
	ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs", NULL, "sprite");
	SoundEngine->play2D("sound/silence.mp3", false);
	lives = 3;
	speedMod = 1.0f;

	Text = new TextRenderer(width, height);
	Text->Load("fonts/Prata-Regular.ttf", 48);

	glm::mat4 projection = glm::ortho(0.f, static_cast<float>(this->width), static_cast<float>(this->height), 0.f, -1.f, 1.f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

	renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	
	ResourceManager::LoadTexture("textures/block.png", false, "block");
	ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
	GameLevel one; one.load("level/one.txt", width, height / 2);
	GameLevel two; two.load("level/two.txt", width, height / 2);
	GameLevel three; three.load("level/three.txt", width, height / 2);
	GameLevel four; four.load("level/four.txt", width, height / 2);
	Levels.push_back(one);
	Levels.push_back(two);
	Levels.push_back(three);
	Levels.push_back(four);
	level = 2;

	ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
	glm::vec2 playerPos{ width / 2.f - PLAYER_SIZE.x / 2.f, height - PLAYER_SIZE.y };
	Player = new GameObject{ playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle") };

	glm::vec2 ballPos{playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, BALL_RADIUS * -2.f)};

	Ball = new BallObject{ballPos, BALL_RADIUS, INIT_BALL_VELOCITY, ResourceManager::GetTexture("face")};
}

void Game::resetPlayer() {
	Player->Position = glm::vec2(width / 2.f - PLAYER_SIZE.x / 2.f, height - PLAYER_SIZE.y);

	Ball->Position = Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, BALL_RADIUS * -2.f);
	Ball->Stuck = true;
	speedMod = 1.025f;
}

void Game::resetLevel() {
	for (auto &x : Levels[level].Bricks) {
		x.Destroyed = false;
	}
	lives = 3;
	speedMod = 1.0f;
}

Collision Game::checkCollision(GameObject &one, GameObject &two) //AABB - AABB collison
{
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x && two.Position.x + two.Size.x >= one.Position.x;
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y && two.Position.y + two.Size.y >= one.Position.y;

	return std::make_tuple(collisionX && collisionY, UP, glm::vec2(0.0));
}

Collision Game::checkCollision(BallObject& one, GameObject& two) {
	glm::vec2 center(one.Position + one.Radius);

	glm::vec2 aabb_half(two.Size.x/2.0, two.Size.y/2.0);
	glm::vec2 aabb_center(two.Position.x + aabb_half.x, two.Position.y + aabb_half.y);

	glm::vec2 difference = center - aabb_center;

	glm::vec2 clamped = glm::clamp(difference, -aabb_half, aabb_half);
	glm::vec2 closest = aabb_center + clamped;

	difference = closest - center;
	if (glm::length(difference) < one.Radius) {
		return std::make_tuple(true, VectorDirection(difference), difference);
	}
	else {
		return std::make_tuple(false, UP, glm::vec2(0.0));
	}
}

Direction Game::VectorDirection(glm::vec2 target) {
	glm::vec2 compass[]{
		glm::vec2(0.0, 1.0),
		glm::vec2(1.0, 0.0),
		glm::vec2(0.0, -1.0),
		glm::vec2(-1.0, 0.0),
	};
	float max = 0.f;
	int bestMatch = 0;
	for (int i{ 0 }; i < 4; i++) {
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max) {
			dot_product = max;
			bestMatch = i;
		}
	}
	return (Direction)bestMatch;
}

void Game::processInput(float dt) {
	if (state == GAME_ACTIVE) {
		float velocity = PLAYER_VELOCITY * dt;
		
		if (keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0.0f) {
				Player->Position.x -= velocity;
				if (Ball->Stuck) {
					Ball->Position.x -= velocity;
				}
			}
		}
		if (keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= width - Player->Size.x){
				Player->Position.x += velocity;
				if (Ball->Stuck) {
					Ball->Position.x += velocity;
				}
			}
		}
		if (keys[GLFW_KEY_SPACE]) {
			Ball->Stuck = false;
		}
	}
	else if (state == GAME_MENU) {
		if (keys[GLFW_KEY_ENTER]) {
			state = GAME_ACTIVE;
		}
		if (keys[GLFW_KEY_1]) {
			level = 0;
		}
		if (keys[GLFW_KEY_2]) {
			level = 1;
		}
		if (keys[GLFW_KEY_3]) {
			level = 2;
		}
		if (keys[GLFW_KEY_4]) {
			level = 3;
		}
	}
	else if (state == GAME_WIN) {
		if (keys[GLFW_KEY_ENTER]) {
			state = GAME_MENU;
		}
	}
}
void Game::update(float dt) {
	if (state == GAME_ACTIVE) {
		Ball->Move(dt, width, speedMod);

		doCollision();

		if (Ball->Position.y >= height) {
			SoundEngine->play2D("sound/lose.wav", false);
			lives--;

			if (lives <= 0) {
				resetLevel();
				state = GAME_MENU;
			}
			resetPlayer();
		}

		if (Levels[level].isCompleted()) {
			resetLevel();
			resetPlayer();
			state = GAME_WIN;
		}
	}
}

void Game::render() {
	renderer->drawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0, 0.0), glm::vec2(width, height), 0.f);
	if (this->state == GAME_ACTIVE || this->state == GAME_MENU) {

		Levels[level].draw(*renderer);

		Player->Draw(*renderer);

		Ball->Draw(*renderer);

		if (state == GAME_ACTIVE) {
			Text->RenderText("Lives: " + std::to_string(lives), 5.f, 5.f, 0.5f);
		}
		else {
			Text->RenderText("Press ENTER to Start", 250.f, height/2.f, 0.5f);
			Text->RenderText("Type '1', '2', '3', or '4' to select a level", 245.f, height / 2.f + 40.f, 0.5f);
		}
	}
	else if (state == GAME_WIN) {
		Text->RenderText("YOU WON!", 250.f, height / 2.f, 0.5f);
		Text->RenderText("Type ENTER to play again or ESC to quit", 245.f, height / 2.f + 40.f, 0.5f);
	}
}

void Game::doCollision() {
	for (GameObject& box : Levels[level].Bricks) {
		if (!box.Destroyed) {
			Collision coll = checkCollision(*Ball, box);
			if (std::get<0>(coll)){
				if (!box.IsSolid) {
					box.Destroyed = true;
					SoundEngine->play2D("sound/bleep.mp3", false);
					speedMod += 0.025;
				}
				else {
					SoundEngine->play2D("sound/solid.wav", false);
				}

				Direction dir = std::get<1>(coll);
				glm::vec2 dirVec = std::get<2>(coll);
				if (dir == LEFT || dir == RIGHT) {
					Ball->Velocity.x = -Ball->Velocity.x;

					float penetration = Ball->Radius - std::abs(dirVec.x);
					Ball->Position.x += dir == LEFT ? penetration : -penetration;
				}
				else {
					Ball->Velocity.y = -Ball->Velocity.y;

					float penetration = Ball->Radius - std::abs(dirVec.y);
					Ball->Position.y += dir == DOWN ? penetration : -penetration;
				}
			}
		}
	}

	if (!Ball->Stuck) {
		Collision result = checkCollision(*Ball, *Player);
		if (std::get<0>(result)) {
			SoundEngine->play2D("sound/bleepPaddle.wav", false);
			float centerBoard = Player->Position.x + Player->Size.x/2.0f;
			float distance = (Ball->Position.x + Ball->Size.x) - centerBoard;
			float percent = distance / (Player->Size.x/2.0f);

			float strength = 2.f;
			glm::vec2 oldVelocity = Ball->Velocity;
			Ball->Velocity.x = INIT_BALL_VELOCITY.x * percent * strength;
			Ball->Velocity.y = -1.0f * std::abs(Ball->Velocity.y);
			Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
		}

	}
}