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
    
    /// In OctaneVM, all memory allocations
    /// store a 32bit integer denoting the
    /// allocated size of the buffer at
    /// the start of the given memory address
    /// subtracted by 64bits (8 bytes), as
    /// there is other metadata stored
    /// alongside it. See Octane::AllocationHeader.
    /// For allocations larger than 4GiB,
    /// you will have to split it between
    /// two (or more) allocations.
    /// Frankly, there is little—if any—reason
    /// to ever allocate 4GiB in contiguous
    /// address space, virtual or not.
    ////////////////////////////////////////
    using AddressSizeSpecificer = u32;
    
    enum SizeLiterals {
        KiB = 1024,
        MiB = 1024 * 1024,
        GiB = 1024 * 1024 * 1024,
    };

    constexpr static inline u64 ToKiB(const u64 Count) noexcept
        { return Count * SizeLiterals::KiB; }

    constexpr static inline u64 ToMiB(const u64 Count) noexcept
        { return Count * SizeLiterals::MiB; }

    constexpr static inline u64 ToGiB(const u64 Count) noexcept
        { return Count * SizeLiterals::GiB; }

    /// @brief A bitfield containing metadata
    /// on how to treat an Allocation done
    /// by either CoreAlocator or Hybrid.
    ////////////////////////////////////////
    struct AllocFlags {
            // Has this Address been freed?
        bool  IsFree     : 1; 
            // Is this Address marked const? [unenforced]
        bool  IsConst    : 1; 
            // Is this Address allocated by the System?
        bool  IsSys      : 1; 
            // Is this Address not vital for the System?
        bool  IsNonVital : 1; 
            // Was this Address allocated via Hybrid Allocator?
        bool  IsHyAlloc  : 1; 
            // Was this Address allocated via Linear Allocator?
        bool  IsLiAlloc  : 1; 
    } OctVM_SternPack;

    static constexpr const AllocFlags DEFAULT_ALLOC_FLAGS = {
        0, // IsFree    
        0, // IsConst   
        0, // IsSys     
        0, // IsNonVital
        0, // IsHyAlloc 
        0, // IsLiAlloc 
    };
    
    /// @brief A struct containing metadata
    /// regarding an Allocation returned
    /// by any of the OctaneVM Allocators.
    ////////////////////////////////////////
    struct AllocationHeader { 
            /// The Size of the Allocation.
        AddressSizeSpecificer Size;    
            /// Amount of padding bytes.
        u16                   Padding; 
            /// Metadata Flags.
        AllocFlags            Flags;   

        /// @brief Logs the metadata to std::cout
        ////////////////////////////////////////
        void Log(const char* const Prefix = "") const noexcept;
    };

    /// @brief An address to a block of
    /// memory that is allocated by
    /// an Allocator, such as CoreAllocator
    /// or HybridAllocator.
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
            /// On top of this, we might need to lock
            /// this to 8 bytes again. alignof is
            /// not taking into account that DWORDs
            /// might be stored in contiguous space here.
            ////////////////////////////////////////
            
            /// @brief Computes the number of padding
            /// bytes needed to append in order to
            /// have any subsequent contiguous allocations
            /// aligned by alignof(AllocationHeader).
            /// @param AllocationSize The size of the given
            /// Allocation.
            /// @return A number between 0-8 dictating
            /// how many bytes are required to be appended.
            ////////////////////////////////////////
            constexpr static OctVM_SternInline 
            u8 ComputePaddingBytes(u32 AllocationSize) noexcept
                { return (alignof(void*)-AllocationSize) 
                        % alignof(void*); }
                // { return (alignof(AllocationHeader)-AllocationSize) 
                //         % alignof(AllocationHeader); }

            /// @brief Prints metadata regarding this
            /// Allocation to std::cout.
            ////////////////////////////////////////
            OctVM_SternInline void Log(const char* const Prefix = "") const noexcept
                { (As._HeaderPtr - 1)->Log(Prefix); }

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
                { return (As._HeaderPtr[-1]).Size; }

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
                { return (As._HeaderPtr[-1]).Size
                    + (As._HeaderPtr[-1]).Padding; }

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
                { return (   (As._HeaderPtr[-1]).Size 
                           + (As._HeaderPtr[-1]).Padding
                           + sizeof(AllocationHeader) ); }

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
        MEMORY_HIT_OS_MAXIMUM,
        // The attempted Allocation was deemed too large to be accommodated
        // by this Allocator. 
        // In CoreAllocator: This would be an allocation greater than 4GiB
        // (2**32 or 4 * 1024*1024*1024)
        // In HybridAllocator: this would be either greater than 4KiB
        // (2 * 1024*1024) or it does not fit in the memory space,
        // and must be backed instead by CoreAllocator.
        MEMORY_SIZE_TOO_LARGE,
        // The attempted Allocation has no size, and thus
        // cannot be completed.
        MEMORY_SIZE_IS_ZERO
    };

    /// @brief The Core Allocator for
    /// OctaneVM. Allocations are thread-locked
    /// and uses ::operator new(std::nothrow)
    /// under the hood. Only one instance 
    /// of this Allocator per VM.
    ////////////////////////////////////////
    class CoreAllocator {
        private:
            /// The total number of Bytes allocated by this Allocator
            /// to be used for Program or Storage-mapped Object memory.
            /// This number is signed as to be able to detect if
            /// more deallocations have been done than allocations,
            /// which would indicate that this Allocator is being used
            /// to free memory which does not belong to it.
            i64         m_ObjectAllocations = 0;
            /// The total number of Bytes allocated by this Allocator
            /// to be used for internal VM implementation memory.
            /// This number is signed as to be able to detect if
            /// more deallocations have been done than allocations,
            /// which would indicate that this Allocator is being used
            /// to free memory which does not belong to it.
            i64         m_SystemAllocations = 0;
            /// The maximum amount of bytes that this Allocator
            /// is allowed to allocate. Typically Controlled by 
            /// the VM instance, and can usually change even at
            /// runtime. Leave 0 for no VM-imposed hard-cap.
            u64         m_MaxAllocations   = 0;
            /// The result of the last allocation error.
            /// Does not reset on a good allocation. Instead, clear
            /// manually by using ClearLastError().
            MemoryError m_LastError        = MEMORY_OK;
            /// A Mutex which will lock this Allocator when:
            ///  1: Allocating Memory
            ///  2: Reallocating Memory
            ///  3: Deallocating Memory
            ///  4: Validating Memory
            Mutex       m_AllocLock;
        public:
            constexpr static const u64 MAX_ALLOC_SIZE = ToGiB(4);
            
            /// @brief Validates the Memory of this Allocator.
            /// Effectively just ensures that the internal 
            /// memory is not at or beyond the maximum
            /// capacity
            /// @return The result of the validation.
            /// Review MemoryError for more details.
            ////////////////////////////////////////
            MemoryError   ValidateMemory(void)                       noexcept;
            
            ////////////////////////////////////////
            /// @brief Requests a block of memory
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
            OctVM_WarnDiscard
            MemoryAddress Request(const AddressSizeSpecificer Size,
                        const AllocFlags Flags = DEFAULT_ALLOC_FLAGS) noexcept;

            ////////////////////////////////////////
            /// @brief Requests an array of Objects of a 
            /// given Type (or a single instance if
            /// Count is not specified) and calls its
            /// default constructor.
            /// @param Count The amount of Objects of the
            /// given Type to allocate. Ensure that th
            /// @param Flags A list of flags that will tell
            /// OctaneVM how to handle this memory internally.
            /// @return A MemoryAddress pointing to
            /// the block if the Allocation was
            /// successful. Otherwise returns nullptr.
            /// To see what caused the error, use
            /// GetLastError().
            ////////////////////////////////////////
            template <typename Type, typename ... Args> OctVM_WarnDiscard
            Type* Request(const u32 Count = 1, const AllocFlags
                                    Flags = DEFAULT_ALLOC_FLAGS,
                            Args... Params) noexcept
                {
                    // Sanity check our inputs first
                    if ( !Count )
                        { return nullptr; }
                    if ( sizeof(Type) * Count > MAX_ALLOC_SIZE )
                        { m_LastError = MEMORY_SIZE_TOO_LARGE;
                          return nullptr; }

                    // Do the actual raw memory request
                    MemoryAddress Address = Request(sizeof(Type)*Count, Flags);
                    if ( Address ) {
                        Type* AutoCast = Address.Cast<Type>();
                        
                        // Call constructor(s)
                        for( u32 i = 0; i < Count; i++ ) 
                            ::new(AutoCast + i) Type(Params...);
                        
                        return AutoCast;
                    }

                    return nullptr;
                }

            /// @brief Deallocates the given MemoryAddress
            /// @param Address The MemoryAddress to free.
            /// WARNING: ONLY PROVIDE A MEMORYADDRESS
            /// THAT WAS SUPPLIED BY THIS ALLOCATOR INSTANCE!
            ////////////////////////////////////////
            void          Release(MemoryAddress Address)          noexcept;

            /// @brief Deallocates the given Object(s), and
            /// calls their destructors.
            /// @param Address The Object(s) to free.
            /// WARNING: ONLY PROVIDE AN OBJECT THAT
            /// WAS SUPPLIED BY THIS ALLOCATOR INSTANCE!
            ////////////////////////////////////////
            template <typename Type>
            void          Release(Type* Address)                  noexcept
                {
                    MemoryAddress OriginalAddr = MemoryAddress(Address);

                    /// NOTE: This could cause serious problems
                    /// if the wrong type is passed...
                    ////////////////////////////////////////
                    u32 Count = OriginalAddr.QueryAllocatedSize() 
                              / sizeof(Type);

                    for (u32 i = 0; i < Count; i++ )
                        Address[i].~Type();

                    Release(OriginalAddr);
                }


            ////////////////////////////////////////
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
            OctVM_WarnDiscard
            MemoryError   Resize(MemoryAddress& Address, 
                   const AddressSizeSpecificer  NewSize)          noexcept;
            
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
                { return m_SystemAllocations + m_ObjectAllocations; }

            /// @brief Returns the maximum amount of
            /// bytes that this CoreAllocator can
            /// allocate.
            /// @return The count in bytes. If the
            /// result is 0, then this CoreAllocator
            /// does not impose a hard-cap on the
            /// number of allocations.
            ////////////////////////////////////////
            constexpr OctVM_SternInline 
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
