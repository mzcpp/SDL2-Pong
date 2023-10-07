#ifndef BALL_HPP
#define BALL_HPP

#include <SDL2/SDL.h>

class Game;
class Paddle;

class Ball
{
public:
	Game* game_;	
	SDL_FRect rect_;
	float vx_;
	float vy_;

	Ball() = default;

	void HandleEvent(SDL_Event* e);

	void Tick();

	void Render();

	bool CheckForCollision();

	void BounceBall(const Paddle& paddle);

	void Reset();

	SDL_FPoint GetRotatedPoint(const SDL_FPoint& point, const SDL_FPoint& pivot, int degrees);
};

#endif
