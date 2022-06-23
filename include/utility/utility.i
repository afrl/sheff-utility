%module utility

//////////////////////////////////////////////////////////////////////////////////////////////////
// Import std typemaps

%{
#include <array>
#include <memory>
#include <vector>
%}

%include <typemaps.i>
%include <std_string.i>
%include <std_array.i>
%include <std_shared_ptr.i>
%include <std_vector.i>
%include <stdint.i>

%template(ArrayD3) std::array<double, 3>;
%template(DoubleList) std::vector<double>;
%template(StringList) std::vector<std::string>;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Import numpy typemaps

%{
#define SWIG_FILE_WITH_INIT
%}

%include "numpy.i"

%init
%{
    import_array();
%}

//////////////////////////////////////////////////////////////////////////////////////////////////
// utility bindings

%{
#include "utility/Bitset.h"
#include "utility/SingletonNamedObjectContainer.h"
#include "utility/Slice.h"
%}

%include "utility/Bitset.h"

%include "utility/SingletonNamedObjectContainer.h"

%include "utility/Slice.h"
%template(SliceInt) utility::Slice<int, uint64_t>;
%template(SliceDouble) utility::Slice<double, uint64_t>;

%apply (double* INPLACE_ARRAY1, int DIM1) {(double* data, int size)}
%inline
%{
static utility::Slice<double> slice(double* data, int size)
{
    return utility::Slice<double>(data, size);
}
%}