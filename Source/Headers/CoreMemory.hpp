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


#ifndef OCTVM_MEMORY_HPP
#define OCTVM_MEMORY_HPP 1

#include "Common.hpp"
#include "ThreadingPrimitives.hpp"

namespace Octane {
    
    // In OctaneVM, all memory allocations
    // store a 32bit integer denoting the
    // allocated size of the buffer at
    // the start of the given memory address
    // subtracted by 32bits (4 bytes).
    // For allocations larger than 4GiB,
    // you will have to split it between
    // two (or more) allocations.
    // Frankly, there is little to no reason
    // why you should ever need to allocate
    // 4GiB in contiguous (or even virtual)
    // address space
    ////////////////////////////////////////
    using AddressSizeSpecificer = u32;
    
    /// @brief A bitfield containing metadata
    /// on how to treat an Allocation done
    /// by either CoreAlocator or Hybrid.
    ////////////////////////////////////////
    struct AllocFlags {
        bool  IsFree     : 1; // Has this Address been freed?
        bool  IsConst    : 1; // Is this Address marked const? [unenforced]
        bool  IsSys      : 1; // Is this Address allocated by the System?
        bool  IsNonVital : 1; // Is this Address not vital for the System?
        bool  IsHyAlloc  : 1; // Was this Address allocated via Hybrid?
    } OctVM_SternPack;

    static constexpr const AllocFlags DEFAULT_ALLOC_FLAGS = {
        .IsFree     = 0,
        .IsConst    = 0,
        .IsSys      = 0,
        .IsNonVital = 0,
        .IsHyAlloc  = 0,
    };
    
    /// @brief A struct containing metadata
    /// regarding an Allocation returned
    /// by any of the OctaneVM Allocators.
    ////////////////////////////////////////
    struct AllocationHeader {
        AddressSizeSpecificer Size;    // The Size of the Allocation.
        u8                    Padding; // Amount of padding bytes from 0-8.
        AllocFlags            Flags;   // Metadata Flags.

        /// @brief Logs the metadata to std::cout
        ////////////////////////////////////////
        void Log(void) const noexcept;
    };

    /// @brief An address to a block of
    /// memory that is allocated by
    /// a MemoryManager instance.
    ////////////////////////////////////////
    class MemoryAddress {
        public:
        /// Members
        ////////////////////////////////////////
            union _addr_union {
                void* VoidPtr;
                byte* BytePtr;
                u8*   u8Ptr;   // Same as using As.BytePtr
                u16*  u16Ptr; 
                u32*  u32Ptr; 
                u64*  u64Ptr; 
                i8*   i8Ptr;   // Same as using As.BytePtr
                i16*  i16Ptr; 
                i32*  i32Ptr; 
                i64*  i64Ptr; 
                // Only to be used in internals //
                AllocationHeader* _HeaderPtr;
            } As;
        /// Constructors
        ////////////////////////////////////////
            OctVM_SternInline MemoryAddress(void) noexcept = default;
            OctVM_SternInline MemoryAddress(void* const Ptr) noexcept
                : As{Ptr} {}

        /// Methods
        ////////////////////////////////////////

            //////////////// FIXME: ////////////////
            /// There is probably a faster way to do this
            ////////////////////////////////////////
            
            /// @brief Computes the number of padding
            /// bytes needed to append in order to
            /// have any subsequent contiguous allocations
            /// aligned by alignof(AllocationHeader).
            /// @param AllocationSize The size of the given
            /// Allocation..
            /// @return A number between 0-8 dictating
            /// how many bytes are required to be appended.
            ////////////////////////////////////////
            constexpr static OctVM_SternInline 
            u8 ComputePaddingBytes(u32 AllocationSize) noexcept
                { return (alignof(AllocationHeader)-AllocationSize) 
                        % alignof(AllocationHeader); }

            /// @brief Prints metadata regarding this
            /// Allocation to std::cout.
            ////////////////////////////////////////
            OctVM_SternInline void Log(void) const noexcept
                { (As._HeaderPtr - 1)->Log(); }

            /// @brief Casts the Address into a 
            /// pointer of the given templated type.
            /// @return Returns a pointer to the
            /// given templated type.
            ////////////////////////////////////////
            template<typename Type> OctVM_SternInline
            Type* Cast(void) noexcept
                { return (Type*)As.VoidPtr; }

            /// @brief Determine whether the Address
            /// is defined
            /// @return Returns true if the Address
            /// is defined, otherwise false.
            /// Note that this does NOT mean the
            /// Address is safe to use. It only means
            /// that it is not NULL.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            operator bool(void) const noexcept
                { return (As.VoidPtr ? true : false); }

            /// @brief Sets the Address
            /// @param Ptr The Address to store.
            /// WARNING: ONLY use this if you are
            /// 100% certain the given pointer
            /// was allocated by a MemoryManager instance!
            /// @return A reference to the current
            /// MemoryAddress instance.
            ////////////////////////////////////////
            OctVM_SternInline
            MemoryAddress& operator =(void* const Ptr) noexcept
                { this->As.VoidPtr = Ptr; return *this; }
            
            /// @brief Compares this Address to
            /// an arbitrary Pointer
            /// @param Ptr The Pointer to compare.
            /// @return True if the Addresses are
            /// equal, otherwise false.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            bool operator ==(void* const Ptr) const noexcept
                { return (this->As.VoidPtr == Ptr); }

            /// @brief Compares this Address to
            /// another MemoryAddress
            /// @param Ptr The Address to compare.
            /// @return True if the Addresses are
            /// equal, otherwise false.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            bool operator ==(const MemoryAddress& Other) const noexcept
                { return (this->As.VoidPtr == Other.As.VoidPtr); }

            /// @brief Queries the allocated size of
            /// this buffer.
            /// @return An integer containing
            /// the number of bytes that were passed
            /// into the Allocator used
            /// to generate this object. Useful for
            /// managing arrays.
            /// Note that this does not take into
            /// account the size of the header itself, or
            /// the amount of padding bytes appended to
            /// the Allocation. For this, use
            /// QueryTotalAllocatedSize() to include
            /// the total, or QueryContiguousSize()
            /// to include just the padding size.
            ////////////////////////////////////////
            constexpr OctVM_SternInline 
            AddressSizeSpecificer QueryAllocatedSize(void) const noexcept
            {
                return (As._HeaderPtr[-1]).Size;
            }

            /// @brief Queries the allocated size of
            /// this buffer including the amount of
            /// padding bytes appended to the end of
            /// this Allocation.
            /// @return An integer containing the
            /// size of the buffer plus the paddings.
            /// Note that this does NOT include the
            /// AllocationHeader. For this, use
            /// QueryTotalAllocatedSize().
            ////////////////////////////////////////
            constexpr OctVM_SternInline 
            AddressSizeSpecificer QueryContiguousSize(void) const noexcept
            {
                return (As._HeaderPtr[-1]).Size
                       + (As._HeaderPtr[-1]).Padding;
            }

            /// @brief Queries the allocated size of
            /// this buffer including the size of its header
            /// and the amount of padding bytes.
            /// @return An integer containing
            /// the total number of bytes that were
            /// passed into the Allocator, including
            /// the AllocationHeader itself and
            /// any trailing padding bytes.
            ////////////////////////////////////////
            constexpr OctVM_SternInline 
            AddressSizeSpecificer QueryTotalAllocatedSize(void) const noexcept
            {
                return (
                    (As._HeaderPtr[-1]).Size +(As._HeaderPtr[-1]).Padding
                    + sizeof(AllocationHeader) );
            }

            OctVM_SternInline AllocationHeader* Header(void) noexcept
                { return (As._HeaderPtr - 1); }

            /// @brief Returns a reference of the
            /// templated Type object at the given
            /// index.
            /// @param Index The Index to retrieve
            /// from. Note that this index is aligned
            /// to the size of the type, which means
            /// that its analogous to using array
            /// notation on an array of the templated Type,
            /// with each step in the index being 
            /// offset by sizeof(Type).
            /// @return A reference to the Object
            /// at the given index.
            ////////////////////////////////////////
            template<typename Type> OctVM_SternInline
            Type& GetAtIndexAligned(const AddressSizeSpecificer Index) noexcept
                { return (Type&) (*(As.BytePtr + (Index * sizeof(Type)))); }
            
            /// @brief Returns a reference of the
            /// templated Type object at the given
            /// index.
            /// @param Index The Index to retrieve
            /// from. Note that this index is direct,
            /// which means that its analogous to 
            /// (Type&) (*(Address + Index))
            /// @return A reference to the Object
            /// at the given index.
            ////////////////////////////////////////
            template<typename Type> OctVM_SternInline
            Type& GetAtIndexDirect(const AddressSizeSpecificer Index) noexcept
                { return (Type&) (*(As.BytePtr + Index)); }

    };

    /// @brief An enumeration of all potential
    /// errors when using any Allocator.
    ////////////////////////////////////////
    enum MemoryError : u8 {
        // Memory is valid.
        MEMORY_OK,
        // The amount of total deallocations is
        // greater than the amount of total allocations.
        // This would mean that this Allocator is
        // deallocating memory that was not allocated by
        // this Allocator.
        MEMORY_NEGATIVE_MEMORY_USAGE,
        // This Allocator cannot allocate any more memory
        // because the amount of total allocations is already
        // at the amount of this Allocator's maximum allocations,
        // or the previous attempted allocation 
        // would have surpassed the maximum.
        // Either deallocate unused memory, or increase the maximum
        // amount by using SetMaxAllocations() on the VM's CoreAllocator.
        MEMORY_HIT_VM_MAXIMUM,
        // This Allocator cannot allocate any more memory
        // because the underlying operating system is out
        // of memory to allocate, be it virtual or physical.
        // Either deallocate unused memory or terminate execution.
        MEMORY_HIT_OS_MAXIMUM
    };

    /// @brief The Core Allocator for
    /// OctaneVM. Allocations are thread-locked
    /// and uses ::operator new(std::nothrow)
    /// under the hood. Only one instance 
    /// of this Allocator per VM.
    ////////////////////////////////////////
    class CoreAllocator {
        private:
            // The total number of Bytes allocated by this Allocator.
            // This number is signed as to be able to detect if
            // more deallocations have been done than allocations,
            // which would indicate that this Allocator is being used
            // to free memory which does not belong to it.
            i64         m_TotalAllocations = 0;
            // The maximum amount of bytes that this Allocator
            // is allowed to allocate. Typically Controlled by 
            // the VM instance, and can usually change even at
            // runtime. Leave 0 for no VM-imposed hard-cap.
            u64         m_MaxAllocations   = 0;
            // The result of the last allocation error.
            // Does not reset on a good allocation. Instead, clear
            // manually by using ClearLastError().
            MemoryError m_LastError        = MEMORY_OK;
            // A Mutex which will lock this Allocator when:
            //  1: Allocating Memory
            //  2: Reallocating Memory
            //  3: Deallocating Memory
            //  4: Validating Memory
            Mutex       m_AllocLock;
        public:
            /// @brief Validates the Memory of this Allocator.
            /// Effectively just ensures that the internal 
            /// memory is not at or beyond the maximum
            /// capacity
            /// @return The result of the validation.
            /// Review MemoryError for more details.
            ////////////////////////////////////////
            MemoryError   ValidateMemory(void)                       noexcept;
            
            ////////////////////////////////////////
            OctVM_WarnDiscard("The returned Address MUST be managed manually!")
            /// @brief Allocates a block of memory
            /// by using direct calls to the OS.
            /// @param Size The Size of the Allocation.
            /// Please note that OctaneVM does not support 
            /// contiguous Allocations larger than 4GiB
            /// Also note that this method also does
            /// not check if Size is 0. Passing 0
            /// will return a "valid" MemoryAddress
            /// but further usage will be undefined behaviour.
            /// @param Flags A list of flags that will tell
            /// OctaneVM how to handle this memory internally.
            /// @return A MemoryAddress pointing to
            /// the block if the Allocation was
            /// successful. Otherwise returns nullptr.
            /// To see what caused the error, use
            /// GetLastError().
            ////////////////////////////////////////
            MemoryAddress Allocate(const AddressSizeSpecificer Size,
                        const AllocFlags Flags = DEFAULT_ALLOC_FLAGS) noexcept;
            
            /// @brief Deallocates the given MemoryAddress
            /// @param Address The MemoryAddress to free.
            /// WARNING: ONLY PROVIDE A MEMORYADDRESS
            /// THAT WAS SUPPLIED BY THIS ALLOCATOR INSTANCE!
            ////////////////////////////////////////
            void          Deallocate(MemoryAddress Address)          noexcept;
            
            ////////////////////////////////////////
            OctVM_WarnDiscard("This could potentially fail!")
            /// @brief Reallocates this block of memory
            /// to a new size and copies over the data.
            /// @param Address A Reference to a MemoryAddress
            /// Object. The Reference will be updated to
            /// point to the new block.
            /// @param NewSize The size of the new block.
            /// @return MEMORY_OK if the reallocation
            /// was successful, otherwise will return
            /// the appropriate error code.
            ////////////////////////////////////////
            MemoryError   Reallocate(MemoryAddress& Address, 
                       const AddressSizeSpecificer  NewSize)         noexcept;
            
            /// @brief Returns the last error thrown
            /// by this Allocator. Note that this
            /// is not reset when a successful 
            /// Allocation occurs. To ensure a reset,
            /// call the ClearLastError() method.
            /// @return The last error code thrown.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            MemoryError GetLastError(void) const noexcept
                { return m_LastError; }
            
            /// @brief Clears the last error thrown by
            /// this Allocator, and sets it to MEMORY_OK.
            ////////////////////////////////////////
            OctVM_SternInline
            void ClearLastError(void) noexcept
                { m_LastError = MEMORY_OK; }
            
            /// @brief Returns the total count in bytes
            /// of all active Allocations from this
            /// CoreAllocator.
            /// @return The count in bytes. If the
            /// result is a negative value, there
            /// was a severe error that occured.
            /// See MemoryError for more details.
            ////////////////////////////////////////
            constexpr OctVM_SternInline 
            i64 GetTotalAllocations(void) const noexcept
                { return m_TotalAllocations; }
            constexpr OctVM_SternInline 

            /// @brief Returns the maximum amount of
            /// bytes that this CoreAllocator can
            /// allocate.
            /// @return The count in bytes. If the
            /// result is 0, then this CoreAllocator
            /// does not impose a hard-cap on the
            /// number of allocations.
            ////////////////////////////////////////
            u64 GetMaxAllocations(void) const noexcept
                { return m_MaxAllocations; }

            /// @brief Sets the maximum amount of
            /// Allocations that this CoreAllocator
            /// can allocate.
            /// @param NewMax The amount in bytes
            /// that this CoreAllocator can allocate.
            /// If 0, there will be no imposed hard-cap.
            ////////////////////////////////////////
            OctVM_SternInline 
            void SetMaxAllocations(const u64 NewMax) noexcept
                { m_MaxAllocations = NewMax; }
    };

}

#endif /* !OCTVM_MEMORY_HPP */