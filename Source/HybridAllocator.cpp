///////////////////////////////////////////////////////////////////////////////
//                           Copyright (c) 2023                              //
//                         Rosetta H&S Integrated                            //
///////////////////////////////////////////////////////////////////////////////
//  Permission is hereby granted, free of charge, to any person obtaining    //
//        a copy of this software and associated documentation files         //
//  (the "Software"), to deal in the Software without restriction, including //
//     without limitation the right to use, copy, modify, merge, publish,    //
//     distribute, sublicense, and/or sell copies of the Software, and to    //
//         permit persons to whom the Software is furnished to do so,        //
//                     subject to the following conditions:                  //
///////////////////////////////////////////////////////////////////////////////
// The above copyright notice and this permission notice shall be included   //
//          in all copies or substantial portions of the Software.           //
///////////////////////////////////////////////////////////////////////////////
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS   //
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                //
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.    //
// IN NO EVENT SHALL THE   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY    //
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT //
// OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  //
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                //
///////////////////////////////////////////////////////////////////////////////

#include "Headers/HybridAllocator.hpp"

namespace Octane {

    /// TODO: This is just directly
    /// calling CoreAllocator methods, as
    /// HybridAllocator isn't implemented yet
    /// and won't be for a while. Programs
    /// should still work just fine.
    ////////////////////////////////////////
    
    /// FUNC: Allocate
    ////////////////////////////////////////
    MemoryAddress 
    HybridAllocator::Request(const AddressSizeSpecificer Size, 
                             const AllocFlags Flags) 
    noexcept {
        return m_CoreAlloc->Request(Size, Flags);
    }

    /// FUNC: Deallocate
    ////////////////////////////////////////
    bool HybridAllocator::Release(MemoryAddress Address) noexcept {
        m_CoreAlloc->Release(Address);
        return true;
    }

    /// FUNC: Reallocate
    ////////////////////////////////////////
    MemoryError 
    HybridAllocator::Resize(MemoryAddress& Address, 
                const AddressSizeSpecificer NewSize) noexcept 
    {
        return m_CoreAlloc->Resize(Address, NewSize);
    }

}