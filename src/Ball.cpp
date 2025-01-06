#include "Ball.hpp"
#include "Game.hpp"
#include "Paddle.hpp"
#include "Constants.hpp"
#include "States/GamePlayState.hpp"

#include <SDL2/SDL.h>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

void Ball::HandleEvent(SDL_Event* e)
{
	(void)e;
}

void Ball::Tick()
{
	rect_.x += vx_;
	rect_.y += vy_;

	SDL_FRect intersect;
	
	if (SDL_HasIntersectionF(&rect_, &GamePlayState::Instance()->player1_paddle_.rect_))
	{
		rect_.x -= vx_;
		rect_.y -= vy_;

		if (SDL_IntersectFRect(&rect_, &GamePlayState::Instance()->player1_paddle_.rect_, &intersect))
		{
			rect_.x -= intersect.w / 2.0;
			rect_.y -= intersect.h / 2.0;
		}

		BounceBall(GamePlayState::Instance()->player1_paddle_);
	}
	
	if (SDL_HasIntersectionF(&rect_, &GamePlayState::Instance()->player2_paddle_.rect_))
	{
		rect_.x -= vx_;
		rect_.y -= vy_;

		if (SDL_IntersectFRect(&rect_, &GamePlayState::Instance()->player2_paddle_.rect_, &intersect))
		{
			rect_.x -= intersect.w / 2.0;
			rect_.y -= intersect.h / 2.0;
		}

		BounceBall(GamePlayState::Instance()->player2_paddle_);
	}

	if (rect_.y + rect_.w > constants::screen_height || rect_.y < 0)
	{
		rect_.y -= vy_;
		vy_ = -vy_;

		direction_ray_.start_point.x = rect_.x + (rect_.w / 2);
		direction_ray_.start_point.y = rect_.y + (rect_.h / 2);
		direction_ray_.end_point.x = rect_.x + (rect_.w / 2) + ((constants::screen_width + constants::screen_height) * vx_);
		direction_ray_.end_point.y = rect_.y + (rect_.h / 2) + ((constants::screen_width + constants::screen_height) * vy_);
	}
}

void Ball::Render()
{
	SDL_SetRenderDrawColor(game_->renderer_, 0xD3, 0xD3, 0xD3, 0xFF);
	SDL_RenderFillRectF(game_->renderer_, &rect_);
}

void Ball::BounceBall(const Paddle& paddle)
{
	const int mid_level = rect_.y + (rect_.w / 2);
	const double collision_point_normalized = std::clamp(static_cast<double>(mid_level - paddle.rect_.y) / static_cast<double>(paddle.rect_.h), 0.0, 1.0);

	constexpr int right_angle = 90;
	const double reflection_angle = ((right_angle / 2) + (right_angle * collision_point_normalized));
	
	SDL_FPoint reflection_vector;
	reflection_vector.x = 0.0;
	reflection_vector.y = 1.0;

	SDL_FPoint pivot;
	pivot.x = 0.0;
	pivot.y = 0.0;

	reflection_vector = GetRotatedPoint(reflection_vector, pivot, reflection_angle);

	constexpr float speed_multiple = 15.0;
	
	vx_ = vx_ > 0 ? reflection_vector.x : -reflection_vector.x;
	vy_ = -reflection_vector.y;

	direction_ray_.start_point.x = rect_.x + (rect_.w / 2);
	direction_ray_.start_point.y = rect_.y + (rect_.h / 2);
	direction_ray_.end_point.x = rect_.x + (rect_.w / 2) + ((constants::screen_width + constants::screen_height) * vx_);
	direction_ray_.end_point.y = rect_.y + (rect_.h / 2) + ((constants::screen_width + constants::screen_height) * vy_);

	vx_ *= speed_multiple;
	vy_ *= speed_multiple;
}

void Ball::Reset()
{
	constexpr int ball_side_size = 14;

	rect_.x = static_cast<float>((constants::screen_width / 2) - (ball_side_size / 2));
	rect_.y = static_cast<float>((constants::screen_height / 2) - (ball_side_size / 2));

	std::srand(std::time(nullptr));
	std::rand();

	constexpr float initial_speed = 5.0f;

	vx_ = (std::rand() % 2 == 0) ? initial_speed : -initial_speed;
	vy_ = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * initial_speed;
	
	direction_ray_.start_point.x = rect_.x + (rect_.w / 2);
	direction_ray_.start_point.y = rect_.y + (rect_.h / 2);
	direction_ray_.end_point.x = rect_.x + (rect_.w / 2) + ((constants::screen_width + constants::screen_height) * vx_);
	direction_ray_.end_point.y = rect_.y + (rect_.h / 2) + ((constants::screen_width + constants::screen_height) * vy_);
}

SDL_FPoint Ball::GetRotatedPoint(const SDL_FPoint& point, const SDL_FPoint& pivot, int degrees)
{
	SDL_FPoint result_point = point;

	const double pi = std::acos(-1);
	const double deg_to_rad = static_cast<double>(degrees) * pi / 180.0;
	const double sin_degrees = std::sin(deg_to_rad);
	const double cos_degrees = std::cos(deg_to_rad);

	const double new_x = (result_point.x - pivot.x) * cos_degrees - (result_point.y - pivot.y) * sin_degrees;
	const double new_y = (result_point.x - pivot.x) * sin_degrees + (result_point.y - pivot.y) * cos_degrees;

	result_point.x = new_x + pivot.x;
	result_point.y = new_y + pivot.y;

	return result_point;
}
