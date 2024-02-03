#include "Headers/Common.hpp"
#include "Headers/CoreMemory.hpp"
#include "Headers/FlatStorage.hpp"
#include "Headers/Instructions.hpp"
#include "Headers/ThreadMemory.hpp"
#include "Headers/Functions.hpp"
#include <iostream>
#include <memory>

using std::cout;
using namespace Octane;

int main(void)
{
    CoreAllocator Memory;
    FlatStorage Storage;
    RelocationTable Reloc;

    Function F;

    /// Initialise Storage
    Storage.Init(Memory);

    /// Set up some dummy Symbols
    StorageRequest Req;

    Req.Type = SymbolType::DATA;
    Req.Key = "KeyA";
    Storage.AssignSymbol(Req);

    Req.Type = SymbolType::DATA;
    Req.Key = "KeyB";
    Storage.AssignSymbol(Req);

    /// Init the Reloc table
    Reloc.Init(Memory, &Storage, 3);
    Reloc.AssignIDX(0, "KeyA");
    Reloc.AssignIDX(1, "KeyB");
    Reloc.AssignIDX(2, "KeyC"); // Doesn't exist!

    /// Test the query!
    cout << Reloc.RetrieveIDXKey(0) << " : " << Reloc.RetrieveIDX(0) << '\n';
    cout << Reloc.RetrieveIDXKey(1) << " : " << Reloc.RetrieveIDX(1) << '\n';
    cout << Reloc.RetrieveIDXKey(2) << " : " << Reloc.RetrieveIDX(2) << '\n';

    return 0;
}
