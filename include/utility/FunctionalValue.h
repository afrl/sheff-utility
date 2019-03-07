#pragma once

#include "expressions/exprtk.hpp"

#include <functional>

namespace utility
{
    /// \brief A wrapper for a value that is given by a lambda function.
    ///
    /// This class is intended to be used for material properties and/or boundary condition
    /// values, allowing these parameters to become functions (especially time).  This class
    /// is suitable for values dependent on non-local 
    class TemporalValue
    {
    public:
        using Function = std::function<double()>;

        void set_function(const Function func)
        {
            func_ = func;
        }

        double value() const
        {
            return func_();
        }

    private:
        Function func_;
    };

    /// Provides the interface of a functional value.
    class SpatialTemporalValue
    {
    public:
        using Function = std::function<double(double x, double y, double z)>;

        double value(double x, double y, double z) const
        {
            return func_(x, y, z);
        }

    private:
        Function func_;
    };
}