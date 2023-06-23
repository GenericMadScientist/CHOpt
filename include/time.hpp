#ifndef SIGHTREAD_TIME_HPP
#define SIGHTREAD_TIME_HPP

#include <compare>
#include <cstdint>
#include <ostream>

namespace SightRead {
class Measure;
class Second;

class Tick {
private:
    int m_value;

public:
    explicit Tick(int value)
        : m_value {value}
    {
    }
    [[nodiscard]] int value() const { return m_value; }

    std::strong_ordering operator<=>(const Tick&) const = default;
    bool operator==(const Tick&) const = default;

    Tick& operator+=(const Tick& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }
    Tick& operator-=(const Tick& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    friend Tick operator+(Tick lhs, const Tick& rhs)
    {
        lhs += rhs;
        return lhs;
    }
    friend Tick operator-(Tick lhs, const Tick& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend std::ostream& operator<<(std::ostream& os, const Tick& tick)
    {
        os << tick.value() << " ticks";
        return os;
    }
};

class Beat {
private:
    double m_value;

public:
    explicit Beat(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Second to_second(std::int64_t bpm) const;
    [[nodiscard]] Measure to_measure(double beat_rate) const;

    std::partial_ordering operator<=>(const Beat& rhs) const
    {
        return m_value <=> rhs.m_value;
    }

    Beat& operator+=(const Beat& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    Beat& operator-=(const Beat& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    Beat& operator*=(double rhs)
    {
        m_value *= rhs;
        return *this;
    }

    friend Beat operator+(Beat lhs, const Beat& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Beat operator-(Beat lhs, const Beat& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend Beat operator*(Beat lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const Beat& lhs, const Beat& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Beat& beats)
    {
        os << beats.value() << 'b';
        return os;
    }
};

class Measure {
private:
    double m_value;

public:
    explicit Measure(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Beat to_beat(double beat_rate) const
    {
        return Beat(m_value * beat_rate);
    }

    std::partial_ordering operator<=>(const Measure& rhs) const
    {
        return m_value <=> rhs.m_value;
    }

    Measure& operator+=(const Measure& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    Measure& operator-=(const Measure& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    Measure& operator*=(double rhs)
    {
        m_value *= rhs;
        return *this;
    }

    friend Measure operator+(Measure lhs, const Measure& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Measure operator-(Measure lhs, const Measure& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend Measure operator*(Measure lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const Measure& lhs, const Measure& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Measure& measures)
    {
        os << measures.value() << 'm';
        return os;
    }
};

class OdBeat {
private:
    double m_value;

public:
    explicit OdBeat(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Beat to_beat(double beat_rate) const
    {
        return Beat(m_value * beat_rate);
    }

    std::partial_ordering operator<=>(const OdBeat& rhs) const
    {
        return m_value <=> rhs.m_value;
    }

    OdBeat& operator+=(const OdBeat& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    OdBeat& operator-=(const OdBeat& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    OdBeat& operator*=(double rhs)
    {
        m_value *= rhs;
        return *this;
    }

    friend OdBeat operator+(OdBeat lhs, const OdBeat& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend OdBeat operator-(OdBeat lhs, const OdBeat& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend OdBeat operator*(OdBeat lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const OdBeat& lhs, const OdBeat& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }
};

class Second {
private:
    double m_value;

public:
    explicit Second(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Beat to_beat(std::int64_t bpm) const
    {
        constexpr double MS_PER_MINUTE = 60000.0;
        return Beat(m_value * bpm / MS_PER_MINUTE);
    }

    std::partial_ordering operator<=>(const Second& rhs) const
    {
        return m_value <=> rhs.m_value;
    }

    Second& operator+=(const Second& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    Second& operator-=(const Second& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    Second& operator*=(double rhs)
    {
        m_value *= rhs;
        return *this;
    }

    friend Second operator+(Second lhs, const Second& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Second operator-(Second lhs, const Second& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend Second operator*(Second lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const Second& lhs, const Second& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Second& seconds)
    {
        os << seconds.value() << 's';
        return os;
    }
};

inline Measure Beat::to_measure(double beat_rate) const
{
    return Measure(m_value / beat_rate);
}

inline Second Beat::to_second(std::int64_t bpm) const
{
    constexpr double MS_PER_MINUTE = 60000.0;
    return Second(m_value * MS_PER_MINUTE / bpm);
}
}

#endif
