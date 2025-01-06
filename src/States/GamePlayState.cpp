#include "States/GamePlayState.hpp"
#include "Constants.hpp"
#include "LineEquation.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <memory>
#include <cmath>
#include <array>

std::unique_ptr<GamePlayState> GamePlayState::game_play_state_ = std::make_unique<GamePlayState>();

GamePlayState::~GamePlayState()
{
}

void GamePlayState::DrawDividerRects()
{
	SDL_SetRenderDrawColor(game_->renderer_, 0xD3, 0xD3, 0xD3, 0xFF);

	constexpr int rect_size = 20;
	constexpr int rects_num = constants::screen_height / rect_size;

	for (int i = 0; i < rects_num; i += 2)
	{
		SDL_Rect rect = { (constants::screen_width / 2) - (rect_size / 2), i * rect_size, rect_size, rect_size };
		SDL_RenderFillRect(game_->renderer_, &rect);
	} 
}

GamePlayState* GamePlayState::Instance()
{
	return game_play_state_.get();
}

bool GamePlayState::Enter(Game* game)
{
	game_ = game;
	font_ = TTF_OpenFont("res/font/font.ttf", 98);

	if (font_ == nullptr)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	constexpr int ball_side_size = 14;

	ball_.game_ = game_;
	
	ball_.rect_.x = static_cast<float>((constants::screen_width / 2) - (ball_side_size / 2));
	ball_.rect_.y = static_cast<float>((constants::screen_height / 2) - (ball_side_size / 2));
	ball_.rect_.w = static_cast<float>(ball_side_size);
	ball_.rect_.h = ball_.rect_.w;

	ball_.vx_ = 5.0f;
	ball_.vy_ = 0.0f;
	ball_.direction_ray_.start_point.x = ball_.rect_.x + (ball_.rect_.w / 2);
	ball_.direction_ray_.start_point.y = ball_.rect_.y + (ball_.rect_.h / 2);
	ball_.direction_ray_.end_point.x = ball_.rect_.x + (ball_.rect_.w / 2) + ((constants::screen_width + constants::screen_height) * ball_.vx_);
	ball_.direction_ray_.end_point.y = ball_.rect_.y + (ball_.rect_.h / 2) + ((constants::screen_width + constants::screen_height) * ball_.vy_);

	player1_score_ = 0;
	player2_score_ = 0;

	constexpr float paddle_w_ = 20.0f;
	constexpr float paddle_h_ = 100.0f;
	constexpr int paddle_x_offset = 30;

	player1_paddle_.game_ = game_;
	player1_paddle_.rect_.x = static_cast<float>(constants::screen_width - paddle_w_ - paddle_x_offset);
	player1_paddle_.rect_.y = static_cast<float>((constants::screen_height / 2) - (paddle_h_ / 2));
	player1_paddle_.rect_.w = paddle_w_;
	player1_paddle_.rect_.h = paddle_h_;
	player1_paddle_.vy_ = 0.0f;
	
	player2_paddle_.game_ = game_;
	player2_paddle_.rect_.x = static_cast<float>(paddle_x_offset);
	player2_paddle_.rect_.y = static_cast<float>((constants::screen_height / 2) - (paddle_h_ / 2));
	player2_paddle_.rect_.w = paddle_w_;
	player2_paddle_.rect_.h = paddle_h_;
	player2_paddle_.vy_ = 0.0f;

	const SDL_Color score_color = { 0xD3, 0xD3, 0xD3, 0xFF };

	player1_score_texture_ = std::make_unique<Texture>();
	player1_score_texture_->LoadFromText(game_->renderer_, font_, std::to_string(player1_score_).c_str(), score_color);

	player2_score_texture_ = std::make_unique<Texture>();
	player2_score_texture_->LoadFromText(game_->renderer_, font_, std::to_string(player2_score_).c_str(), score_color);

	ball_resetting_ = false;
	ball_reset_ticks_ = 60;

	intersection_point_.x = std::numeric_limits<float>::max();
	intersection_point_.y = std::numeric_limits<float>::max();

	return true;
}

void GamePlayState::Exit()
{
	if (font_ != nullptr)
	{
		TTF_CloseFont(font_);
		font_ = nullptr;
	}

	player1_score_texture_->FreeTexture();
	player2_score_texture_->FreeTexture();
}

void GamePlayState::Pause()
{
}

void GamePlayState::Resume()
{
}

void GamePlayState::HandleEvents()
{
	SDL_Event e;

	constexpr int speed = 10;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			game_->Stop();
		}

		if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
		{
			if (e.key.keysym.sym == SDLK_UP)
			{
				player1_paddle_.vy_ = -speed;
			}
			
			if (e.key.keysym.sym == SDLK_DOWN)
			{
				player1_paddle_.vy_ = speed;
			}

			if (game_->game_mode_ == GameMode::MULTI_PLAYER)
			{
				if (e.key.keysym.sym == SDLK_w)
				{
					player2_paddle_.vy_ = -speed;
				}
				
				if (e.key.keysym.sym == SDLK_s)
				{
					player2_paddle_.vy_ = speed;
				}
			}
		}

		if (e.type == SDL_KEYUP && e.key.repeat == 0)
		{
			if (e.key.keysym.sym == SDLK_UP)
			{
				player1_paddle_.vy_ = 0;
			}
			
			if (e.key.keysym.sym == SDLK_DOWN)
			{
				player1_paddle_.vy_ = 0;
			}

			if (game_->game_mode_ == GameMode::MULTI_PLAYER)
			{
				if (e.key.keysym.sym == SDLK_w)
				{
					player2_paddle_.vy_ = 0;
				}
				
				if (e.key.keysym.sym == SDLK_s)
				{
					player2_paddle_.vy_ = 0;
				}
			}
		}

		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
		{
			game_->PopState();
		}
	}
}

void GamePlayState::Tick()
{
	if (game_->game_mode_ == GameMode::SINGLE_PLAYER)
	{
		
		if (ball_reset_ticks_ == 0)
		{
			ball_reset_ticks_ = 30;
			ball_resetting_ = false;
			ball_.Reset();
		}

		if (ball_resetting_)
		{
			--ball_reset_ticks_;
			player1_paddle_.Tick();
			player2_paddle_.Tick();
			return;
		}

		ball_.Tick();

		if (ball_.rect_.x + ball_.rect_.w < 0)
		{
			++player1_score_;
			ball_resetting_ = true;
			const SDL_Color score_color = { 0xD3, 0xD3, 0xD3, 0xFF };
			player1_score_texture_->LoadFromText(game_->renderer_, font_, std::to_string(player1_score_).c_str(), score_color);

		}
		else if (ball_.rect_.x > constants::screen_width)
		{
			++player2_score_;
			ball_resetting_ = true;
			const SDL_Color score_color = { 0xD3, 0xD3, 0xD3, 0xFF };
			player2_score_texture_->LoadFromText(game_->renderer_, font_, std::to_string(player2_score_).c_str(), score_color);
		}

		const float paddle_mid_point_y = player2_paddle_.rect_.y + (player2_paddle_.rect_.h / 2.0);
		float speed = 0.0f;

		if (game_->game_difficulty_ == GameDifficulty::EASY)
		{
			speed = 5.0f;
		}
		else if (game_->game_difficulty_ == GameDifficulty::MEDIUM)
		{
			speed = 6.0f;
		}
		else if (game_->game_difficulty_ == GameDifficulty::HARD || game_->game_difficulty_ == GameDifficulty::IMPOSSIBLE)
		{
			speed = 7.0f;
		}

		const Line top_edge = { 0.0f, 0.0f, static_cast<float>(constants::screen_width), 0.0f };
		const Line right_edge = { static_cast<float>(constants::screen_width), 0.0f, static_cast<float>(constants::screen_width), static_cast<float>(constants::screen_height) };
		const Line bottom_edge = { static_cast<float>(constants::screen_width), static_cast<float>(constants::screen_height), 0.0f, static_cast<float>(constants::screen_height) };
		const Line left_edge = { 0.0f, static_cast<float>(constants::screen_height), 0.0f, 0.0f };

		const std::array<Line, 4> edges = { top_edge, right_edge, bottom_edge, left_edge };

		for (std::size_t i = 0; i < edges.size(); ++i)
		{
			intersection_point_ = GetIntersectionPoint(edges[i], ball_.direction_ray_);

			if (intersection_point_.x != std::numeric_limits<float>::max() && intersection_point_.y != std::numeric_limits<float>::max())
			{
				if (game_->game_difficulty_ != GameDifficulty::IMPOSSIBLE)
				{
					break;
				}
				else
				{
					// Line reflected_line = ball_.direction_ray_;

					// while (!FloatingPointSame(intersection_point_.x, 0.0f) || !FloatingPointSame(intersection_point_.x, static_cast<float>(constants::screen_width)))
					// {
					// 	reflected_line.start_point = intersection_point_;
					// 	reflected_line.end_point = intersection_point_;

					// 	SDL_FPoint reflected_vector = { ball_.vx_, -ball_.vy_ };

					// 	reflected_line.end_point.x = reflected_line.end_point.x + ((constants::screen_width + constants::screen_height) * reflected_vector.x);
					// 	reflected_line.end_point.y = reflected_line.end_point.y + ((constants::screen_width + constants::screen_height) * reflected_vector.y);
					// }
				}
			}
		}

		// REWORK THIS
		const float dist = intersection_point_.y - paddle_mid_point_y;

		if (std::fabs(dist) > player2_paddle_.rect_.h / 2)
		{
			if (dist < 0.0)
			{
				player2_paddle_.vy_ = -speed;
			}
			else
			{
				player2_paddle_.vy_ = speed;
			}
		}
		else
		{
			player2_paddle_.vy_ = 0.0;
		}
	}
	
	player1_paddle_.Tick();
	player2_paddle_.Tick();
}

void GamePlayState::Render()
{
	SDL_SetRenderDrawColor(game_->renderer_, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(game_->renderer_);

	DrawDividerRects();

	ball_.Render();

	player1_paddle_.Render();
	player2_paddle_.Render();

	SDL_SetRenderDrawColor(game_->renderer_, 0xD3, 0xD3, 0xD3, 0xFF);

	constexpr int score_y_pos = 0;
	constexpr int score_x_offset = 400;

	player1_score_texture_->Render(game_->renderer_, constants::screen_width - score_x_offset, score_y_pos);
	player2_score_texture_->Render(game_->renderer_, score_x_offset - player2_score_texture_->width_, score_y_pos);
	
	SDL_RenderDrawLineF(game_->renderer_, ball_.rect_.x + (ball_.rect_.w / 2), ball_.rect_.y + (ball_.rect_.h / 2), ball_.direction_ray_.end_point.x, ball_.direction_ray_.end_point.y);

						SDL_FPoint intersect_copy = intersection_point_;
						Line reflected_line = { intersect_copy.x, intersect_copy.y, intersect_copy.x, intersect_copy.y };

						//while (!FloatingPointSame(intersect_copy.x, 0.0f) || !FloatingPointSame(intersect_copy.x, static_cast<float>(constants::screen_width)))
						{
							reflected_line.start_point = intersect_copy;
							reflected_line.end_point = intersect_copy;

							SDL_FPoint reflected_vector = { ball_.vx_, -ball_.vy_ };

							reflected_line.end_point.x = reflected_line.end_point.x + ((constants::screen_width + constants::screen_height) * reflected_vector.x);
							reflected_line.end_point.y = reflected_line.end_point.y + ((constants::screen_width + constants::screen_height) * reflected_vector.y);

							SDL_RenderDrawLineF(game_->renderer_, reflected_line.start_point.x, reflected_line.start_point.y, reflected_line.end_point.x, reflected_line.end_point.y);
							
						}
	
	{
		constexpr float box_size = 10.0f;
		const SDL_FRect collision_box = { intersection_point_.x - (box_size / 2), intersection_point_.y - (box_size / 2), box_size, box_size };
		SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0x00, 0x00, 0xff);
		SDL_RenderDrawRectF(game_->renderer_, &collision_box);
	}
	
	SDL_RenderPresent(game_->renderer_);
}

const SDL_FPoint GamePlayState::GetIntersectionPoint(const Line& line_1, const Line& line_2) const
{
	SDL_FPoint result_intersection = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	
	const auto line_1_coefficients = LinearEquationCoefficients(line_1.start_point.x, line_1.start_point.y, line_1.end_point.x, line_1.end_point.y);
    const auto line_2_coefficients = LinearEquationCoefficients(line_2.start_point.x, line_2.start_point.y, line_2.end_point.x, line_2.end_point.y);

    const float& A1 = std::get<0>(line_1_coefficients);
    const float& B1 = std::get<1>(line_1_coefficients);
    const float& C1 = std::get<2>(line_1_coefficients);
   
    const float& A2 = std::get<0>(line_2_coefficients);
    const float& B2 = std::get<1>(line_2_coefficients);
    const float& C2 = std::get<2>(line_2_coefficients);

    const float det = A1 * B2 - A2 * B1;

    if (det != 0)
    {
        const float x = (B2 * C1 - B1 * C2) / det;
        const float y = (A1 * C2 - A2 * C1) / det;

		const bool x_on_line_1 = (FloatingPointLessThan(std::min(line_1.start_point.x, line_1.end_point.x), x) || FloatingPointSame(std::min(line_1.start_point.x, line_1.end_point.x), x)) && 
								(FloatingPointLessThan(x, std::max(line_1.start_point.x, line_1.end_point.x)) || FloatingPointSame(x, std::max(line_1.start_point.x, line_1.end_point.x)));

		const bool y_on_line_1 = (FloatingPointLessThan(std::min(line_1.start_point.y, line_1.end_point.y), y) || FloatingPointSame(std::min(line_1.start_point.y, line_1.end_point.y), y)) && 
								(FloatingPointLessThan(y, std::max(line_1.start_point.y, line_1.end_point.y)) || FloatingPointSame(y, std::max(line_1.start_point.y, line_1.end_point.y)));

		const bool x_on_line_2 = (FloatingPointLessThan(std::min(line_2.start_point.x, line_2.end_point.x), x) || FloatingPointSame(std::min(line_2.start_point.x, line_2.end_point.x), x)) && 
								(FloatingPointLessThan(x, std::max(line_2.start_point.x, line_2.end_point.x)) || FloatingPointSame(x, std::max(line_2.start_point.x, line_2.end_point.x)));

		const bool y_on_line_2 = (FloatingPointLessThan(std::min(line_2.start_point.y, line_2.end_point.y), y) || FloatingPointSame(std::min(line_2.start_point.y, line_2.end_point.y), y)) && 
								(FloatingPointLessThan(y, std::max(line_2.start_point.y, line_2.end_point.y)) || FloatingPointSame(y, std::max(line_2.start_point.y, line_2.end_point.y)));					

        if (x_on_line_1 && y_on_line_1 && x_on_line_2 && y_on_line_2)
        {
			result_intersection.x = x;
			result_intersection.y = y;
        }
    }

	return result_intersection;
}
