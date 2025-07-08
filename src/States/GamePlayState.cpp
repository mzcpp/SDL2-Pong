#include "States/GamePlayState.hpp"
#include "Constants.hpp"
#include "Utility.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <iostream>
#include <memory>
#include <cmath>
#include <vector>
#include <array>
#include <cassert>
#include <string>
#include <optional>

#define DEBUGGING 0

std::unique_ptr<GamePlayState> GamePlayState::game_play_state_ = std::make_unique<GamePlayState>();
	
GamePlayState::GamePlayState() : 
	game_(nullptr), 
	font_(nullptr), 
	player1_score_(0), 
	player2_score_(0), 
	player1_score_texture_(nullptr), 
	player2_score_texture_(nullptr), 
	ball_resetting_(false), 
	ball_reset_ticks_(0)
{
	intersection_point_.x = 0.0f;
	intersection_point_.y = 0.0f;
}

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

		const float paddle_mid_point_y = player2_paddle_.rect_.y + (player2_paddle_.rect_.h / 2.0f);
		const float dist = intersection_point_.y - paddle_mid_point_y;

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
	
#if DEBUGGING
	SDL_RenderDrawLineF(game_->renderer_, ball_.direction_ray_.start_point.x, ball_.direction_ray_.start_point.y, ball_.direction_ray_.end_point.x, ball_.direction_ray_.end_point.y);

	SDL_FPoint intersect_copy = intersection_point_;						
	Line reflected_line = ball_.direction_ray_;
	SDL_FPoint reflected_vector = { intersection_point_.x - reflected_line.start_point.x, intersection_point_.y - reflected_line.start_point.y };

	constexpr float epsilon = 0.01f;

	while (!FloatingPointSame(intersect_copy.x, 0.0f, epsilon) && !FloatingPointSame(intersect_copy.x, static_cast<float>(constants::screen_width), epsilon))
	{
		reflected_line.start_point = intersect_copy;
		reflected_line.end_point = intersect_copy;

		reflected_vector.y = -reflected_vector.y;

		constexpr float multiplier = 0.005f;

		reflected_line.end_point.x = reflected_line.end_point.x + ((constants::screen_width + constants::screen_height) * reflected_vector.x);
		reflected_line.end_point.y = reflected_line.end_point.y + ((constants::screen_width + constants::screen_height) * reflected_vector.y);

		SDL_RenderDrawLineF(game_->renderer_, reflected_line.start_point.x, reflected_line.start_point.y, reflected_line.end_point.x, reflected_line.end_point.y);

		std::vector<SDL_FPoint> found_crosses;
							
		for (std::size_t i = 0; i < edges.size(); ++i)
		{
			const std::optional<SDL_FPoint> cross_point_opt = GetLinesIntersectionPoint(edges[i], reflected_line);

			if (cross_point_opt.has_value())
			{
				found_crosses.emplace_back(cross_point_opt.value());
			}
		}

		if (found_crosses.empty())
		{
			break;
		}

		SDL_FPoint intersect = *std::min_element(found_crosses.begin(), found_crosses.end(), [this](SDL_FPoint p1, SDL_FPoint p2)
			{
				return PointsDistance(p1.x, p1.y, intersection_point_.x, intersection_point_.y) > PointsDistance(p2.x, p2.y, intersection_point_.x, intersection_point_.y);
			});

		intersect_copy = intersect;

		SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0x00, 0x00, 0xff);
		constexpr float box_size = 30.0f;
		const SDL_FRect collision_box = { intersect_copy.x - (box_size / 2), intersect_copy.y - (box_size / 2), box_size, box_size };
		SDL_RenderFillRectF(game_->renderer_, &collision_box);
		SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0xff, 0xff, 0xff);
	}
							
	SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0x00, 0x00, 0xff);

	constexpr float box_size = 30.0f;
	SDL_FRect collision_box = { intersection_point_.x - (box_size / 2), intersection_point_.y - (box_size / 2), box_size, box_size };
	SDL_RenderFillRectF(game_->renderer_, &collision_box);
	SDL_SetRenderDrawColor(game_->renderer_, 0xff, 0xff, 0xff, 0xff);
#endif
	
	SDL_RenderPresent(game_->renderer_);
}

const std::optional<SDL_FPoint> GamePlayState::GetLinesIntersectionPoint(const Line& line_1, const Line& line_2) const
{
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
			return intersection_point;
        }
    }

	return std::nullopt;
}

void GamePlayState::GetEdgeIntersectionPoint()
{
	std::vector<SDL_FPoint> found_crosses;

	for (std::size_t i = 0; i < edges.size(); ++i)
	{
		const std::optional<SDL_FPoint> cross_point_opt = GetLinesIntersectionPoint(edges[i], ball_.direction_ray_);

		if (cross_point_opt.has_value())
		{
			found_crosses.emplace_back(cross_point_opt.value());
		}
	}

	if (found_crosses.empty())
	{
		assert(false);
		return;
	}

	intersection_point_ = *std::min_element(found_crosses.begin(), found_crosses.end(), [this](SDL_FPoint p1, SDL_FPoint p2)
		{
			return PointsDistance(p1.x, p1.y, intersection_point_.x, intersection_point_.y) > PointsDistance(p2.x, p2.y, intersection_point_.x, intersection_point_.y);
		});


	if (game_->game_difficulty_ != GameDifficulty::IMPOSSIBLE)
	{
		return;
	}

	constexpr float epsilon = 0.01f;
	SDL_FPoint intersect_copy = intersection_point_;
	Line reflected_line = ball_.direction_ray_;
	SDL_FPoint reflected_vector = { intersection_point_.x - reflected_line.start_point.x, intersection_point_.y - reflected_line.start_point.y };

	while (!FloatingPointSame(intersect_copy.x, 0.0f, epsilon) && !FloatingPointSame(intersect_copy.x, static_cast<float>(constants::screen_width), epsilon))
	{
		reflected_line.start_point = intersect_copy;
		reflected_line.end_point = intersect_copy;

		reflected_vector.y = -reflected_vector.y;

		constexpr float multiplier = 0.005f;

		reflected_line.end_point.x = reflected_line.end_point.x + ((constants::screen_width + constants::screen_height) * reflected_vector.x);
		reflected_line.end_point.y = reflected_line.end_point.y + ((constants::screen_width + constants::screen_height) * reflected_vector.y);

		std::vector<SDL_FPoint> found_crosses;

		for (std::size_t i = 0; i < edges.size(); ++i)
		{
			const std::optional<SDL_FPoint> cross_point_opt = GetLinesIntersectionPoint(edges[i], reflected_line);

			if (cross_point_opt.has_value())
			{
				found_crosses.emplace_back(cross_point_opt.value());
			}
			else
			{
				continue;
			}
		}

		SDL_FPoint intersect = *std::min_element(found_crosses.begin(), found_crosses.end(), [this](SDL_FPoint p1, SDL_FPoint p2)
			{
				return PointsDistance(p1.x, p1.y, intersection_point_.x, intersection_point_.y) > PointsDistance(p2.x, p2.y, intersection_point_.x, intersection_point_.y);
			});

		intersect_copy = intersect;
	}

	intersection_point_ = intersect_copy;
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
