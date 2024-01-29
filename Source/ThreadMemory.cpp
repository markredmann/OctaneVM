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

#include <iostream>
#include "Headers/ThreadMemory.hpp"

namespace Octane {

/// MANAGEMENT:
////////////////////////////////////////

    /// INIT:
    ////////////////////////////////////////
    MemoryError ThreadMemory::Init(CoreAllocator& Memory,
                                   u16 StackSize, u32 LocalSize) noexcept
    {
        // Sanity check the allocation
        if ( StackSize + LocalSize > CoreAllocator::MAX_ALLOC_SIZE )
            return MEMORY_SIZE_TOO_LARGE;
        // Both buffers exist end-to-end in the same allocation
        m_RawSpace = Memory.Request(StackSize + LocalSize, SYSTEM_ALLOC_FLAGS)
                           .As.BytePtr;
        if ( !m_RawSpace )
            return Memory.GetLastError();
        
        m_StackSize         = StackSize;
        m_StackIDX          = 0;
        m_LocalSize         = LocalSize;
        m_LocalIDX          = 0;
        m_CurrentLocalFrame = nullptr;

        return MEMORY_OK;
    }
    
    /// FREE:
    ////////////////////////////////////////
    void ThreadMemory::Free(CoreAllocator& Memory) noexcept
    {
        Memory.Release(m_RawSpace);
        m_StackSize         = 0;
        m_LocalSize         = 0;
        m_StackIDX          = 0;
        m_LocalIDX          = 0;
        m_RawSpace          = nullptr;
        m_CurrentLocalFrame = nullptr;

    }

    /// LOG:
    ////////////////////////////////////////
    void ThreadMemory::Log(void) const noexcept
    {
        using std::cout;

        cout << "ThreadData : "     << (void*)this  << '\n';
        cout << "    StackSize  : " << m_StackSize  << '\n';
        cout << "    LocalSize  : " << m_LocalSize  << '\n';
        cout << "    StackUsage : " << m_StackIDX   << '\n';
        cout << "    LocalUsage : " << m_LocalIDX   << '\n';
        cout << "    RawAddress : " << (void*)m_RawSpace   << '\n';
        cout << "    ------------\n";
        cout << "    Total Allocation Size : " << GetAllocationSize() << '\n';
        cout << "    Remaining Stack Space : " << GetStackRemaining() << '\n';
        cout << "    Remaining Local Space : " << GetLocalRemaining() << '\n';
    }

/// STACK:
////////////////////////////////////////

    /// PUSH:
    ////////////////////////////////////////
    i32 ThreadMemory::StackPush8(u8 Data) noexcept
    {
        // Sanity check. If a push that will overrun the
        // Stack is attempted, return how many bytes would have
        // ran over the buffer (as a negative value)
        if ( m_StackIDX + sizeof(Data) > m_StackSize )
            return ( m_StackSize - (m_StackIDX + sizeof(Data)) );
        
        *( GetStackStart() + m_StackIDX ) = Data;
        m_StackIDX += sizeof(Data);
        // Return the remaining space on the Stack
        return m_StackSize - m_StackIDX;
    }

    i32 ThreadMemory::StackPush16(u16 Data) noexcept
    {
        // Sanity check. If a push that will overrun the
        // Stack is attempted, return how many bytes would have
        // ran over the buffer (as a negative value)
        if ( m_StackIDX + sizeof(Data) > m_StackSize )
            return ( m_StackSize - (m_StackIDX + sizeof(Data)) );
        
        *(u16*)( GetStackStart() + m_StackIDX ) = Data;
        m_StackIDX += sizeof(Data);
        // Return the remaining space on the Stack
        return m_StackSize - m_StackIDX;
    }

    i32 ThreadMemory::StackPush32(u32 Data) noexcept
    {
        // Sanity check. If a push that will overrun the
        // Stack is attempted, return how many bytes would have
        // ran over the buffer (as a negative value)
        if ( m_StackIDX + sizeof(Data) > m_StackSize )
            return ( m_StackSize - (m_StackIDX + sizeof(Data)) );
        
        *(u32*)( GetStackStart() + m_StackIDX ) = Data;
        m_StackIDX += sizeof(Data);
        // Return the remaining space on the Stack
        return m_StackSize - m_StackIDX;
    }

    i32 ThreadMemory::StackPush64(u64 Data) noexcept
    {
        // Sanity check. If a push that will overrun the
        // Stack is attempted, return how many bytes would have
        // ran over the buffer (as a negative value)
        if ( m_StackIDX + sizeof(Data) > m_StackSize )
            return ( m_StackSize - (m_StackIDX + sizeof(Data)) );
        
        *(u64*)( GetStackStart() + m_StackIDX ) = Data;
        m_StackIDX += sizeof(Data);
        // Return the remaining space on the Stack
        return m_StackSize - m_StackIDX;
    }

    i32 ThreadMemory::StackPushMem(void* Data, u16 Size) noexcept
    {
        // Sanity check. If a push that will overrun the
        // Stack is attempted, return how many bytes would have
        // ran over the buffer (as a negative value)
        if ( m_StackIDX + Size > m_StackSize )
            return ( m_StackSize - (m_StackIDX + Size) );

        QuickCopy(Data, GetStackStart() + m_StackIDX, Size);
        m_StackIDX += Size;
        // Return the remaining space on the Stack
        return m_StackSize - m_StackIDX;
    }

    i32 ThreadMemory::StackMerge(ThreadMemory& Other, u16 Size) noexcept
    {
        // Sanity check. If a merge that will overrun the
        // Stack is attempted, return how many bytes would have
        // ran over the buffer (as a negative value)
        if ( m_StackIDX + Size > m_StackSize )
            return ( m_StackSize - (m_StackIDX + Size) );
        
        byte* Addr = GetStackStart() + m_StackIDX;
        PopOpt MergeResult = Other.StackPopMem(Addr, Size);

        // If the pop fails, return a special value
        if ( !MergeResult.Valid )
            return MERGE_FAILURE;
        
        m_StackIDX += Size;
        // Return the remaining space on the Stack
        return m_StackSize - m_StackIDX;
    }

    /// POP:
    ////////////////////////////////////////
    
    ThreadMemory::PopOpt ThreadMemory::StackPop8(void) noexcept
    {
        /// If the Pop will overrun the buffer (backwards)
        /// return an empty struct with `Valid` set to false
        if ( (i32)m_StackIDX - sizeof(u8) < 0 )
            return { -((i32)m_StackIDX - sizeof(u8)), false };

        m_StackIDX -= sizeof(u8);
        return { *( GetStackStart() + m_StackIDX), true };
    }
    
    ThreadMemory::PopOpt ThreadMemory::StackPop16(void) noexcept
    {
        /// If the Pop will overrun the buffer (backwards)
        /// return an empty struct with `Valid` set to false
        if ( (i32)m_StackIDX - sizeof(u16) < 0 )
            return { -((i32)m_StackIDX - sizeof(u16)), false };

        m_StackIDX -= sizeof(u16);
        return { *(u16*)( GetStackStart() + m_StackIDX), true };
    }

    ThreadMemory::PopOpt ThreadMemory::StackPop32(void) noexcept
    {
        /// If the Pop will overrun the buffer (backwards)
        /// return an empty struct with `Valid` set to false
        if ( (i32)m_StackIDX - sizeof(u32) < 0 )
            return { -((i32)m_StackIDX - sizeof(u32)), false };

        m_StackIDX -= sizeof(u32);
        return { *(u32*)( GetStackStart() + m_StackIDX), true };
    }

    ThreadMemory::PopOpt ThreadMemory::StackPop64(void) noexcept
    {
        /// If the Pop will overrun the buffer (backwards)
        /// return an empty struct with `Valid` set to false
        if ( (i32)m_StackIDX - sizeof(u64) < 0 )
            return { -((i32)m_StackIDX - sizeof(u64)), false };

        m_StackIDX -= sizeof(u64);
        return { *(u64*)( GetStackStart() + m_StackIDX), true };
    }

    ThreadMemory::PopOpt ThreadMemory::StackPopMem(byte* Out, u16 Size)
    noexcept {
        /// If the Pop will overrun the buffer (backwards)
        /// return an empty struct with `Valid` set to false
        if ( (i32)m_StackIDX - Size < 0 )
            return { (u64) (-((i64)m_StackIDX - (Size))), false };
        
        m_StackIDX -= Size;
        
        if ( Out ) 
            QuickCopy(GetStackStart() + m_StackIDX, Out, Size);
        
        /// `Value` field is unset but `Valid` is still important
        return { 0, true };
    }

/// LOCAL:
////////////////////////////////////////

    /// LOCALFRAMENEW:
    ////////////////////////////////////////
    bool ThreadMemory::LocalFrameNew(void) noexcept
    {
        // Sanity checks
        if ( (i32)GetLocalRemaining() - sizeof(Frame) < 0 )
            return false;
        
        // Create new Frame
        Frame* LocalFrame = (Frame*)(GetLocalStart() + m_LocalIDX);
        LocalFrame->Offset    = m_LocalIDX;
        LocalFrame->Usage     = 0;
        LocalFrame->LastFrame = m_CurrentLocalFrame;

        // Assign to the current Frame
        m_CurrentLocalFrame = LocalFrame;

        m_LocalIDX += sizeof(Frame);
        return true;
    }

    /// LOCALFRAMEDROP:
    ////////////////////////////////////////
    bool ThreadMemory::LocalFrameDrop(void) noexcept
    {
        // Sanity checks
        if ( !m_CurrentLocalFrame )
            return false;
        
        // Reset our address index to the starting position
        // of the current Frame itself (NOT the exposed data)
        m_LocalIDX = m_CurrentLocalFrame->Offset;
        // Seek back to the last Frame
        m_CurrentLocalFrame = m_CurrentLocalFrame->LastFrame;

        return ( m_CurrentLocalFrame ? true : false );
    }

    /// LOCALFRAMERESET:
    ////////////////////////////////////////
    bool ThreadMemory::LocalFrameReset(void) noexcept
    {
        // Sanity checks
        if ( !m_CurrentLocalFrame )
            return false;

        // Reset the address index to the starting position of
        // the current Frame's exposed data
        m_LocalIDX = m_CurrentLocalFrame->Offset + sizeof(Frame);
        m_CurrentLocalFrame->Usage = 0;

        return true;
    }

    /// LOCALREQUESTBYTES:
    ////////////////////////////////////////
    byte* ThreadMemory::LocalRequestBytes(u16 Size) noexcept
    {
        // Sanity checks
        if ( !m_CurrentLocalFrame )
            return nullptr;
        
        // Ensure our allocation fits within the space
        if ( (i32)GetLocalRemaining() - Size < 0 )
            return nullptr;
        
        
        byte* Ptr = GetLocalStart() + m_LocalIDX;

        m_LocalIDX += Size;
        m_CurrentLocalFrame->Usage += Size;

        return Ptr;
    }

    /// LOCALDROPBTYES:
    ////////////////////////////////////////
    i32 ThreadMemory::LocalDropBytes(u16 Size) noexcept
    {
        // Sanity checks
        if ( !m_CurrentLocalFrame )
            return -1;
        
        // How many bytes are left allocated in our Frame?
        i32 Remaining = (i32)m_CurrentLocalFrame->Usage - Size;

        // If this deallocation won't deallocate behind our Frame,
        // follow through and decrement our address offset
        if ( Remaining >= 0 ) {
            m_LocalIDX -= Size;
            m_CurrentLocalFrame->Usage -= Size;
        }

        return Remaining;
    }

    /// LOCALGETATADDRESS:
    ////////////////////////////////////////
    byte* ThreadMemory::LocalGetAtAddress(u16 Offset) noexcept
    {
        // Sanity checks
        if ( !m_CurrentLocalFrame || Offset >= m_CurrentLocalFrame->Usage )
            return nullptr;
        
        // Return the offsetted address relative to the current Frame data
        return ( GetLocalStart() + m_CurrentLocalFrame->Offset 
                 + sizeof(Frame) + Offset );
    }

}