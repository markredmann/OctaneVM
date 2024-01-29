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

#ifndef OCTVM_THREADMEMORY_HPP
#define OCTVM_THREADMEMORY_HPP 1

#include "CoreMemory.hpp"

namespace Octane {

    /// @brief An encapsulation of and handler for
    /// the LocalSpace and Stack of a specific `IThread`
    ////////////////////////////////////////
    class ThreadMemory {
        private:
            //////////////// TODO: /////////////////
            /// Add Function trace information
            ////////////////////////////////////////
            
            /// @brief A small struct containing
            /// metadata about a given call frame
            ////////////////////////////////////////
            struct Frame {
                u16    Offset;
                u16    Usage;
                Frame* LastFrame;
            };

            /// The size in bytes allocated for the Stack.
            /// Note that in the OctaneVM, the Stack is NOT where
            /// Local variables are stored. Instead, the Stack is
            /// for transferring extra parameters through to Functions
            /// and/or for temporary storage between Function calls.
            u16    m_StackSize;
            /// The current pointer offset of the Stack.
            u16    m_StackIDX;
            /// The size in bytes allocated for the Local Address Space.
            /// This is used for storing local variables and is kept
            /// private to each Function call.
            u32    m_LocalSize;
            /// The current pointer offset of the Local Address Space.
            u32    m_LocalIDX;
            /// The combined address space, containing both the Stack 
            /// and Local Address Space.
            byte*  m_RawSpace;
            /// The current Local Frame which all Local operations will
            /// operate relative to.
            Frame* m_CurrentLocalFrame;
        public:
        /// MANAGEMENT:
        ////////////////////////////////////////

            /// @brief Initialise and allocate
            /// thread-local memory buffers
            /// @param Allocator The `CoreAllocator`
            /// owned by this VM instance
            /// @param StackSize The size of the Stack
            /// Note that local variables are NOT stored
            /// here. The Stack is for transfer between
            /// function calls, not for variable storage.
            /// @param LocalSize The size of the preallocated
            /// Local address space. Each function call
            /// recieves its own address space that is
            /// sectioned off from this buffer
            /// @return A `MemoryError` indicating the
            /// result of the internal allocation from
            /// the `CoreAllocator` supplied in Allocator
            ////////////////////////////////////////
            MemoryError Init(CoreAllocator& Allocator, 
                             u16 StackSize, u32 LocalSize) noexcept;

            /// @brief Deallocates internal memory
            /// to prepare for VM shutdown
            /// @param Allocator The `CoreAllocator`
            /// owned by this VM instance. Ensure that
            /// this is the same allocator that was used
            /// to initialise this thread's memory buffers
            /// @return 
            ////////////////////////////////////////
            void        Free(CoreAllocator& Allocator)     noexcept;

            /// @brief Logs memory metadata to the console
            ////////////////////////////////////////
            void        Log(void) const noexcept;

        /// STACK:
        ////////////////////////////////////////
            static constexpr const i32 MERGE_FAILURE = INT_MIN; 
            
            /// @brief Pushes an 8-bit value to the Stack
            /// @param Data The value to push
            /// @return If the Stack has enough space
            /// to accommodate the data, a positive
            /// value denoting the amount of available
            /// bytes on the Stack will be returned.
            ///
            /// If pushing the data would result in
            /// overrunning the allocated Stack space,
            /// a negative value denoting how many bytes
            /// beyond the maximum Stack size this push
            /// operation would have used is returned,
            /// and the push operation is never performed.
            /// Free space on the Stack and try again.
            ////////////////////////////////////////
            i32 StackPush8   (u8  Data)                      noexcept;
            /// @brief Pushes a 16-bit value to the Stack
            /// @param Data The value to push
            /// @return If the Stack has enough space
            /// to accommodate the data, a positive
            /// value denoting the amount of available
            /// bytes on the Stack will be returned.
            ///
            /// If pushing the data would result in
            /// overrunning the allocated Stack space,
            /// a negative value denoting how many bytes
            /// beyond the maximum Stack size this push
            /// operation would have used is returned,
            /// and the push operation is never performed.
            /// Free space on the Stack and try again.
            ////////////////////////////////////////
            i32 StackPush16  (u16 Data)                      noexcept;
            /// @brief Pushes a 32-bit value to the Stack
            /// @param Data The value to push
            /// @return If the Stack has enough space
            /// to accommodate the data, a positive
            /// value denoting the amount of available
            /// bytes on the Stack will be returned.
            ///
            /// If pushing the data would result in
            /// overrunning the allocated Stack space,
            /// a negative value denoting how many bytes
            /// beyond the maximum Stack size this push
            /// operation would have used is returned,
            /// and the push operation is never performed.
            /// Free space on the Stack and try again.
            ////////////////////////////////////////
            i32 StackPush32  (u32 Data)                      noexcept;
            /// @brief Pushes a 64-bit value to the Stack
            /// @param Data The value to push
            /// @return If the Stack has enough space
            /// to accommodate the data, a positive
            /// value denoting the amount of available
            /// bytes on the Stack will be returned.
            /// If pushing the data would result in
            /// overrunning the allocated Stack space,
            /// a negative value denoting how many bytes
            /// beyond the maximum Stack size this push
            /// operation would have used is returned,
            /// and the push operation is never performed.
            /// Free space on the Stack and try again.
            ////////////////////////////////////////
            i32 StackPush64  (u64 Data)                      noexcept;
            
            /// @brief Pushes an arbitrary amount of
            /// memory to the Stack
            /// @param Data The starting address of
            /// the meory to push
            /// @param Size The amount of bytes to push
            /// @return If the Stack has enough space
            /// to accommodate the data, a positive
            /// value denoting the amount of available
            /// bytes on the Stack will be returned.
            ///
            /// If pushing the data would result in
            /// overrunning the allocated Stack space,
            /// a negative value denoting how many bytes
            /// beyond the maximum Stack size this push
            /// operation would have used is returned,
            /// and the push operation is never performed.
            /// Free space on the Stack and try again.
            ////////////////////////////////////////
            i32 StackPushMem (void* Data, u16 Size)          noexcept;
            /// @brief Pops off N-bytes from `Other`'s Stack
            /// and stores them in this Stack
            /// @param Other The Stack to merge with the current
            /// @param Size The amount of bytes to pop off the
            /// other Stack and push into the current
            /// @return If the Stack has enough space
            /// to accommodate the data, a positive
            /// value denoting the amount of available
            /// bytes on the Stack will be returned.
            ///
            /// If pushing the data would result in
            /// overrunning the allocated Stack space,
            /// a negative value denoting how many bytes
            /// beyond the maximum Stack size this push
            /// operation would have used is returned,
            /// and the push operation is never performed.
            /// Free space on the Stack and try again.
            ///
            /// If, however, the pop operation were to
            /// fail on `Other`'s Stack, the specific value
            /// `ThreadMemory::MERGE_FAILURE` is returned
            /// to denote this special case.
            ////////////////////////////////////////
            i32 StackMerge   (ThreadMemory& Other, u16 Size) noexcept;

            /// @brief An optional containing the
            /// result of a Pop operation
            ////////////////////////////////////////
            struct PopOpt {
                /// The value stored on the Stack
                u64  Value;
                /// Was the Pop successful?
                bool Valid;
            };

            /// @brief Pops off an 8-bit value from the Stack
            /// @return An optional containing two fields,
            /// with `PopOpt::Valid` denoting whether or
            /// not the Pop could be performed.
            ///
            /// If Valid is true, then `PopOpt::Value` contains
            /// the data retrieved from the Stack.
            ///
            /// If Valid is false, the operation failed and
            /// `PopOpt::Value` contains how many bytes
            /// the pop operation would have underflowed
            ////////////////////////////////////////
            PopOpt StackPop8   (void) noexcept;
            /// @brief Pops off a 16-bit value from the Stack
            /// @return An optional containing two fields,
            /// with `PopOpt::Valid` denoting whether or
            /// not the Pop could be performed.
            ///
            /// If Valid is true, then `PopOpt::Value` contains
            /// the data retrieved from the Stack.
            ///
            /// If Valid is false, the operation failed and
            /// `PopOpt::Value` contains how many bytes
            /// the pop operation would have underflowed
            ////////////////////////////////////////
            PopOpt StackPop16  (void) noexcept;
            /// @brief Pops off a 32-bit value from the Stack
            /// @return An optional containing two fields,
            /// with `PopOpt::Valid` denoting whether or
            /// not the Pop could be performed.
            ///
            /// If Valid is true, then `PopOpt::Value` contains
            /// the data retrieved from the Stack.
            ///
            /// If Valid is false, the operation failed and
            /// `PopOpt::Value` contains how many bytes
            /// the pop operation would have underflowed
            ////////////////////////////////////////
            PopOpt StackPop32  (void) noexcept;
            /// @brief Pops off a 64-bit value from the Stack
            /// @return An optional containing two fields,
            /// with `PopOpt::Valid` denoting whether or
            /// not the Pop could be performed.
            ///
            /// If Valid is true, then `PopOpt::Value` contains
            /// the data retrieved from the Stack.
            ///
            /// If Valid is false, the operation failed and
            /// `PopOpt::Value` contains how many bytes
            /// the pop operation would have underflowed
            ////////////////////////////////////////
            PopOpt StackPop64  (void) noexcept;
            /// @brief Pops off an arbitrary amount of memory
            /// from the Stack
            /// @param Out A pointer to the storage address
            /// @param Size The amount of bytes to pop
            /// @return An optional containing two fields,
            /// with `PopOpt::Valid` denoting whether or
            /// not the Pop could be performed.
            ///
            /// If Valid is true, then `Out` contains the
            /// memory copied out from the Stack
            ///
            /// If Valid is false, the operation failed and
            /// `PopOpt::Value` contains how many bytes
            /// the pop operation would have underflowed
            ////////////////////////////////////////
            PopOpt StackPopMem (byte* Out, u16 Size) noexcept;


            /// @return True if the Stack is valid
            /// and initialised
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            bool StackValid(void) const noexcept
                { return ( m_RawSpace && m_StackSize > 0 ); }

        /// LOCAL:
        ////////////////////////////////////////
            /// @brief Creates a new Local Frame,
            /// and sets the current Frame to the newly
            /// allocated Frame. All subsequent 
            /// Local operations will act upon this Frame
            /// @return True if the Frame was created,
            /// otherwise False on failure due to
            /// insufficient memory in the Local Space
            ////////////////////////////////////////
            bool LocalFrameNew      (void) noexcept;
            /// @brief Drops the current Local Frame,
            /// freeing all Local allocations from it,
            /// and setting the current Frame to the
            /// previous Frame. All subsequent Local
            /// operations will act upon the last Frame
            /// @return True if a Frame could be dropped
            /// and another frame is remaining.
            /// 
            /// If this Frame was the last one, or
            /// there is no Frame currently set,
            /// this will return false.
            ////////////////////////////////////////
            bool LocalFrameDrop     (void) noexcept;
            /// @brief Frees all Local allocations
            /// made in this Frame, but retains
            /// everything else.
            /// @return True if there is a Frame
            /// to reset, otherwise false.
            ////////////////////////////////////////
            bool LocalFrameReset    (void) noexcept;

            /// @brief Requests N-bytes from the Local
            /// Frame's Address Space
            /// @param Size The amount of bytes to allocate
            /// @return On success, a pointer to the newly
            /// allocated memory. If the allocation would
            /// overflow the Local space, or no Frame is
            /// currently defined, a `nullptr` is returned.
            ////////////////////////////////////////
            byte*  LocalRequestBytes (u16 Size)   noexcept;
            /// @brief Releases N-bytes from the Local
            /// Frame's Address Space
            /// @param Size The amount of bytes to free
            /// @return The amount of allocated bytes still
            /// remaining in the current Frame address space.
            /// If the attempted deallocation is larger than the
            /// amount of allocated bytes, the operation will
            /// fail and instead return a negative value denoting
            /// how many bytes this deallocation would have underflowed
            ////////////////////////////////////////
            i32    LocalDropBytes    (u16 Size)   noexcept;
            /// @brief Converts a Local address relative
            /// to the current Frame to a Global pointer
            /// @param Offset The offset from the Frame
            /// Address Space
            /// @return Returns a converted pointer in Global
            /// space. If, however, the offset is beyond
            /// the reserved Frame Address Space, or no
            /// Frame is currently defined, a `nullptr` is returned.
            ////////////////////////////////////////
            byte*  LocalGetAtAddress (u16 Offset) noexcept;

            /// @return True if the Local Space is valid
            /// and initialised for use, otherwise false.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            bool LocalValid(void) const noexcept
                { return ( m_RawSpace && m_LocalSize > 0 
                           && m_CurrentLocalFrame ); }
        /// CLEARING:
        ////////////////////////////////////////
            /// @brief Resets the Stack and clears
            /// all pushed data
            ////////////////////////////////////////
            OctVM_SternInline
            void ResetStack(void) noexcept
                { m_StackIDX = 0; }
            
            /// @brief Resets the Local Space and
            /// clears all data and Frames
            ////////////////////////////////////////
            OctVM_SternInline
            void ResetLocal(void) noexcept
                { m_LocalIDX = 0; m_CurrentLocalFrame = 0; }
        /// GETTERS:
        ////////////////////////////////////////
            /// @return Returns the total size in bytes
            /// of the combined Stack and Local
            /// preallocated space
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u32 GetAllocationSize(void) const noexcept
                { return m_StackSize + m_LocalSize; }
            
            /// @return Returns the number of free
            /// bytes left on the Stack
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetStackRemaining(void) const noexcept
                { return m_StackSize - m_StackIDX; }

            /// @return Returns the number of free
            /// bytes left in the Local Space
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetLocalRemaining(void) const noexcept
                { return m_LocalSize - m_LocalIDX; }

            /// @return Returns the total size in bytes
            /// of the Stack's reserved allocation
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetStackSize(void) const noexcept
                { return m_StackSize; }

            /// @return Returns the total size in bytes
            /// of the Local Space's reserved allocation
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetLocalSize(void) const noexcept
                { return m_LocalSize; }

            /// @return Returns the amount of bytes
            /// currently being used by the Stack.
            ///
            /// Also functions as an index/offset into
            /// the reserved Stack space where the
            /// last pushed byte is stored
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetStackUsage(void) const noexcept
                { return m_StackIDX; }
            
            /// @return Returns the amount of bytes
            /// currently being used in the Local Space.
            ///
            /// Also functions as an index/offset into
            /// the reserved Local Space where the
            /// last requested byte is stored
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetLocalUsage(void) const noexcept
                { return m_LocalIDX; }

            /// @return Returns a pointer denoting
            /// the start of the Stack allocation
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            byte* GetStackStart(void) const noexcept
                { return m_RawSpace; }
            
            /// @return Returns a pointer denoting
            /// the start of the Local Space allocation
            ///
            /// Note that this does not return the
            /// start of the current Frame. For this,
            /// use `ThreadMemory::LocalGetAtAddress`
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            byte* GetLocalStart(void) const noexcept
                { return m_RawSpace + m_StackSize; }
    };

}

#endif /* !OCTVM_THREADMEMORY_HPP */
