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

#include "Headers/CoreMemory.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>

using std::cout;

namespace Octane {
    
    /// FUNC: Header Log
    ////////////////////////////////////////
    void AllocationHeader::Log(void) const noexcept {
        cout << "Allocation : " << (void*)this << '\n';
        cout << "    Exposed Address : " << (AllocationHeader*)this+1 << '\n';
        cout << "    Flags.IsFree    : " << BoolStr(Flags.IsFree) << '\n';
        cout << "    Flags.IsConst   : " << BoolStr(Flags.IsConst) << '\n';
        cout << "    Flags.IsSys     : " << BoolStr(Flags.IsSys) << '\n';
        cout << "    Padding Bytes   : " << (int)Padding << '\n';
        cout << "    Usaable Size    : " << Size << '\n';
        cout << "    Total Size      : "
             << Size + sizeof(*this) + Padding << '\n';
        
        MemoryAddress Addr = ( ((byte*)this) + sizeof(*this) );
        switch ( Size ) {
            case 0: {
                cout << "  [SIZE IS 0! DO NOT USE THIS POINTER!]\n";
            break;}

            case sizeof(u8): {
                cout << "    Data [i8: " << (i16)(*Addr.As.i8Ptr) << ", u8: ";
                cout << (u16)(*Addr.As.u8Ptr) << ", char: ";
                cout << (char)(*Addr.As.i8Ptr) << "]\n";
            break;}
            
            case sizeof(u16): {
                cout << "    Data [i16: " << (i16)(*Addr.As.i16Ptr) << ", u16: ";
                cout << (u16)(*Addr.As.u16Ptr) << "]\n";
            break;}

            case sizeof(u32): {
                cout << "    Data [i32: " << (i32)(*Addr.As.i32Ptr) << ", u32: ";
                cout << (u32)(*Addr.As.u32Ptr) << "]\n";
            break;}

            case sizeof(u64): {
                cout << "    Data [i64: " << (i64)(*Addr.As.i64Ptr) << ", u64: ";
                cout << (u64)(*Addr.As.u64Ptr) << "]\n";
            break;}
        }
    }

    /// FUNC: ValidateMemory
    ////////////////////////////////////////
    MemoryError 
    CoreAllocator::ValidateMemory(void) noexcept {
        RAIIMutex Locker(m_AllocLock);
        m_LastError = MEMORY_OK;
        
        // If m_MaxAllocations is 0, do not impose a hard-cap.
        if (m_MaxAllocations && m_TotalAllocations >= m_MaxAllocations)
            m_LastError = MEMORY_HIT_OS_MAXIMUM;
        if (m_TotalAllocations < 0)
            m_LastError = MEMORY_NEGATIVE_MEMORY_USAGE;
        
        return m_LastError;
    }

    /// FUNC: Allocate
    ////////////////////////////////////////
    MemoryAddress 
    CoreAllocator::Allocate(const AddressSizeSpecificer Size, 
                            const AllocFlags Flags) 
    noexcept {
        /// This doesn't check for silly cases such as
        /// Size == 0. These should be checked
        /// when the allocator is called.
        /// Don't waste cycles baby-proofing
        /// what is SUPPOSED to be fast code
        /// ONLY to be used by the internal systems.
        ////////////////////////////////////////
        RAIIMutex Locker(m_AllocLock);
        MemoryAddress Address;
        
        const u8 PaddingBytes = MemoryAddress::ComputePaddingBytes(Size);
        // cout << "DEBUG : PaddingBytes : " << (int)PaddingBytes << '\n';
        
        // If a maximum cap is set
        if (m_MaxAllocations) {
            // Check if in bounds
            if ( m_TotalAllocations + Size + PaddingBytes
                 + sizeof(AllocationHeader) > m_MaxAllocations )
            {
                m_LastError = MEMORY_HIT_VM_MAXIMUM;
                return nullptr;
            }
        }
        // Allocate Block, plus the size of the AllocationHeader
        Address = ::operator new( (Size + sizeof(AllocationHeader)),
                                  std::nothrow );
        if (Address == nullptr) {
            m_LastError = MEMORY_HIT_OS_MAXIMUM;
            return nullptr;
        }
        // If successful, store the metadata and return.
        Address.As._HeaderPtr->Flags   = Flags;
        Address.As._HeaderPtr->Size    = Size;
        Address.As._HeaderPtr->Padding = PaddingBytes;
        // Address.As._HeaderPtr->Log(); // DEBUG
        Address.As.BytePtr += sizeof(AllocationHeader);
        m_TotalAllocations += Size + sizeof(AllocationHeader) + PaddingBytes;
        ////////////////////////////////////////
        /// If QueryAllocatedSize is performed,
        /// it will return the correct size of
        /// the buffer, no matter where the 
        /// pointer has been passed to.
        ////////////////////////////////////////

        return Address;
    }

    /// FUNC: Deallocate
    ////////////////////////////////////////
    void CoreAllocator::Deallocate(MemoryAddress Address) noexcept {
        /// Once again, not checking for numbskulls
        /// supplying nullptrs or invalid addresses
        /// into this function. Use at your own risk.
        /// ALWAYS! CHECK! YOUR! POINTERS!
        ////////////////////////////////////////
        RAIIMutex Locker(m_AllocLock);
        
        AddressSizeSpecificer Size = Address.QueryTotalAllocatedSize();
        ::operator delete(Address.As.BytePtr - sizeof(AllocationHeader));
        m_TotalAllocations -= Size;
    }

    /// FUNC: Reallocate
    ////////////////////////////////////////
    MemoryError 
    CoreAllocator::Reallocate(MemoryAddress& Address, 
                const AddressSizeSpecificer NewSize) noexcept 
    {
        // Allocate a new block to store the data in
        MemoryAddress NewAddress = Allocate(NewSize, 
                                            Address.Header()->Flags);
        if (NewAddress == nullptr)
            return m_LastError;
        
        // Copy the data to the new address
        AddressSizeSpecificer OldSize = Address.QueryAllocatedSize();
        if (NewSize > OldSize)
            memcpy(NewAddress.As.VoidPtr, Address.As.VoidPtr, OldSize);
        else
            memcpy(NewAddress.As.VoidPtr, Address.As.VoidPtr, NewSize);
        
        // Free the old data
        Deallocate(Address);
        // Set the reference to the new address
        Address = NewAddress;
        
        return MEMORY_OK;
    }
}