#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <SDL.h>

#include <cmath>
#include <limits>
#include <type_traits>
#include <tuple>

template <typename T>
const std::tuple<T, T, T> GetLinearEquationCoefficients(T x_1, T y_1, T x_2, T y_2)
{
	const T A = y_2 - y_1;
	const T B = x_1 - x_2;
	const T C = A * x_1 + B * y_1;

	//std::cout << A << "x + " << B << "y = " << C << '\n';

	return { A, B, C };
}

template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
bool FloatingPointSame(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
	return std::fabs(a - b) <= ((std::fabs(a) > std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
}

template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
bool FloatingPointGreaterThan(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
	return (a - b) > (std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon;
}

template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
bool FloatingPointLessThan(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
	return (b - a) > ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
}

struct Line
{
	SDL_FPoint start_point;
	SDL_FPoint end_point;

    Line()
    {
        start_point.x = std::numeric_limits<float>::max();
        end_point.y = std::numeric_limits<float>::max();
        start_point.x = std::numeric_limits<float>::max();
        end_point.y = std::numeric_limits<float>::max();
    }

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

template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
inline T PointsDistance(T x1, T y1, T x2, T y2)
{
	return std::sqrtf(std::powf(x2 - x1, 2) + std::powf(y2 - y1, 2) * static_cast<T>(1.0));
}

#endif