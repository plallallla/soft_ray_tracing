#pragma once
#include <limits>

struct Interval
{
    float _min{ std::numeric_limits<float>::min() };
    float _max{ std::numeric_limits<float>::max() };
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

inline Interval operator+(const Interval& ival, float displacement) 
{
    return { ival._min + displacement, ival._max + displacement };
}  

inline Interval operator+(float displacement, const Interval& ival) 
{
    return { ival._min + displacement, ival._max + displacement };
}  

static inline Interval EMPTY_INTERVAL = Interval
{
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::min(),
};

static inline Interval UNIVERSE_INTERVAL = Interval{};