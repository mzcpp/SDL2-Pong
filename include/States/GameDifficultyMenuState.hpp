#ifndef GAME_DIFFICULTY_MENU_STATE_HPP
#define GAME_DIFFICULTY_MENU_STATE_HPP

#include "GameState.hpp"
#include "Button.hpp"

#include <memory>
#include <vector>

class GameDifficultyMenuState : public GameState
{
private:
	static std::unique_ptr<GameDifficultyMenuState> game_difficulty_menu_state_;

	Game* game_;
	TTF_Font* font_;

	std::unique_ptr<Button> easy_difficulty_button_;
	std::unique_ptr<Button> medium_difficulty_button_;
	std::unique_ptr<Button> hard_difficulty_button_;
	std::unique_ptr<Button> impossible_difficulty_button_;

public:
	GameDifficultyMenuState() = default;

	~GameDifficultyMenuState() override;

	static GameDifficultyMenuState* Instance();

	bool Enter(Game* game) override;

	void Exit() override;

	void Pause() override;

	void Resume() override;

	void HandleEvents() override;

	void Tick() override;

	void Render() override;
};

#endif
