#ifndef PADDLE_HPP
#define PADDLE_HPP

#include <SDL2/SDL.h>

class Game;

class Paddle
{
public:
	Game* game_;	
	SDL_FRect rect_;
	float vy_;

	Paddle() = default;

	void HandleEvent(SDL_Event* e);

	void Tick();

	void Render();
};

#endif