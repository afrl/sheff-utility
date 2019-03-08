/// Unit test for SpatialTemporalValue.h

#include "utility/SpatialTemporalValue.h"
#include "utility/GlobalVariables.h"

#include <assert.h>

using namespace utility; 

int main()
{
    SpatialTemporalValue test;
    test.set_function("2x + y + time");

    GlobalVariables::analysis_time__ = 5;
    assert(test.value(1, 3, 4) == 10);

    return 0;
}
