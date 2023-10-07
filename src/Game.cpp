#include "Game.hpp"
#include "Constants.hpp"
#include "States/GameState.hpp"
#include "States/GamePlayState.hpp"
#include "States/GameModeMenuState.hpp"
#include "States/GameDifficultyMenuState.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <cstdint>
#include <iostream>

Game::Game() : 
	initialized_(false), 
	running_(false), 
	window_(nullptr), 
	renderer_(nullptr), 
	game_mode_(GameMode::SINGLE_PLAYER), 
	game_difficulty_(GameDifficulty::MEDIUM)
{
	initialized_ = Initialize();
}

Game::~Game()
{
	while (!states_.empty())
	{
		states_.top()->Exit();
		states_.pop();
	}

	Finalize();
}

bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
	{
		printf("%s\n", "Warning: Texture filtering is not enabled!");
	}

	window_ = SDL_CreateWindow(constants::game_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, constants::screen_width, constants::screen_height, SDL_WINDOW_SHOWN);

	if (window_ == nullptr)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

	if (renderer_ == nullptr)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	constexpr int img_flags = IMG_INIT_PNG;

	if (!(IMG_Init(img_flags) & img_flags))
	{
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not be initialized! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not be initialized! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}

	return true;
}

void Game::Finalize()
{
	SDL_DestroyWindow(window_);
	window_ = nullptr;

	SDL_DestroyRenderer(renderer_);
	renderer_ = nullptr;

	IMG_Quit();
	SDL_Quit();
	TTF_Quit();
	Mix_Quit();
}

void Game::ChangeState(GameState* state)
{
	if (!states_.empty())
	{
		states_.top()->Exit();
		states_.pop();
	}

	states_.emplace(state);
	states_.top()->Enter(this);
}
	
void Game::PushState(GameState* state)
{
	if (!states_.empty())
	{
		states_.top()->Pause();
	}

	states_.emplace(state);
	states_.top()->Enter(this);
}

void Game::PopState()
{
	if (!states_.empty())
	{
		states_.top()->Exit();
		states_.pop();
	}

	if (!states_.empty())
	{
		states_.top()->Resume();
	}
}

void Game::Run()
{
	if (!initialized_)
	{
		return;
	}

	running_ = true;
	ChangeState(GameModeMenuState::Instance());

	constexpr long double ms = 1.0 / 60.0;
	std::uint64_t last_time = SDL_GetPerformanceCounter();
	long double delta = 0.0;

	double timer = SDL_GetTicks();

	int frames = 0;
	int ticks = 0;

	while (running_)
	{
		const std::uint64_t now = SDL_GetPerformanceCounter();
		const long double elapsed = static_cast<long double>(now - last_time) / static_cast<long double>(SDL_GetPerformanceFrequency());

		last_time = now;
		delta += elapsed;

		HandleEvents();

		while (delta >= ms)
		{
			Tick();
			delta -= ms;
			++ticks;
		}

		//printf("%Lf\n", delta / ms);
		Render();
		++frames;

		if (SDL_GetTicks() - timer > 1000)
		{
			timer += 1000;
			//printf("Frames: %d, Ticks: %d\n", frames, ticks);
			frames = 0;
			ticks = 0;
		}
	}
}

void Game::Stop()
{
	running_ = false;
}

void Game::HandleEvents()
{
	states_.top()->HandleEvents();
}

void Game::Tick()
{
	states_.top()->Tick();
}

void Game::Render()
{
	states_.top()->Render();
}