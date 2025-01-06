#ifndef GAME_PLAY_STATE_HPP
#define GAME_PLAY_STATE_HPP

#include "GameState.hpp"
#include "Paddle.hpp"
#include "Ball.hpp"
#include "Utility.hpp"

#include <memory>
#include <cmath>

class GamePlayState : public GameState
{
private:
	static std::unique_ptr<GamePlayState> game_play_state_;

	Game* game_;
	TTF_Font* font_;

	Ball ball_;

	int player1_score_;
	int player2_score_;
	std::unique_ptr<Texture> player1_score_texture_;
	std::unique_ptr<Texture> player2_score_texture_;

	bool ball_resetting_;
	int ball_reset_ticks_;

	SDL_FPoint intersection_point_;

	void DrawDividerRects();

public:
	Paddle player1_paddle_;
	Paddle player2_paddle_;
	
	GamePlayState() = default;

	~GamePlayState() override;

	static GamePlayState* Instance();

	bool Enter(Game* game) override;

	void Exit() override;

	void Pause() override;

	void Resume() override;

	void HandleEvents() override;

	void Tick() override;

	void Render() override;

	const SDL_FPoint GetIntersectionPoint(const Line& line_1, const Line& line_2) const;
};

#endif