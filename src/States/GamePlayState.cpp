#include "States/GamePlayState.hpp"
#include "Constants.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <memory>
#include <cmath>

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

	ball_.vx_ = 5;
	ball_.vy_ = 0;

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

	if (game_->game_mode_ == GameMode::SINGLE_PLAYER)
	{
		const float paddle_mid_point_y = player2_paddle_.rect_.y + (player2_paddle_.rect_.h / 2.0);
		float speed = 0.0f;

		if (game_->game_difficulty_ == GameDifficulty::EASY)
		{
			speed = 10.0f;
		}
		else if (game_->game_difficulty_ == GameDifficulty::MEDIUM)
		{
			speed = 11.0f;
		}
		else if (game_->game_difficulty_ == GameDifficulty::HARD)
		{
			speed = 12.0f;
		}

		const float dist = ball_.rect_.y - paddle_mid_point_y;

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
}

void GamePlayState::Tick()
{
	player1_paddle_.Tick();
	player2_paddle_.Tick();

	if (ball_reset_ticks_ == 0)
	{
		ball_reset_ticks_ = 60;
		ball_resetting_ = false;
		ball_.Reset();
	}

	if (ball_resetting_)
	{
		--ball_reset_ticks_;
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

	SDL_RenderPresent(game_->renderer_);
}