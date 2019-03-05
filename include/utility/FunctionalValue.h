#pragma once

namespace utility
{

    // Note: this follows the CRTP idiom and static polymorphism
    // See https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern

    /// Provides the interface of a functional value.
    template<typename T>
    class FunctionalValue
    {
    public:
        double value()
        {
            static_cast<T *>(this)->value_impl();
        }
    };

    template<typename T>
    double get_value(const FunctionalValue<T>& f_value)
    {
        return f_value.value();
    }

    class StaticValue : public FunctionalValue<StaticValue>
    {
    public:
        StaticValue(const double value) : value_(value) {}

        double value_impl()
        {
            return value_;
        }

    private:
        double value_;
    };
}