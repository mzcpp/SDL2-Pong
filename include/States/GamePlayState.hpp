#ifndef GAME_PLAY_STATE_HPP
#define GAME_PLAY_STATE_HPP

#include "Constants.hpp"
#include "GameState.hpp"
#include "Paddle.hpp"
#include "Ball.hpp"
#include "Utility.hpp"

#include <memory>
#include <cmath>
#include <array>
#include <optional>

const Line top_edge = { 0.0f, 0.0f, static_cast<float>(constants::screen_width), 0.0f };
const Line right_edge = { static_cast<float>(constants::screen_width), 0.0f, static_cast<float>(constants::screen_width), static_cast<float>(constants::screen_height) };
const Line bottom_edge = { static_cast<float>(constants::screen_width), static_cast<float>(constants::screen_height), 0.0f, static_cast<float>(constants::screen_height) };
const Line left_edge = { 0.0f, static_cast<float>(constants::screen_height), 0.0f, 0.0f };

const std::array<Line, 4> edges = { top_edge, right_edge, bottom_edge, left_edge };

class GamePlayState : public GameState
{
	friend class Ball;

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
	
	GamePlayState();

	~GamePlayState() override;

	static GamePlayState* Instance();

	bool Enter(Game* game) override;

	void Exit() override;

	void Pause() override;

	void Resume() override;

	void HandleEvents() override;

	void Tick() override;

	void Render() override;

	const std::optional<SDL_FPoint> GetLinesIntersectionPoint(const Line& line_1, const Line& line_2) const;

	void GetEdgeIntersectionPoint();

	bool IsPointOnLine(const SDL_FPoint point, const Line& line) const;
};

#endif