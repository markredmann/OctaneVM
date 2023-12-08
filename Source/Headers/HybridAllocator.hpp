#///////////////////////////////////////////////////////////////////////////////
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

#ifndef OCTVM_HYBRID_ALLOCATOR_H
#define OCTVM_HYBRID_ALLOCATOR_H 1

#include "CoreMemory.hpp"

/// TODO: This entire system.
////////////////////////////////////////


namespace Octane {

    static constexpr const AllocFlags DEFAULT_HYALLOC_FLAGS = {
        0, // IsFree    
        0, // IsConst   
        0, // IsSys     
        0, // IsNonVital
        1, // IsHyAlloc 
        0, // IsLiAlloc 
    };

    /// @brief This allocator is not yet
    /// implemented, and will instead just
    /// wrap calls to CoreAllocator for the
    /// time being. In future builds, this will
    /// be a pool-based allocator that *SHOULD*
    /// be far faster than direct calls to 
    /// ::operator new() or malloc(), and
    /// is handled using as much static memory
    /// as possible
    ////////////////////////////////////////
    class HybridAllocator {
        private:
            CoreAllocator* m_CoreAlloc;
            // MemoryError    m_LastError;
            // There's nothing here.. yet.
        public:

            OctVM_SternInline
            void AssignCoreAllocator(CoreAllocator* Allocator)
                { m_CoreAlloc = Allocator; }

            OctVM_WarnDiscard
            MemoryAddress Request(const AddressSizeSpecificer Size,
                    const AllocFlags Flags = DEFAULT_HYALLOC_FLAGS) noexcept;
            
            bool          Release(MemoryAddress Address)            noexcept;
            
            OctVM_WarnDiscard
            MemoryError   Resize(MemoryAddress& Address, 
                   const AddressSizeSpecificer  NewSize)            noexcept;
    };

}


#endif /* !OCTVM_HYBRID_ALLOCATOR_H */