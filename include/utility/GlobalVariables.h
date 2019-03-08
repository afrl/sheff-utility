#pragma once

namespace utility
{
	/// This effectively wraps some global variables that are shared by
	/// all the finite elements within an analysis.  A class is used to
	/// provide const access.  It is advised that the methods are used
	/// unless you are modifying the values.
	class GlobalVariables
	{
	public:
        /// The current time (simulated) within the current FEA analysis
		static double analysis_time__;
        /// The current time step (simulated) within the current FEA analysis
		static double analysis_delta_time__;
        /// The previous time (simulated) at the last step within the current FEA analysis
        static double analysis_prev_time__;
        /// The dimensionality of the analysis (1 for 1D, 2 for 2D, 3 for 3D)
		static unsigned short analysis_dimensionality__;

        /// Accessor to the current analysis time (simulated).  Use this method unless you are
        /// changing the value to ensure const-ness.
		static const double& get_analysis_time();
        /// Accessor to the current time increment (simulated).  Use this method unless you are
        /// changing the value to ensure const-ness.
        static const double& get_analysis_delta_time();
        /// Accessor to the previous analysis time (simulated).  Use this method unless you are
        /// changing the value to ensure const-ness.
        static const double& get_analysis_prev_time();
        /// Accessor to the dimensionality of the analysis (1 for 1D, 2 for 2D, 3 for 3D).
        /// Use this method unless you are changing the value to ensure const-ness.
		static const unsigned short& get_dimensionality();
	};
}