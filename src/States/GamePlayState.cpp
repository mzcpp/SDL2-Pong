#include "States/GamePlayState.hpp"
#include "Constants.hpp"
#include "LineEquation.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <memory>
#include <cmath>
#include <vector>
#include <array>
#include <cassert>
#include <iomanip>

std::unique_ptr<GamePlayState> GamePlayState::game_play_state_ = std::make_unique<GamePlayState>();
	
bool pause = false;

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

	GetEdgeIntersectionPoint();

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
			//game_->PopState();
			pause = !pause;
		}
	}
}

void GamePlayState::Tick()
{
	if (pause)
	{
		return;
	}

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

		// for (std::size_t i = 0; i < edges.size(); ++i)
		// {
		// 	intersection_point_ = GetIntersectionPoint(edges[i], ball_.direction_ray_);

		// 	if (intersection_point_.x != std::numeric_limits<float>::max() && intersection_point_.y != std::numeric_limits<float>::max())
		// 	{
		// 		if (game_->game_difficulty_ != GameDifficulty::IMPOSSIBLE)
		// 		{
		// 			std::cout << i << '\n';
		// 			break;
		// 		}
		// 		else
		// 		{
		// 			// Line reflected_line = ball_.direction_ray_;

		// 			// while (!FloatingPointSame(intersection_point_.x, 0.0f) || !FloatingPointSame(intersection_point_.x, static_cast<float>(constants::screen_width)))
		// 			// {
		// 			// 	reflected_line.start_point = intersection_point_;
		// 			// 	reflected_line.end_point = intersection_point_;

		// 			// 	SDL_FPoint reflected_vector = { ball_.vx_, -ball_.vy_ };

		// 			// 	reflected_line.end_point.x = reflected_line.end_point.x + ((constants::screen_width + constants::screen_height) * reflected_vector.x);
		// 			// 	reflected_line.end_point.y = reflected_line.end_point.y + ((constants::screen_width + constants::screen_height) * reflected_vector.y);
		// 			// }
		// 		}
		// 	}
		// }

		const float paddle_mid_point_y = player2_paddle_.rect_.y + (player2_paddle_.rect_.h / 2.0);
		const float dist = intersection_point_.y - paddle_mid_point_y;


		// fix stutter back and forth
		if (!FloatingPointSame(paddle_mid_point_y, intersection_point_.y, 0.05f))
		{
			if (dist < 0.0f)
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
	
	SDL_RenderDrawLineF(game_->renderer_, ball_.direction_ray_.start_point.x, ball_.direction_ray_.start_point.y, ball_.direction_ray_.end_point.x, ball_.direction_ray_.end_point.y);

						std::vector<SDL_FPoint> ints;
						
						constexpr float epsilon = 0.1f;
						
						SDL_FPoint intersect_copy = intersection_point_;						
						Line reflected_line = ball_.direction_ray_;
						SDL_FPoint reflected_vector = { intersection_point_.x - reflected_line.start_point.x, intersection_point_.y - reflected_line.start_point.y };
						int count = 0;
						ints.push_back(intersect_copy);


						while (!FloatingPointSame(intersect_copy.x, 0.0f, epsilon) && !FloatingPointSame(intersect_copy.x, static_cast<float>(constants::screen_width), epsilon))
						{
							++count;
							

							reflected_line.start_point = intersect_copy;
							reflected_line.end_point = intersect_copy;

							reflected_vector.y = -reflected_vector.y;

							constexpr float multiplier = 0.005f;

							reflected_line.start_point.x = reflected_line.start_point.x + (multiplier * reflected_vector.x);
							reflected_line.start_point.y = reflected_line.start_point.y + (multiplier * reflected_vector.y);
							reflected_line.end_point.x = reflected_line.end_point.x + ((constants::screen_width + constants::screen_height) * reflected_vector.x);
							reflected_line.end_point.y = reflected_line.end_point.y + ((constants::screen_width + constants::screen_height) * reflected_vector.y);

							SDL_RenderDrawLineF(game_->renderer_, reflected_line.start_point.x, reflected_line.start_point.y, reflected_line.end_point.x, reflected_line.end_point.y);

							bool found = false;
							
							for (std::size_t i = 0; i < edges.size(); ++i)
							{
								SDL_FPoint intersect = GetLinesIntersectionPoint(edges[i], reflected_line);
								
								if (intersect.x != std::numeric_limits<float>::max() && intersect.y != std::numeric_limits<float>::max())
								{
									intersect_copy = intersect;
									found = true;
									ints.push_back(intersect_copy);
									SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0x00, 0x00, 0xff);
									constexpr float box_size = 30.0f;
									SDL_FRect collision_box = { intersect_copy.x - (box_size / 2), intersect_copy.y - (box_size / 2), box_size, box_size };
									SDL_RenderFillRectF(game_->renderer_, &collision_box);
									SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0xff, 0xff, 0xff);
									// std::cout << std::setprecision(10);
									// std::cout << intersect_copy.x << ' ' << intersect_copy.y << '\n';

									break;
								}
							}

							if (count > 10)
							{
								std::cout << ints.size() << '\n';
								
								for (SDL_FPoint point : ints)
								{
									std::cout << point.x << ' ' << point.y << '\n';
								}
							 	
								std::cout << "BREAK\n";
								break;
							}

							if (!found)
							{
								std::cout << "DID NOT FIND\n";
								std::cout << reflected_line.start_point.x << ' ' << reflected_line.start_point.y << '\n';
								std::cout << reflected_line.end_point.x << ' ' << reflected_line.end_point.y << '\n';
								std::cout << "DID NOT FIND\n";
								break;
							}
						}
	{
		SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0x00, 0x00, 0xff);

		constexpr float box_size = 30.0f;
		SDL_FRect collision_box = { intersection_point_.x - (box_size / 2), intersection_point_.y - (box_size / 2), box_size, box_size };
		SDL_RenderFillRectF(game_->renderer_, &collision_box);
		SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0xff, 0xff, 0xff);

		// collision_box = { intersect_copy.x - (box_size / 2), intersect_copy.y - (box_size / 2), box_size, box_size };
		// SDL_RenderFillRectF(game_->renderer_, &collision_box);
	}
	
	SDL_RenderPresent(game_->renderer_);
}

const SDL_FPoint GamePlayState::GetLinesIntersectionPoint(const Line& line_1, const Line& line_2) const
{
	SDL_FPoint result_intersection = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	
	const auto line_1_coefficients = GetLinearEquationCoefficients(line_1.start_point.x, line_1.start_point.y, line_1.end_point.x, line_1.end_point.y);
    const auto line_2_coefficients = GetLinearEquationCoefficients(line_2.start_point.x, line_2.start_point.y, line_2.end_point.x, line_2.end_point.y);

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
		const SDL_FPoint intersection_point = { x, y };

        if (IsPointOnLine(intersection_point, line_1) && IsPointOnLine(intersection_point, line_2))
        {
			result_intersection.x = x;
			result_intersection.y = y;
        }
    }

	return result_intersection;
}

void GamePlayState::GetEdgeIntersectionPoint()
{
	for (std::size_t i = 0; i < edges.size(); ++i)
	{
		intersection_point_ = GetLinesIntersectionPoint(edges[i], ball_.direction_ray_);

		if (intersection_point_.x != std::numeric_limits<float>::max() && intersection_point_.y != std::numeric_limits<float>::max())
		{
			if (game_->game_difficulty_ != GameDifficulty::IMPOSSIBLE)
			{
				return;
			}
			else
			{
				constexpr float epsilon = 0.01f;
				Line reflected_line = { intersection_point_.x, intersection_point_.y, intersection_point_.x, intersection_point_.y };
				SDL_FPoint intersect_copy = intersection_point_;

				while (!FloatingPointSame(intersect_copy.x, 0.0f, epsilon) && !FloatingPointSame(intersect_copy.x, static_cast<float>(constants::screen_width), epsilon))
				{

				}

				intersection_point_ = intersect_copy;
				
				// while (intersection point on either of the vertical lines)
					// reflect ray from intersection point
					// get another intersection point
			}
		}
	}
}

bool GamePlayState::IsPointOnLine(const SDL_FPoint point, const Line& line) const
{
	constexpr float epsilon = 0.01;

	const bool x_on_line = (FloatingPointLessThan(std::min(line.start_point.x, line.end_point.x), point.x, epsilon) || FloatingPointSame(std::min(line.start_point.x, line.end_point.x), point.x, epsilon)) && 
							(FloatingPointLessThan(point.x, std::max(line.start_point.x, line.end_point.x), epsilon) || FloatingPointSame(point.x, std::max(line.start_point.x, line.end_point.x), epsilon));

	const bool y_on_line = (FloatingPointLessThan(std::min(line.start_point.y, line.end_point.y), point.y, epsilon) || FloatingPointSame(std::min(line.start_point.y, line.end_point.y), point.y, epsilon)) && 
							(FloatingPointLessThan(point.y, std::max(line.start_point.y, line.end_point.y), epsilon) || FloatingPointSame(point.y, std::max(line.start_point.y, line.end_point.y), epsilon));

	return x_on_line && y_on_line;
}