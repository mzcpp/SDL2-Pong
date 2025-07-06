#include "States/GameDifficultyMenuState.hpp"
#include "States/GamePlayState.hpp"
#include "Button.hpp"
#include "Constants.hpp"

#include <SDL.h>

#include <memory>
#include <iostream>

std::unique_ptr<GameDifficultyMenuState> GameDifficultyMenuState::game_difficulty_menu_state_ = std::make_unique<GameDifficultyMenuState>();

GameDifficultyMenuState::~GameDifficultyMenuState()
{
	if (font_ != nullptr)
	{
		TTF_CloseFont(font_);
		font_ = nullptr;
	}
}

GameDifficultyMenuState* GameDifficultyMenuState::Instance()
{
	return game_difficulty_menu_state_.get();
}

bool GameDifficultyMenuState::Enter(Game* game)
{
	game_ = game;
	font_ = TTF_OpenFont("res/font/font.ttf", 58);

	if (font_ == nullptr)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	easy_difficulty_button_ = std::make_unique<Button>(game_, font_, "Easy");
	easy_difficulty_button_->SetPosition((constants::screen_width / 2) - (easy_difficulty_button_->GetTexture()->width_ / 2), constants::screen_height * 3 / 8);
	
	medium_difficulty_button_ = std::make_unique<Button>(game_, font_, "Medium");
	medium_difficulty_button_->SetPosition((constants::screen_width / 2) - (medium_difficulty_button_->GetTexture()->width_ / 2), constants::screen_height * 4 / 8);
	
	hard_difficulty_button_ = std::make_unique<Button>(game_, font_, "Hard");
	hard_difficulty_button_->SetPosition((constants::screen_width / 2) - (hard_difficulty_button_->GetTexture()->width_ / 2), constants::screen_height * 5 / 8);

	impossible_difficulty_button_ = std::make_unique<Button>(game_, font_, "Impossible");
	impossible_difficulty_button_->SetPosition((constants::screen_width / 2) - (impossible_difficulty_button_->GetTexture()->width_ / 2), constants::screen_height * 6 / 8);
	
	return true;
}

void GameDifficultyMenuState::Exit()
{
	TTF_CloseFont(font_);
	font_ = nullptr;
}

void GameDifficultyMenuState::Pause()
{
}

void GameDifficultyMenuState::Resume()
{
	easy_difficulty_button_->UpdateButtonFlags();
	medium_difficulty_button_->UpdateButtonFlags();
	hard_difficulty_button_->UpdateButtonFlags();
	impossible_difficulty_button_->UpdateButtonFlags();
}

void GameDifficultyMenuState::HandleEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			game_->Stop();
		}

		if (e.type == SDL_MOUSEBUTTONUP)
		{
			if (easy_difficulty_button_->MouseOverlapsButton())
			{
				game_->game_difficulty_ = GameDifficulty::EASY;
				game_->PushState(GamePlayState::Instance());
			}
			else if (medium_difficulty_button_->MouseOverlapsButton())
			{
				game_->game_difficulty_ = GameDifficulty::MEDIUM;
				game_->PushState(GamePlayState::Instance());
			}
			else if (hard_difficulty_button_->MouseOverlapsButton())
			{
				game_->game_difficulty_ = GameDifficulty::HARD;
				game_->PushState(GamePlayState::Instance());
			}
			else if (impossible_difficulty_button_->MouseOverlapsButton())
			{
				game_->game_difficulty_ = GameDifficulty::IMPOSSIBLE;
				game_->PushState(GamePlayState::Instance());
			}
		}
		else if (e.type == SDL_MOUSEMOTION)
		{
			easy_difficulty_button_->HandleEvent(&e);
			medium_difficulty_button_->HandleEvent(&e);
			hard_difficulty_button_->HandleEvent(&e);
			impossible_difficulty_button_->HandleEvent(&e);
		}
		else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
		{
			game_->PopState();
		}
	}
}

void GameDifficultyMenuState::Tick()
{
	easy_difficulty_button_->Tick();
	medium_difficulty_button_->Tick();
	hard_difficulty_button_->Tick();
	impossible_difficulty_button_->Tick();
}

void GameDifficultyMenuState::Render()
{
	SDL_SetRenderDrawColor(game_->renderer_, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(game_->renderer_);

	easy_difficulty_button_->Render();
	medium_difficulty_button_->Render();
	hard_difficulty_button_->Render();
	impossible_difficulty_button_->Render();

	SDL_RenderPresent(game_->renderer_);
}