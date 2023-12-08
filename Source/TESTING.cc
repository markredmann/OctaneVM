#include "Headers/Common.hpp"
#include "Headers/CoreMemory.hpp"
// #include "Headers/HybridAllocator.hpp"
#include <iostream>
#include <memory>

using std::cout;
using namespace Octane;

class DemoType {
    public:
        int x = 42;
        DemoType(void)
            { cout << "CONSTRUCTING!\n"; }
        DemoType(int nX)
            { x = nX;
              cout << x << " : CONSTRUCTING!\n"; }
        ~DemoType(void)
            { cout << "DECONSTRUCTING!\n"; }
};

int main(void) {
    CoreAllocator Memory;

    int* a = Memory.Request<int>(1, DEFAULT_ALLOC_FLAGS, 42);
    cout << *a << '\n';
    // HyChunk Chunk;
    // Chunk.Init(&Memory);
    // cout << "--- CHUNK INIT ---\n";

    // MemoryAddress A = Chunk.Allocate(sizeof(int));
    // MemoryAddress B = Chunk.Allocate(sizeof(int));
    // MemoryAddress C = Chunk.Allocate(sizeof(int));
    // MemoryAddress D = Chunk.Allocate(sizeof(int));
    // *A.As.i32Ptr = 424242;
    // *B.As.i32Ptr = 13371337;
    // *C.As.i32Ptr = 299792458;
    // *C.As.i32Ptr = 696969;
    // // A.Log();
    // // cout << "    A In Bounds? " << BoolStr(Chunk.AllocationInBounds(A)) << '\n';
    // // B.Log();
    // // cout << "    B In Bounds? " << BoolStr(Chunk.AllocationInBounds(B)) << '\n';
    // cout << "\n--- CHUNK LOG ---\n";
    // Chunk.Log();
    
    // Chunk.Deallocate(A);
    // Chunk.Deallocate(B);
    // Chunk.Deallocate(C);
    // // Chunk.Deallocate(D);

    // cout << "\n--- DEALLOCATED! CHUNK LOG ---\n";
    // Chunk.Log();
    
    // cout << "\n--- ATTEMPTING DEFRAG... ---\n";
    // Chunk.Defragment();
    
    // cout << "\n--- DEFRAGMENTED! CHUNK LOG ---\n";
    // Chunk.Log();

    // Chunk.Destroy(&Memory);
    // cout << "\n--- CHUNK DESTROY ---\n";

    // MemoryAddress Address = Memory.Allocate(sizeof(short));
    
    // cout << "\n--- FIRST ALLOCATION ---\n";
    // *Address.As.i32Ptr = 1337;
    // Address.Log();
    // cout << "\nMemory Usage : " << Memory.GetTotalAllocations() << "\n";
    
    // cout << "\n--- REALLOC & VALUEMOD ---\n";
    // (void)Memory.Reallocate(Address, sizeof(int) * 2);
    // *Address.As.i32Ptr = 299792458;

    // Address.Log();
    // cout << "\nMemory Usage : " << Memory.GetTotalAllocations() << "\n";
    
    // cout << "--- FREE ---\n";
    // Memory.Deallocate(Address);
    // cout << "\nMemory Usage : " << Memory.GetTotalAllocations() << "\n";

    return 0;
}
