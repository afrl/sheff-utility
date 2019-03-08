#pragma once

#include <functional>
#include <memory>

/// Forward declare exprtk types so the massive header does not have to appear in this header.
namespace exprtk
{
    template<typename T>
    class expression;
    template<typename T>
    class symbol_table;
}

namespace utility
{
    /// \brief A class describes a value that is given by a function that can include
    ///        spatial coordinate and time.
    ///
    /// This class uses exprtk to evaluate a user-provided function at the current position
    /// time.  The function is set with SpatialTemporalValue::set_function and the value is
    /// accessed with double SpatialTemporalValue::value(double x, double y, double z).
    class SpatialTemporalValue
    {
    public:
        SpatialTemporalValue();
        SpatialTemporalValue(const SpatialTemporalValue& other);
        ~SpatialTemporalValue();

        /// Set the function that will determine the current value.  The function is allowed to
        /// use any math functions documented by exprtk (see https://github.com/ArashPartow/exprtk).
        /// Additionally, it can reference the following variables:
        /// - time: the current simulation time in the analysis
        /// - prev_time: the previous simulation time in the analysis
        /// - delta_time: the current increment of simulation time in the analysis
        /// - x: the current x-coordinate
        /// - y: the current x-coordinate
        /// - z: the current x-coordinate
        void set_function(const std::string& function_string);
        /// Evaluate the current value of the function
        double value(double x, double y, double z);
        /// Assigns the function defining the other value to this value.
        SpatialTemporalValue& operator=(const SpatialTemporalValue& other);
        /// Returns the string that was used to define the expression for the value.
        const std::string& get_function() const;

    private:
        std::unique_ptr<exprtk::expression<double>> expression_;
        std::unique_ptr<exprtk::symbol_table<double>> symbol_table_;
        std::string function_string_;
        double x_cached_;
        double y_cached_;
        double z_cached_;
    };
}