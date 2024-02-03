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

#ifndef OCTVM_FUNCTIONS_HPP
#define OCTVM_FUNCTIONS_HPP 1

#include "Common.hpp"
#include "CoreStorage.hpp"
#include "VPCore.hpp"
#include "Exceptions.hpp"

//////////////// NOTE: /////////////////
/// @markredmann :
/// Blame VS Code Insider's shitty
/// doxygen implementation for these
/// god-awful comments. Half of them
/// don't even render right in 1.87.0
////////////////////////////////////////
/// Times Microsoft has screwed me over:
/// 18,446,744,073,709,551,616,4,294,
/// 967,296,281,474,976,710,656,256 + 1
////////////////////////////////////////

namespace Octane {

/// RELOCTABLE:
////////////////////////////////////////

    /// @brief A `Symbol` table for handling
    /// external(global) access in `Function`
    /// instances
    ////////////////////////////////////////
    class RelocationTable {
        private:
            /// @brief An internal structure
            /// defining a symbol relocation
            ////////////////////////////////////////
            struct Entry {
                /// The `Symbol` if/when resolved from the key
                Symbol*     ResolvedSymbol = nullptr;
                /// The key used to perform the lookup at runtime
                const char* Key            = nullptr;
            };

            /// The `StorageDevice` to perform runtime lookups
            /// on relocation access            
            StorageDevice* m_Storage       = nullptr;
            /// An internal array containing all relocations
            Entry*         m_Array         = nullptr;
            /// The amount of entries in the internal array
            u32            m_ArrayLen      = 0;
        public:
            /// @brief Initialises the internal table
            /// for `Symbol` assignment
            /// @param Allocator The VM's `CoreAllocator`
            /// @param Device The `StorageDevice` to perform
            /// runtime lookups from on relocation access
            /// @param Count The amount of entries to be relocated
            /// @return `MEMORY_OK` on success, otherwise returns a
            /// `MemoryError` denoting why the Allocator failed
            ////////////////////////////////////////
            MemoryError Init(CoreAllocator& Allocator, StorageDevice* Device,
                             u32 Count)                 noexcept;
            /// @brief Deallocates all internal memory
            /// @param Allocator The VM's `CoreAllocator`
            ////////////////////////////////////////
            void        Free(CoreAllocator& Allocator)  noexcept;


            OctVM_SternInline
            /// @brief Assigns the internal `StorageDevice`
            /// which will perform runtime lookups on
            /// relocation access
            /// @param Device A pointer to the VM's `StorageDevice`.
            /// Ensure that the Device is valid and not NULL, otherwise
            /// serious complications may occur.
            ////////////////////////////////////////
            void AssignDevice(StorageDevice* Device) noexcept
                { m_Storage = Device; }
            
            /// @brief Assigns an index in the internal table to
            /// a `Symbol` key that will be resolved upon retrieval
            /// @param IDX The index to store the key in
            /// @param Key The key that corresponds to a given `Symbol`
            /// in the stored `StorageDevice`
            /// @param Resolve Should the `Symbol` be resolved immediately?
            /// If false, the lookup will occur when the key is first
            /// retrieved
            /// @return Returns true if the index does not correspond
            /// to an already existing entry. Otherwise false
            ////////////////////////////////////////
            bool        AssignIDX(u32 IDX, const char* Key,
                                  bool Resolve = false)     noexcept;
            /// @brief Retrieves a `Symbol` from a static index
            /// @param IDX The index into the internal table to
            /// perform a one-time `Symbol` lookup
            /// @return If the index is valid and contains a key
            /// that correctly points to a `Symbol` in the stored
            /// `StorageDevice`, a one-time lookup is performed
            /// and the `Symbol` is returned. Further calls will
            /// immediately return the resolved `Symbol` at no cost.
            ///
            /// If the index is invalid or contains a malformed or
            /// incorrect key, a nullptr is returned
            ////////////////////////////////////////  
            Symbol*     RetrieveIDX(u32 IDX)                noexcept;
            /// @brief Retrieves the key stored at the given index
            /// @param IDX The index into the internal table to
            /// retrieve the stored key for `Symbol` lookup.
            /// @return If the index is valid, this returns the
            /// stored key. Otherwise returns nullptr.
            ////////////////////////////////////////
            const char* RetrieveIDXKey(u32 IDX)             noexcept;
    };

/// FUNCTION:
////////////////////////////////////////

    /// @brief A function pointer to a native
    /// C++ function that can be passed to a 
    /// VM `Function` instance to be executed
    /// alongside emulated instructions
    ////////////////////////////////////////
    using ExposedFunc = Exception::HandlerResult(*)(ExecState&);

    /// @brief An encapsulation of an executable
    /// routine that can be evaluated by the VM
    ////////////////////////////////////////
    class Function {
        private:
            /// The amount of `Instruction`s inside the
            /// bytecode space. Note that double-width and
            /// triple-width instructions have immediates
            /// thtat count towards this value.
            u16  m_InstructionCount = 0;
            /// The size in bytes of the Shared address space
            u16  m_SharedSize       = 0;
            /// The amount of padding between the end of
            /// the bytecode space and the shared space.
            /// 
            /// The internal structure of the raw bytes:
            /// [CODE ... ...][PADDING][SHARED ... ...]
            u8   m_SharedPadding    = 0;
            /// The offset in bytes to where the Shared
            /// address space begins from the start of the raw
            /// aggregate byte array.
            u32  m_SharedOffset     = 0;
            /// Is this Function a VM function comprised of bytecode?
            /// If false, this Function points to a native C++ function
            /// exposed to the VM for faster, non-portable execution.
            bool m_IsVMFunc         = false;
            /// If true, this Function has not been ran by the VM yet,
            /// and *should* be tested and validated prior to execution.
            bool m_FirstRun         = true;
            
            /// A pointer to the `RelocationTable` to lookup all
            /// encoded relocatable indicies stored in `call`,
            /// `spawn`, `spawnanon`, and `eload` instructions.
            RelocationTable* m_RelocTable = nullptr;
            union {
                /// If `m_IsVMFunc` is true, this is set to an
                /// aggregate byte array containing both bytecode
                /// and the Shared address space.
                byte*       VMBytes;
                /// If `m_IsVMFunc` is false, this is set to a native
                /// C++ function exposed to the VM.
                ExposedFunc CFunc;
            } m_Raw = {nullptr};
        public:
            /// @brief
            /// The OctaneVM specification requires that:
            ///
            /// **A:** All VM-bytecode routines must contain at least one
            ///    additional `ret` instruction or at least 4 `ret` opcodes
            ///    appended to the end of the code in the event of an
            ///    executor overrun.
            ///
            /// **B:** The Shared Address Space begins at an address 
            ///    divisible by 8, or otherwise pointer-aligned.
            ///
            /// This OctaneVM implementation contains both the executable
            /// bytecode and the Shared Address Space within the same 
            /// allocation end-to-end. As such, the padding between Code
            /// Space and Shared Space must be at least 4 bytes (1 INS-Width)
            /// and all padding bytes are to be set to the `Instruction::ret`
            /// opcode to ensure safe handling of an executor overrun.
            ///
            /// The internal structure of the aggregate allocation is:
            /// `[CODE ... ...][PADDING][SHARED ... ...]`
            ///
            /// Note that in the event this results in an non pointer-aligned
            /// starting address of the Shared space, additional bytes may
            /// be required. For the full count of padding bytes used in any
            /// given `Function`, please see `GetPaddingBytes()`
            static constexpr int BASE_PADDING_BYTES = 4;

        /// INITIALISATION:
        ////////////////////////////////////////

            /// @brief Initialises this VM Function as
            /// a native exposed C++ function so it can
            /// be called inside of bytecode routines.
            /// @param CFunc A function pointer to the
            /// native C++ function
            ////////////////////////////////////////
            void        InitExposed(ExposedFunc CFunc) noexcept;
            
            /// @brief Initialises the memory for both Code Space
            /// and Shared Address Space for this VM Function.
            /// Note that the Code Space **MUST** be populated externally.
            /// @param Allocator The VM's `CoreAllocator`
            /// @param Reloc A pointer to the `RelocationTable` used
            /// by the bytecode if applicable. Leave as null if the
            /// Function does not call upon or otherwise depend on
            /// external data or routines.
            /// @param INSCount The amount of `Instruction`s in the
            /// Function's bytecode.
            /// @param SharedSize The size, in bytes, of the private
            /// Shared Address Space of this Function.
            /// @return On success, returns `MEMORY_OK`.
            /// Otherwise returns an `Octane::MemoryError` denoting why the
            /// Allocator failed to request sufficient memory.
            ////////////////////////////////////////
            MemoryError Init(CoreAllocator& Allocator, RelocationTable* Reloc,
                             u16 INSCount, u16 SharedSize) noexcept;

            /// @brief Deallocates the memory of both the Code Space
            /// and Shared Address Space for this VM Function.
            /// @param Allocator The VM's `CoreAllocator`
            ////////////////////////////////////////
            void        Free(CoreAllocator& Allocator) noexcept;
        
        /// QUERY:
        ////////////////////////////////////////

            constexpr OctVM_SternInline
            /// @brief Returns true if internal memory
            /// has been allocated.
            /// Note that this does not indicate
            /// whether or not this Function contains
            /// valid or initialised `Instruction`s
            ////////////////////////////////////////
            bool IsSet(void) const noexcept
                { return ( m_Raw.VMBytes ? true : false ); }
            
            constexpr OctVM_SternInline
            /// @brief Returns true if this Function
            /// is comprised of VM-bytecode meant to
            /// be ran in a portable executor.
            /// Otherwise returns false, indicating
            /// that this Function points to a 
            /// native, exposed C++ function
            ////////////////////////////////////////
            bool IsVMFunc(void) const noexcept
                { return m_IsVMFunc; }
            
            constexpr OctVM_SternInline
            /// @brief Returns true if this Function
            /// is a native C++ function exposed to
            /// the VM for use in bytecode routines
            ////////////////////////////////////////
            bool IsCFunc(void) const noexcept
                { return !m_IsVMFunc; }

            constexpr OctVM_SternInline
            /// @brief Returns true if this Function
            /// has not been ran by the VM yet.
            /// If true, the Function *should* be
            /// tested and validated prior to execution.
            ////////////////////////////////////////
            bool IsFirstRun(void) const noexcept
                { return m_FirstRun; }
        
            OctVM_SternInline
            /// @brief Denotes that this Function
            /// has been executed (and likely tested
            /// and validated) by the VM's executor
            /// at least once.
            ////////////////////////////////////////
            void MarkUsed(void) noexcept
                { m_FirstRun = false; }

        /// GETTERS:
        /// --- PISSED OFF NOTE --- @markredmann
        /// Hey, see how *THESE* functions have their
        /// doxygen headers ABOVE the constexpr line?
        /// They render just fine in tooltips! But the ones
        /// above? NOoOoOoO! WE REFUSE TO WORK!!! Harumph.
        ////////////////////////////////////////

            /// @brief Returns a pointer to the
            /// `RelocationTable` used by this Function
            /// if applicable.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            RelocationTable* GetRelocTable(void) const noexcept
                { return m_RelocTable; }

            /// @brief If this Function contains a
            /// native, C++ function, this will return
            /// a pointer to the `ExposedFunc`. Otherwise
            /// returns nullptr.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            ExposedFunc GetCFunc(void) const noexcept
                {
                    if ( !IsSet() || m_IsVMFunc )
                        return nullptr;
                    return m_Raw.CFunc;
                }
            
            /// @brief If this Function contains VM
            /// bytecode, ths will return a pointer
            /// to the Code Space (array of `Instruction`)
            /// Otherwise returns nullptr.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            Instruction* GetCodeSpace(void) const noexcept
                {
                    if ( !IsSet() || !m_IsVMFunc || !m_InstructionCount )
                        return nullptr;
                    return (Instruction*)m_Raw.VMBytes;
                }
            
            /// @brief If this Function contains VM
            /// bytecode, ths will return a pointer
            /// to the private Shared Address Space.
            /// Otherwise returns nullptr.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            byte* GetSharedSpace(void) const noexcept
                {
                    if ( !IsSet() || !m_IsVMFunc || !m_SharedSize )
                        return nullptr;
                    return m_Raw.VMBytes + m_SharedOffset;
                }

            /// @brief If this Function contains VM
            /// bytecode, ths will return the count of
            /// `Instruction`s comprising the Code Space.
            /// Note that double-width and
            /// triple-width instructions have immediates
            /// thtat count towards this value.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetInstructionCount(void) const noexcept
                { return m_InstructionCount; }
            
            /// @brief If this Function contains VM
            /// bytecode, ths will return the count of
            /// bytes allocated for the Shared
            /// Address Space if applicable.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u16 GetSharedSize(void) const noexcept
                { return m_SharedSize; }
            
            /// @brief If this Function contains VM
            /// bytecode, ths will return the count of
            /// padding bytes between the Code Space
            /// and the Shared Address Space.
            ///////////////////////////////////////
            constexpr OctVM_SternInline
            u8 GetPaddingBytes(void) const noexcept
                { return m_SharedPadding; }
        

    };

}

#endif /* !OCTVM_FUNCTIONS_HPP */
