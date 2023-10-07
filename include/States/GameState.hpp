#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "Game.hpp"

class GameState
{
public:
	virtual ~GameState()
	{
	}

	virtual bool Enter(Game* game) = 0;

	virtual void Exit() = 0;

	virtual void Pause() = 0;

	virtual void Resume() = 0;

	virtual void HandleEvents() = 0;

	virtual void Tick() = 0;
	
	virtual void Render() = 0;

	void ChangeState(Game* game, GameState* state)
	{
		game->ChangeState(state);
	}
};

#endif