#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <SDL2/SDL.h>

#include <cmath>

struct Line
{
	SDL_FPoint start_point;
	SDL_FPoint end_point;

    Line() = default;

    Line(float sx, float sy, float ex, float ey)
    {
        start_point.x = sx;
        start_point.y = sy;
        end_point.x = ex;
        end_point.y = ey;
    }
};

template <typename T>
class Vector2d
{
public:
	T x;
	T y;

	void Normalize()
	{
		T length = GetLength();
		x /= length;
		y /= length;
	}

	void SetLength(T length)
	{
		Normalize();
		x *= length;
		y *= length;
	}

	T GetLength()
	{
		return std::sqrt((x * x) + (y * y));
	}
};

#endif