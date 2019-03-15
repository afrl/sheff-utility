#include "utility/SpatialTemporalValue.h"

#include "utility/Exceptions.h"
#include "utility/GlobalVariables.h"

#include "second_party_math/ExprtkWrappers.h"

namespace utility
{
    SpatialTemporalValue::SpatialTemporalValue()
    {
        x_cached_ = 0;
        y_cached_ = 0;
        z_cached_ = 0;

        expression_.add_variable("time", GlobalVariables::analysis_time__);
        expression_.add_variable("prev_time", GlobalVariables::analysis_prev_time__);
        expression_.add_variable("delta_time", GlobalVariables::analysis_delta_time__);
        expression_.add_variable("x", x_cached_);
        expression_.add_variable("y", y_cached_);
        expression_.add_variable("z", z_cached_);

        expression_.register_symbol_table();
    }

    SpatialTemporalValue::SpatialTemporalValue(const SpatialTemporalValue & other)
        : SpatialTemporalValue()
    {
        *this = other;
    }

    SpatialTemporalValue::~SpatialTemporalValue()
    {
    }

    void SpatialTemporalValue::set_function(const std::string & function_string)
    {
        expression_.set_function(function_string);
        // Keep this for assignment operator
        function_string_ = function_string;
    }

    double SpatialTemporalValue::value(double x, double y, double z)
    {
        x_cached_ = x;
        y_cached_ = y;
        z_cached_ = z;
        return expression_.value();
    }

    SpatialTemporalValue & SpatialTemporalValue::operator=(const SpatialTemporalValue & other)
    {
        if (!other.function_string_.empty())
        {
            this->set_function(other.function_string_);
        }
        else
        {
            function_string_ = "";
        }
        return *this;
    }

    const std::string& SpatialTemporalValue::get_function() const
    {
        return function_string_;
    }
}