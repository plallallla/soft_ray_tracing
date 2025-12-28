#pragma once
#include <limits>
#include <cmath>

struct Interval
{
    float _min{ std::numeric_limits<float>::max() };
    float _max{ std::numeric_limits<float>::min() };
    Interval() = default;
    Interval(float min_val, float max_val) : _min{min_val}, _max{max_val} {}
    inline bool contains(float val) const { return _max >= val && _min <= val; }
    inline bool surrounds(float val) const { return _max > val && _min < val; }
    inline float length() const { return _max - _min; }
    void expand(float half_delta)
    {
        _min -= half_delta;
        _max += half_delta;
    }
    bool operator==(const Interval& other)
    {
        return this->_max == other._max && this->_min == other._min;
    } 
    static inline float f_max = std::numeric_limits<float>::max();
    static inline float f_min = std::numeric_limits<float>::min();
};

inline Interval operator+(const Interval& _1, const Interval& _2) 
{
    return { std::fmin(_1._min, _2._min), std::fmax(_1._max, _2._max) };
}

inline Interval operator+(const Interval& ival, float displacement) 
{
    return { ival._min + displacement, ival._max + displacement };
}

inline Interval operator+(float displacement, const Interval& ival) 
{
    return { ival._min + displacement, ival._max + displacement };
}

inline static Interval EMPTY_INTERVAL;

inline static Interval UNIVERSE_INTERVAL
{
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::min(),
};
