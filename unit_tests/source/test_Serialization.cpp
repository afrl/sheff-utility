#include "utility/serialization/Serialization.h"

#include <array>
#include <iostream>
#include <sstream>
#include <vector>

int main()
{
    utility::SerialByteCounter counter;
    std::stringstream ss;
    utility::OutputSerializer to_binary_ss(ss);
    utility::JSONSerializer to_json_cout(std::cout);

    std::vector<double> list_of_doubles = { 0., 1., 3.};

    counter(list_of_doubles);
    to_binary_ss(list_of_doubles);
    to_json_cout(utility::named_ref("list_of_doubles", list_of_doubles));
    to_json_cout(utility::named_value("ex_int", 1));

    to_json_cout.write_out();
    to_json_cout.reset();

    std::cout << "# of bytes: " << counter.n_bytes() << std::endl;

    utility::InputDeserializer input_from_ss(ss);
    std::vector<double> list_of_doubles_copy(3, 0.);
    input_from_ss(list_of_doubles_copy);

    to_json_cout(utility::named_ref("list_of_doubles_copy", list_of_doubles_copy));
    to_json_cout.write_out();
    to_json_cout.reset();

    assert(list_of_doubles == list_of_doubles_copy);
}