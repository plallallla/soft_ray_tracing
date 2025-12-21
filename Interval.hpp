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
    inline float get_max() const { return _max; }
    inline float get_min() const { return _min; }
    static inline float f_max = std::numeric_limits<float>::max();
    static inline float f_min = std::numeric_limits<float>::min();
};

static inline Interval EMPTY_INTERVAL = Interval
{
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::min(),
};

static inline Interval UNIVERSE_INTERVAL = Interval{};