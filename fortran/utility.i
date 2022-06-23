%module utility

//////////////////////////////////////////////////////////////////////////////////////////////////
// Import std typemaps

%include <typemaps.i>
%include <std_string.i>
%include <std_vector.i>
%include <stdint.i>

%template(StringList) std::vector<std::string>;
%template(BoolList) std::vector<bool>;
%template(IntList) std::vector<int>;

//////////////////////////////////////////////////////////////////////////////////////////////////
// utility bindings

%{
#include "utility/SingletonNamedObjectContainer.h"
%}

%include "utility/Bitset.h"
%include "utility/SingletonNamedObjectContainer.h"

%apply(SWIGTYPE *DATA, size_t SIZE) { (double* begin_a, int n_objects_a) };
%apply(SWIGTYPE *DATA, size_t SIZE) { (double* const data, const int size) };

%apply(SWIGTYPE *DATA, size_t SIZE) { (double* begin_a, uint64_t n_objects_a) };
%apply(SWIGTYPE *DATA, size_t SIZE) { (double* const data, const uint64_t size) };
%apply(SWIGTYPE *DATA, size_t ROWS, size_t COLS) { (double* const data, const uint64_t rows, const uint64_t cols) };

%include "utility/Slice.h"
%template(SliceInt) utility::Slice<int>;
%template(SliceDouble) utility::Slice<double>;
%template(slice) utility::slice<int, uint64_t>;
%template(slice) utility::slice<double, uint64_t>;