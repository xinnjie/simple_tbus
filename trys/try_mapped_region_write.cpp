#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>

int main(int argc, char* argv[ ])
{
    using namespace boost::interprocess;
    try {
        // creating our first shared memory object.
        shared_memory_object sharedmem1 (open_or_create, "Hello", read_write);

        // setting the size of the shared memory
        sharedmem1.truncate (256);

        // map the shared memory to current process
        mapped_region region (sharedmem1, read_write);

        // access the mapped region using get_address
        std::strcpy(static_cast<char* >(region.get_address()), "Hello World!\n");
        *static_cast<char*>(region.get_address()) = 'h';
        std::cout << static_cast<char*>(region.get_address()) << std::endl;

    } catch (interprocess_exception& e) {
        // .. .  clean up
    }
    return 0;
}