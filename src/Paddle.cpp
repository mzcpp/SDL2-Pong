#include "Paddle.hpp"
#include "Constants.hpp"
#include "Game.hpp"

#include <SDL.h>

Paddle::Paddle() :
	game_(nullptr),
	vy_(0.0f)
{
	rect_.x = 0.0f;
	rect_.y = 0.0f;
	rect_.w = 0.0f;
	rect_.h = 0.0f;
}

void Paddle::HandleEvent(SDL_Event* e)
{
	(void)e;
}

void Paddle::Tick()
{
	rect_.y += vy_;

	if (rect_.y < 0)
	{
		rect_.y = 0;
	}
	else if (rect_.y > constants::screen_height - rect_.h)
	{
		rect_.y = constants::screen_height - rect_.h;
	}
}

void Paddle::Render()
{
	SDL_SetRenderDrawColor(game_->renderer_, 0xD3, 0xD3, 0xD3, 0xFF);
	SDL_RenderFillRectF(game_->renderer_, &rect_);
}