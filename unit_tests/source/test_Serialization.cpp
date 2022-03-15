#include "utility/serialization/Serialization.h"

#include <array>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

struct Foo
{
    double a = 0.0;

    template<typename SerializerType>
    void serialize(SerializerType& s) const
    {
        s(utility::named_value("a", a));
    }

    template<typename SerializerType>
    void deserialize(SerializerType& s)
    {
        s(a);
    }
};

int main(int argc, char** argv)
{
    utility::MPIUtils::init(&argc, &argv);

    {
        utility::SerialByteCounter counter;
        std::stringstream ss;
        utility::OutputSerializer to_binary_ss(ss);
        utility::JSONSerializer to_json_cout(std::cout);

        std::stringstream ss2;
        utility::LocatableBinarySerializer to_locatable_binary_ss(std::cout, ss2);

        std::vector<double> list_of_doubles = { 0., 1., 3. };
        Foo foo;

        counter(list_of_doubles);
        counter(foo);
        to_binary_ss(list_of_doubles);
        to_binary_ss(foo);

        to_json_cout(utility::named_ref("list_of_doubles", list_of_doubles));
        to_json_cout(utility::named_value("foo", foo));

        to_locatable_binary_ss(utility::named_ref("list_of_doubles", list_of_doubles));
        to_locatable_binary_ss(utility::named_value("foo", foo));

        std::cout << "JSON representation of data:" << std::endl;
        to_json_cout.write_out();
        to_json_cout.reset();

        std::cout << "Binary lookup table:" << std::endl;
        to_locatable_binary_ss.write_lookup_table();

        assert(ss.str().size() == ss2.str().size());

        std::cout << "# of bytes: " << counter.n_bytes() << std::endl;

        utility::InputDeserializer input_from_ss(ss);
        std::vector<double> list_of_doubles_copy(3, 0.);
        input_from_ss(list_of_doubles_copy);

        to_json_cout(utility::named_ref("list_of_doubles_copy", list_of_doubles_copy));
        to_json_cout.write_out();

        assert(list_of_doubles == list_of_doubles_copy);

        Foo foo_copy;
        utility::save("test_serialization_dump",
                      utility::named_ref("list_of_doubles", list_of_doubles),
                      utility::named_value("foo", foo));
        utility::load("test_serialization_dump", 0, { "list_of_doubles" }, list_of_doubles_copy);
        utility::load("test_serialization_dump", 0, { "foo" }, foo_copy);
        assert(list_of_doubles == list_of_doubles_copy);
        assert(foo.a == foo_copy.a);
    }

    // Note: odd behavior was observed for uint8_t related to new line chars, so test
    // to be sure we don't break that again.
    {
        std::ofstream file_out("int_array.bin", std::ios_base::binary);
        utility::OutputSerializer to_binary_ss(file_out);
        std::vector<uint8_t> int_array = { 9, 10, 11, 12, 13, 14 };
        to_binary_ss(utility::slice(int_array));
        file_out.close();

        // Note: the file is indeed 7 bytes on Windows
        // When 10 is written ('\n') then Windows decides to write 13 10 ('\r\n').
        // Reading still works oddly enough.
        // Resolution: adding std::ios_base::binary fixes the issue
        // Unfortunately, there is no way to test if the stream was opened with the binary flag
        // inside OutputSerializer (ofstream does not store the openmode)

        std::ifstream file_in("int_array.bin");
        utility::InputDeserializer input_from_ss(file_in);
        std::vector<uint8_t> int_array_copy(int_array.size(), 0);
        input_from_ss(utility::slice(int_array_copy));
        file_in.close();
        assert(int_array == int_array_copy);
    }
}