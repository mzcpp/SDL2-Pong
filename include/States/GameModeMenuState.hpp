#ifndef GAME_MODE_MENU_STATE_HPP
#define GAME_MODE_MENU_STATE_HPP

#include "GameState.hpp"
#include "Button.hpp"

#include <memory>
#include <vector>

#include <SDL_ttf.h>

class GameModeMenuState : public GameState
{
private:
	static std::unique_ptr<GameModeMenuState> game_menu_state_;

	Game* game_;
	TTF_Font* font_;
	
	std::unique_ptr<Texture> title_texture_;
	
	std::unique_ptr<Button> single_player_button_;
	std::unique_ptr<Button> multi_player_button_;

public:
	GameModeMenuState() = default;

	~GameModeMenuState() override;

	static GameModeMenuState* Instance();

	bool Enter(Game* game) override;

	void Exit() override;

	void Pause() override;

	void Resume() override;

	void HandleEvents() override;

	void Tick() override;

	void Render() override;
};

#endif
