#include "Headers/Common.hpp"
#include "Headers/CoreMemory.hpp"
#include "Headers/FlatStorage.hpp"
#include "Headers/ThreadMemory.hpp"
#include <iostream>
#include <memory>

using std::cout;
using namespace Octane;

int main(void) {
    CoreAllocator Memory;
    ThreadMemory Data;

    Data.Init(Memory, 128, 128);

    cout << "New Frame : " << BoolStr( Data.LocalFrameNew() ) << '\n';
    u32* Addr1 = (u32*)Data.LocalRequestBytes(4);
    *Addr1 = 0xABCDEF98;
    cout << "From Return " << (void*)Addr1 << " : " << std::hex << (int)*Addr1 << "\n";
    Addr1 = (u32*)Data.LocalGetAtAddress(0);
    cout << "From Access " << (void*)Addr1 << " : " << std::hex << (int)*Addr1 << "\n\n";

    // New Frame

    cout << "New Frame : " << BoolStr( Data.LocalFrameNew() ) << '\n';
    u32* Addr2 = (u32*)Data.LocalRequestBytes(4);
    *Addr2 = 0xCAFEBEEF;
    cout << "From Return " << (void*)Addr2 << " : " << std::hex << (int)*Addr2 << "\n";
    Addr2 = (u32*)Data.LocalGetAtAddress(0);
    cout << "From Access " << (void*)Addr2 << " : " << std::hex << (int)*Addr2 << "\n\n";
    
    // Dropping Frame
    Data.LocalFrameDrop();
    cout << "--- DROPPED ---\n";
    Addr1 = (u32*)Data.LocalGetAtAddress(0);
    cout << "From Access " << (void*)Addr1 << " : " << std::hex << (int)*Addr1 << "\n\n";

    // Dropping Bytes
    Data.LocalDropBytes(2);
    cout << "--- DROPPED BYTES ---\n";
    
    Addr1 = (u32*)Data.LocalGetAtAddress(2);
    // Expecting a segfault here!
    cout << "From Access " << (void*)Addr1 << " : " << std::hex << (int)*Addr1 << "\n\n";

    Data.Free(Memory);

    // int* a = Memory.Request<int>(1, DEFAULT_ALLOC_FLAGS, 42);
    // cout << *a << '\n';
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
