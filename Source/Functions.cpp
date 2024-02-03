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

#define OCTVM_INTERNAL 1

#include "Headers/Functions.hpp"
#include <iostream>

namespace Octane {

/// SECTORRELOC:
////////////////////////////////////////
    /// INIT:
    ////////////////////////////////////////
    MemoryError RelocationTable::Init(CoreAllocator& Allocator,
                                      StorageDevice* Device, u32 Count)
    noexcept
    {
        m_Storage  = Device;
        m_ArrayLen = Count;
        m_Array    = Allocator.Request<Entry>(Count, SYSTEM_ALLOC_FLAGS);

        if ( !m_Array )
            return Allocator.GetLastError();

        return MEMORY_OK;
    }

    /// FREE:
    ////////////////////////////////////////
    void RelocationTable::Free(CoreAllocator& Allocator) noexcept
    {
        Allocator.Release(m_Array);
    }

    /// ASSIGNIDX:
    ////////////////////////////////////////
    bool RelocationTable::AssignIDX(u32 IDX, const char* Key, bool Resolve)
    noexcept
    {
        // Sanity check
        if ( IDX > m_ArrayLen || !Key )
            return false;
        
        Entry& Slot = m_Array[IDX];
        // If already set, return false
        if ( Slot.Key )
            return false;

        Slot.Key = Key;
        if ( Resolve && m_Storage )
            Slot.ResolvedSymbol = m_Storage->LookupSymbol(Key);
        else
            Slot.ResolvedSymbol = nullptr;

        
        return true;
    }

    /// RETRIEVEIDX:
    ////////////////////////////////////////
    Symbol* RelocationTable::RetrieveIDX(u32 IDX) noexcept
    {
        // Sanity Check
        if ( IDX > m_ArrayLen || !m_Storage )
            return nullptr;
        
        Entry& Slot = m_Array[IDX];

        // If already resolved, just return that
        if ( Slot.ResolvedSymbol )
            return Slot.ResolvedSymbol;
        
        // Perform a lookup and store result
        Slot.ResolvedSymbol = m_Storage->LookupSymbol(Slot.Key);

        // Note: This can be null in the event the Key doesn't refer to
        // any defined Symbol
        return Slot.ResolvedSymbol;
    }

    /// RETRIEVEIDXKEY:
    ////////////////////////////////////////
    const char* RelocationTable::RetrieveIDXKey(u32 IDX) noexcept
    {
        // Sanity Check
        if ( IDX > m_ArrayLen )
            return nullptr;
        
        return m_Array[IDX].Key;
    }
    
                                      

/// FUNCTION:
////////////////////////////////////////

    //////////////// NOTE: /////////////////
    /// The `Function` class only acts as a
    /// wrapper around executable code, and
    /// thus has very little logic of its own
    ////////////////////////////////////////
    

    /// INITEXPOSED:
    ////////////////////////////////////////
    void Function::InitExposed(ExposedFunc CFunc) noexcept
    {
        m_InstructionCount = 0;
        m_SharedSize       = 0;
        m_SharedPadding    = 0;
        m_SharedOffset     = 0;
        m_IsVMFunc         = false;
        m_FirstRun         = true;
        m_Raw.CFunc        = CFunc; 
    }

    /// INIT:
    ////////////////////////////////////////
    MemoryError Function::Init(CoreAllocator& Allocator,
                               RelocationTable* Reloc,
                               u16 INSCount, u16 SharedSize) noexcept
    {
        /// In order to protect the VM from any corrupted instructions,
        /// there are a few bytes of padding between the end of the
        /// Code Space (VM bytecode) and Shared Space (static shared
        /// function-local memory) which is also aligned for any arbitrary
        /// data.
        ///
        /// The layout of the memory space is as follows:
        /// [CODE ... ...][PADDING][SHARED ... ...]
        /// The padding bytes are also made up of the opcode for returning
        /// in order to force the executor to halt before running into
        /// user memory space.
        ///
        /// A minimum padding size of 4 is required, although it is typically
        /// 4 or 8 bytes.
        int Padding =
        (
            BASE_PADDING_BYTES +
            MemoryAddress::ComputePaddingBytes
                ( (sizeof(Instruction) * INSCount) + BASE_PADDING_BYTES )
        );
        /// This is where the Shared Space will begin
        int Offset = ( (sizeof(Instruction) * INSCount) + Padding );

        /// Perform the aggregate allocation combining 
        /// both Code Space and Shared Space.
        ///
        /// Alongside this, set all bytes to the `ret` opcode
        /// as per reasons stated above.
        ////////////////////////////////////////
        m_Raw.VMBytes = Allocator.Request<byte>(
            (sizeof(Instruction) * INSCount) + Padding + SharedSize,
            DEFAULT_ALLOC_FLAGS,
            (byte)Instruction::ret
        );
        if ( !m_Raw.VMBytes ) // Failed!
            return Allocator.GetLastError();
        
        /// Store all the other variables
        m_InstructionCount = INSCount;
        m_SharedSize       = SharedSize;
        m_SharedPadding    = Padding;
        m_SharedOffset     = Offset;
        m_IsVMFunc         = true;
        m_FirstRun         = true;

        return MEMORY_OK;
    }

    /// FREE:
    ////////////////////////////////////////
    void Function::Free(CoreAllocator& Allocator) noexcept
    {
        /// Better performance this way. TODO: WTF
        Allocator.Release(MemoryAddress(m_Raw.VMBytes));
    }



}