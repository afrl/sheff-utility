#include "utility/GlobalVariables.h"

namespace utility
{
	double GlobalVariables::analysis_time__ = 0;
	double GlobalVariables::analysis_delta_time__ = 0;
    double GlobalVariables::analysis_prev_time__ = 0;
	unsigned short GlobalVariables::analysis_dimensionality__ = 3;


	const double & GlobalVariables::get_analysis_time()
	{
		return GlobalVariables::analysis_time__;
	}

	const double & GlobalVariables::get_analysis_delta_time()
	{
		return GlobalVariables::analysis_delta_time__;
	}

    const double & GlobalVariables::get_analysis_prev_time()
    {
        return GlobalVariables::analysis_prev_time__;
    }

	const unsigned short & GlobalVariables::get_dimensionality()
	{
		return GlobalVariables::analysis_dimensionality__;
	}

}