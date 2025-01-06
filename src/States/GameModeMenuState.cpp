#include "States/GameModeMenuState.hpp"
#include "States/GamePlayState.hpp"
#include "States/GameDifficultyMenuState.hpp"
#include "Button.hpp"
#include "Constants.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <memory>

std::unique_ptr<GameModeMenuState> GameModeMenuState::game_menu_state_ = std::make_unique<GameModeMenuState>();

GameModeMenuState::~GameModeMenuState()
{
	if (font_ != nullptr)
	{
		TTF_CloseFont(font_);
		font_ = nullptr;
	}
}

GameModeMenuState* GameModeMenuState::Instance()
{
	return game_menu_state_.get();
}

bool GameModeMenuState::Enter(Game* game)
{
	game_ = game;
	font_ = TTF_OpenFont("res/font/font.ttf", 58);

	if (font_ == nullptr)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	title_texture_ = std::make_unique<Texture>();
	title_texture_->LoadFromPath(game_->renderer_, "res/gfx/pong_title.png");

	single_player_button_ = std::make_unique<Button>(game_, font_, "Singleplayer");
	single_player_button_->SetPosition((constants::screen_width / 2) - (single_player_button_->GetTexture()->width_ / 2), constants::screen_height * 3 / 7);
	
	multi_player_button_ = std::make_unique<Button>(game_, font_, "Multiplayer");
	multi_player_button_->SetPosition((constants::screen_width / 2) - (multi_player_button_->GetTexture()->width_ / 2), constants::screen_height * 4 / 7);

	spectator_button_ = std::make_unique<Button>(game_, font_, "Spectator");
	spectator_button_->SetPosition((constants::screen_width / 2) - (spectator_button_->GetTexture()->width_ / 2), constants::screen_height * 5 / 7);
	
	return true;
}

void GameModeMenuState::Exit()
{
	TTF_CloseFont(font_);
	font_ = nullptr;
}

void GameModeMenuState::Pause()
{
}

void GameModeMenuState::Resume()
{
	single_player_button_->UpdateButtonFlags();
	multi_player_button_->UpdateButtonFlags();
	spectator_button_->UpdateButtonFlags();
}

void GameModeMenuState::HandleEvents()
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
			if (single_player_button_->MouseOverlapsButton())
			{
				game_->game_mode_ = GameMode::SINGLE_PLAYER;
				game_->PushState(GameDifficultyMenuState::Instance());
			}
			else if (multi_player_button_->MouseOverlapsButton())
			{
				game_->game_mode_ = GameMode::MULTI_PLAYER;
				game_->PushState(GamePlayState::Instance());
			}
			else if (spectator_button_->MouseOverlapsButton())
			{
				game_->game_mode_ = GameMode::SPECTATOR;
				game_->PushState(GamePlayState::Instance());
			}
		}
		else if (e.type == SDL_MOUSEMOTION)
		{
			single_player_button_->HandleEvent(&e);
			multi_player_button_->HandleEvent(&e);
			spectator_button_->HandleEvent(&e);
		}
	}
}

void GameModeMenuState::Tick()
{
	single_player_button_->Tick();
	multi_player_button_->Tick();
	spectator_button_->Tick();
}

void GameModeMenuState::Render()
{
	SDL_SetRenderDrawColor(game_->renderer_, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(game_->renderer_);

	const float scale = 5.0;

	title_texture_->Render(game_->renderer_, (constants::screen_width / 2) - ((title_texture_->width_ * scale) / 2), constants::screen_height * 1 / 7, nullptr, scale);

	single_player_button_->Render();
	multi_player_button_->Render();
	spectator_button_->Render();

	SDL_RenderPresent(game_->renderer_);
}