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
    /// Messy, but its only for internal
    /// testing anyway so it doesnt matter.
    /// Debug code is debug code.
    ////////////////////////////////////////
    void AllocationHeader::Log(const char* const Prefix) const noexcept {
        cout << Prefix << "Allocation : " << (void*)this << '\n';
        cout << Prefix;
        cout << "    Exposed Address  : " << (AllocationHeader*)this+1 << '\n';
        cout << Prefix;
        cout << "    Flags.IsFree     : " << BoolStr(Flags.IsFree) << '\n';
        cout << Prefix;
        cout << "    Flags.IsConst    : " << BoolStr(Flags.IsConst) << '\n';
        cout << Prefix;
        cout << "    Flags.IsSys      : " << BoolStr(Flags.IsSys) << '\n';
        cout << Prefix;
        cout << "    Flags.IsNonVital : " << BoolStr(Flags.IsNonVital) << '\n';
        cout << Prefix;
        cout << "    Flags.IsHyAlloc  : " << BoolStr(Flags.IsHyAlloc) << '\n';
        cout << Prefix;
        cout << "    Padding Bytes    : " << (int)Padding << '\n';
        cout << Prefix;
        cout << "    Requested Size   : " << Size << '\n';
        cout << Prefix;
        cout << "    Contiguous Size  : " << Size + Padding << '\n';
        cout << Prefix;
        cout << "    Total Size       : "
             << Size + sizeof(*this) + Padding << '\n';
        
        MemoryAddress Addr = ( ((byte*)this) + sizeof(*this) );
        switch ( Size ) {
            case 0: {
                cout << Prefix << "  [SIZE IS 0! DO NOT USE THIS POINTER!]\n";
            break;}

            case sizeof(u8): {
                cout << Prefix << "    Data [i8: " << (i16)(*Addr.As.i8Ptr) 
                     << ", u8: " << (u16)(*Addr.As.u8Ptr) << ", char: "
                     << (char)(*Addr.As.i8Ptr) << "]\n";
            break;}
            
            case sizeof(u16): {
                cout << Prefix << "    Data [i16: " << (i16)(*Addr.As.i16Ptr) 
                     << ", u16: " << (u16)(*Addr.As.u16Ptr) << "]\n";
            break;}

            case sizeof(u32): {
                cout << Prefix << "    Data [i32: " << (i32)(*Addr.As.i32Ptr) 
                     << ", u32: " << (u32)(*Addr.As.u32Ptr) << "]\n";
            break;}

            case sizeof(u64): {
                cout << Prefix << "    Data [i64: " << (i64)(*Addr.As.i64Ptr) 
                     << ", u64: " << (u64)(*Addr.As.u64Ptr) << "]\n";
            break;}
        }
    }

    /// FUNC: ValidateMemory
    ////////////////////////////////////////
    MemoryError 
    CoreAllocator::ValidateMemory(void) noexcept {
        RAIIMutex Locker(m_AllocLock);
        m_LastError = MEMORY_OK;
        
        // -Wsign-compare flipped out here before,
        // so check if our allocations are negative first, then
        // run the comparison.

        if ( m_ObjectAllocations < 0 || m_SystemAllocations < 0 )
        {
            m_LastError = MEMORY_NEGATIVE_MEMORY_USAGE;
            return m_LastError;
        }

        // If m_MaxAllocations is 0, do not impose a hard-cap.
        if ( m_MaxAllocations &&
            (u64)GetTotalAllocations() >= m_MaxAllocations )
        {
            m_LastError = MEMORY_HIT_OS_MAXIMUM;
            return m_LastError;
        }
        
        return MEMORY_OK;
    }

    /// FUNC: Allocate
    ////////////////////////////////////////
    MemoryAddress 
    CoreAllocator::Request(const AddressSizeSpecificer Size, 
                            const AllocFlags Flags) 
    noexcept {
        /// Babyproofing has won over
        ////////////////////////////////////////
        if ( !Size ) 
            { m_LastError = MEMORY_SIZE_IS_ZERO;
              return nullptr; }

        RAIIMutex Locker(m_AllocLock);
        MemoryAddress Address;
        
        const u8 PaddingBytes = MemoryAddress::ComputePaddingBytes(Size);
        
        // If a maximum cap is set
        if (m_MaxAllocations) {
            // Check if in bounds
            if ( GetTotalAllocations() + Size + PaddingBytes
                 + sizeof(AllocationHeader) > m_MaxAllocations )
            { m_LastError = MEMORY_HIT_VM_MAXIMUM;
              return nullptr; }
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
        Address.As.BytePtr += sizeof(AllocationHeader);
        if ( Flags.IsSys )
            m_SystemAllocations += Size + sizeof(AllocationHeader) 
                                        + PaddingBytes;
        else
            m_ObjectAllocations += Size + sizeof(AllocationHeader) 
                                        + PaddingBytes;
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
    void CoreAllocator::Release(MemoryAddress Address) noexcept {
        /// Once again, not checking for numbskulls
        /// supplying nullptrs or invalid addresses
        /// into this function. Use at your own risk.
        /// ALWAYS! CHECK! YOUR! POINTERS!
        ////////////////////////////////////////
        RAIIMutex Locker(m_AllocLock);
        
        AddressSizeSpecificer Size = Address.QueryTotalAllocatedSize();
        if ( Address.Header()->Flags.IsSys )
            m_SystemAllocations -= Size;
        else
            m_ObjectAllocations -= Size;
        
        ::operator delete( (void*)(&Address.As._HeaderPtr[-1]) );
    }

    /// FUNC: Reallocate
    ////////////////////////////////////////
    MemoryError 
    CoreAllocator::Resize(MemoryAddress& Address, 
                const AddressSizeSpecificer NewSize) noexcept 
    {
        // Allocate a new block to store the data in
        MemoryAddress NewAddress = Request(NewSize, 
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
        Release(Address);
        // Set the reference to the new address
        Address = NewAddress;
        
        return MEMORY_OK;
    }
}
