#include "Paddle.hpp"
#include "Constants.hpp"
#include "Game.hpp"

#include <SDL2/SDL.h>

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