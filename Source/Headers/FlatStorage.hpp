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

#ifndef OCTVM_FLATSTORGE_HPP
#define OCTVM_FLATSTORGE_HPP 1

#include "CoreMemory.hpp"
#include "CoreStorage.hpp"

namespace Octane {

    /// @brief The Default implementation of
    /// `StorageDevice`. A simple HashMap 
    /// which contains `Symbol`s at the top most
    /// level, with no collections/directories.
    ////////////////////////////////////////
    class FlatStorage final : public StorageDevice {
        private:
            /// @brief Internal extension of base
            /// `Symbol` type. Not to be returned directly!
            ////////////////////////////////////////
            struct FSSymbol : public Symbol {
                // All fields marked Public because this is
                // only accessible to FlatStorage and is
                // fully managed by FlatStorage.
                public:
                    /// The maximum length for a Key is 255
                    /// including the null terminator.
                    constexpr static const int MAX_KEY_SIZE = 0xfe;
                    /// A copy of the string used as this `Symbol`'s Key
                    char*     Key;
                    /// The length in bytes of the Key, minus the null byte
                    u8        KeySize;
                    /// The SDBM hash of the Key
                    u64       KeyHash;
                    /// In the event of a hash collision, the collided
                    /// `Symbol` entry will be stored in this linked list.
                    FSSymbol* CollisonNext;
            };

            /// The default amount of slots for this HashMap.
            constexpr static int MAP_BASESIZE = 32;
            /// When the HashMap begins to run out of slots,
            /// it will grow the array by this amount.
            constexpr static int MAP_STEPSIZE = 16;
            /// A pointer to the VM's Allocator
            CoreAllocator* m_Allocator = nullptr;
            /// The internal HashMap itself
            FSSymbol**     m_Map       = nullptr;
            /// The estimated usage of the Map
            u32            m_MapUsage  = 0;
            /// The real amount of slots currently allocated
            u32            m_MapSize   = 0;

            static bool AssignToIDX(FSSymbol** Map, u32 MapSize,
                                    FSSymbol* Sym) noexcept;
            
            /// @brief Initialises the internal HashMap
            /// @return True on successful allocation,
            /// false on OOM.
            ////////////////////////////////////////
            bool InitMap(void) noexcept;
            /// @brief Regreows the internal HashMap
            /// by `FlatStorage::MAP_STEPSIZE`
            /// @return True on successful allocation,
            /// false on OOM.
            ////////////////////////////////////////
            bool GrowMap(void) noexcept;
            /// @brief Deletes all stored `Symbol`s and
            /// the internal HashMap itself
            ////////////////////////////////////////
            void FreeMap(void) noexcept;
        public:
            /// @brief Logs out the HashMap state out to the Terminal
            /// @param LogEmpty Whether or not unused slots
            /// should be logged
            ////////////////////////////////////////
            void Log(bool LogEmpty = true)     noexcept;

            /// @brief Allocates memory and initialises this
            /// StorageDevice
            /// @param Allocator A pointer to the VM's
            /// CoreAllocator
            /// @return `MEMORY_OK` on successful initialisation.
            /// For other error returns, please see `MemoryError`
            ////////////////////////////////////////
            MemoryError Init(CoreAllocator* Allocator) noexcept;
            /// @brief Releases the memory of all stored
            /// `Symbol`s and other internal allocations
            ////////////////////////////////////////
            inline void Free(void)                     noexcept
                { if ( m_Map && m_Allocator ) { FreeMap(); } }
            
            /// @brief Creates a `Symbol` with the
            /// requested attributes
            /// @param Request A reference to a `StorageRequest`
            /// struct which defines which attributes are to 
            /// be used to initialise this `Symbol`
            /// @return On success, a pointer to the newly
            /// allocated `Symbol`  is returned, and result of
            /// `StorageDevice::GetLastError` is set to `SRError::OK`
            /// On failure, a nullptr is returned, and
            /// `StorageDevice::GetLastError` will return the
            /// error value. Review `SRError` for the list of
            /// return values
            ////////////////////////////////////////
            Symbol* AssignSymbol(StorageRequest& Request) 
            noexcept override final;
            
            /// @brief Retrieves a `Symbol` stored at
            /// the given Key.
            /// @param Key The key used to store the `Symbol`
            /// @return A pointer to the Symbol if it
            /// could be resolved. Otherwise returns `nullptr`
            ////////////////////////////////////////
            Symbol* LookupSymbol(const char* Key)
            noexcept override final;
            
            /// @brief Deletes the `Symbol` stored at
            /// the given key.
            /// @param Key The key used to store the `Symbol`
            /// @return True if the `Symbol` could be
            /// found and deallocated. Otherwise false.
            ////////////////////////////////////////
            bool    DeleteSymbol(const char* Key)
            noexcept override final;

            /// @brief Gets the estimated usage of this
            /// `StorageDevice`
            /// @return The number of `Symbol`s stored
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u32 GetUsage(void) const noexcept
                { return m_MapUsage; }
            
            /// @return The number of slots
            /// allocated for the internal HashMap
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            u32 GetSlotCount(void) const noexcept
                { return m_MapSize; }
    };
    
}

#endif /* !OCTVM_FLATSTORGE_HPP */
