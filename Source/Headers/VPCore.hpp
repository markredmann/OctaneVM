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

#ifndef OCTVM_VPCORE_HPP
#define OCTVM_VPCORE_HPP 1

#include "Common.hpp"
#include "CoreStorage.hpp"
#include "ThreadingPrimitives.hpp"
#include "ThreadMemory.hpp"
#include "Instructions.hpp"

namespace Octane {

    // Forward Decl

    class VM;
    class Function;

/// VPCORE:
////////////////////////////////////////
    class VPCore {
        public:
            union Register {
                static constexpr byte COUNT  = 16;
                static constexpr byte UNUSED = 0xFF;
                MemoryAddress AsPtr;
                u64           AsU64;
                u64           AsI64;
                f32           AsF32;
                f64           AsF64;
            };
        private:
            IThread*     m_IThread;
            ThreadMemory m_Memory;
            u16          m_ID;
        public:
            constexpr OctVM_SternInline
            bool IsMainThread(void) const noexcept
                { return ( m_ID == 0 ? true : false ); }
    };

/// EXECSTATE:
////////////////////////////////////////
    struct ExecState {
        VM&              VMInstance;
        Instruction*     IP; 
        VPCore::Register Reg[VPCore::Register::COUNT];
        VPCore&          Thread;
        ThreadMemory&    ThreadMemory;
        CoreAllocator&   Allocator;
        StorageDevice&   Storage;
        Function&        CurrentFunc;
    };

}

#endif /* !OCTVM_VPCORE_HPP */
