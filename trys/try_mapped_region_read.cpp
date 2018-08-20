#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[ ])
{
    using namespace boost::interprocess;
    try {
        // opening an existing shared memory object
        shared_memory_object sharedmem2 (open_only, "Hello", read_only);

        // map shared memory object in current address space
        mapped_region mmap (sharedmem2, read_only);

        // need to type-cast since get_address returns void*
        char *str1 = static_cast<char*> (mmap.get_address());
        std::cout << str1 << std::endl;
        shared_memory_object::remove("Hello");

    } catch (interprocess_exception& e) {
        std::cout << e.what( ) << std::endl;
    }
    return 0;
}