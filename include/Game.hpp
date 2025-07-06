#ifndef GAME_HPP
#define GAME_HPP

#include "Texture.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <memory>
#include <stack>

class GameState;

enum class GameMode
{
	SINGLE_PLAYER, MULTI_PLAYER
};

enum class GameDifficulty
{
	EASY, MEDIUM, HARD, IMPOSSIBLE
};

class Game
{
private:
	bool initialized_;
	bool running_;

public:
	SDL_Window* window_;
	SDL_Renderer* renderer_;

	GameMode game_mode_;
	GameDifficulty game_difficulty_;
	std::stack<GameState*> states_;

	Game();

	~Game();

	bool Initialize();

	void Finalize();

	void Run();

	void Stop();

	void ChangeState(GameState* state);
	
	void PushState(GameState* state);

	void PopState();

	void HandleEvents();

	void Tick();

	void Render();
};

#endif