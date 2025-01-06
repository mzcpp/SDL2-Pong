#include <tuple>
#include <type_traits>

template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
bool FloatingPointSame(T a, T b)
{
    return std::fabs(a - b) <= ((std::fabs(a) > std::fabs(b) ? std::fabs(b) : std::fabs(a)) * std::numeric_limits<T>::epsilon());
}

template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
bool FloatingPointGreaterThan(T a, T b)
{
    return (a - b) > (std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * std::numeric_limits<T>::epsilon();
}

template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
bool FloatingPointLessThan(T a, T b)
{
    return (b - a) > ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * std::numeric_limits<T>::epsilon());
}

template <typename T>
const std::tuple<T, T, T> LinearEquationCoefficients(T x_1, T y_1, T x_2, T y_2)
{
    const T A = y_2 - y_1;
    const T B = x_1 - x_2;
    const T C = A * x_1 + B * y_1;

    //std::cout << A << "x + " << B << "y = " << C << '\n';

    return { A, B, C };
}