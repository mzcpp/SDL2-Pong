#ifndef PADDLE_HPP
#define PADDLE_HPP

#include <SDL.h>

class Game;

class Paddle
{
public:
	Game* game_;	
	SDL_FRect rect_;
	float vy_;

	Paddle();

	void HandleEvent(SDL_Event* e);

	void Tick();

	void Render();
};

#endif