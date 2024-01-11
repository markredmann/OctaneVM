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

#ifndef OCTVM_CORESTORAGE_HPP
#define OCTVM_CORESTORAGE_HPP 1
#include "Common.hpp"

namespace Octane {

/// SYMBOL:
////////////////////////////////////////


    /// @brief An enumeration containing
    /// different possible internal types for
    /// Symbols stored in StorageDevices.
    ////////////////////////////////////////
    enum class SymbolType : u8 {
        /// This `Symbol` was not properly
        /// initialised and/or does not exist.
        INVALID,
        /// This `Symbol` points to a `Function`
        /// that can be used with a `call`,
        /// `calldyn`, `spawn` or `spawnanon`
        /// instruction.
        /// Note that those instructions can ONLY use
        /// Symbols with this Type. If this is not set,
        /// the VM will refuse to execute the `Function`.
        FUNC,
        /// This `Symbol` points to arbitrary data
        DATA,
        /// This `Symbol` points to arbitrary data
        /// not to be accessed by the running executable
        METADATA,
        /// This `Symbol` points not to any data
        /// or function, but instead a collection of
        /// `Symbol`s, be it functions, data, or even
        /// other collections.
        /// This is not used by OctaneVM itself but can
        /// be used in languages implementing namespaces
        /// such as REL.
        COLLECTION,
        /// This `Symbol` points to custom type that
        /// is defined by a third-party implementation
        /// or runtime. If implementing it yourself,
        /// set `m_ExtendedType` to your own
        /// custon enumerator if you wish to denote a
        /// new, specific custom type.
        EXTENDED,
    }; 

    //////////////// NOTE: /////////////////
    /// Symbols are to be extended by the
    /// Storage Device implementation.
    ////////////////////////////////////////
    
    /// @brief A Base Class for representing
    /// a globally accessible `Symbol` for
    /// usage in VM executables. 
    ///
    /// This class can be extended with new
    /// members or internal functionality for
    /// different implementations of `StorgeDevice`
    ////////////////////////////////////////
    struct Symbol {
        /// What is the Type of this `Symbol`?
        SymbolType Type;
        /// If `Type` is `SymbolType::EXTENDED`, this will
        /// denote the extended Type. This is
        /// set and maintained by third-party
        /// implementations and/or runtimes.
        /// Custom `StorageDevice` implementations
        /// may also use this for internal purposes.
        u32         ExtendedType;
        /// A pointer to the actual contents of this `Symbol`.
        void*      Value;

        template <typename Type>
        constexpr OctVM_SternInline
        Type* CastValue(void) const noexcept
            { return (Type*)Value; }
    };

/// STORAGE:
////////////////////////////////////////

    struct StorageRequest {
        /// What is the `SymbolType` of the `Symbol` to store?
        SymbolType  Type;
        /// If Type is `SymbolType::EXTENDED`, this value
        /// is used to denote a more specific third-party
        /// type implementation, This is used if your
        /// implementation of a `StorageDevice` requires
        /// extra types that are not used by the OctaneVM
        u8          ExtendedType;
        /// What is the Key that will be used to lookup
        /// this `Symbol` from? Note that some `StorageDevice` 
        /// implementations may place restrictions on what is and 
        /// is not allowed inside of a Key, such as collection/namespace
        /// separators, UTF-8 support or lack thereof, etc.
        /// Review the documentation for the given `StorageDevice`.
        const char* Key;
        /// What is the Value that this `Symbol` will store?
        void*       Value;
        /// How large in bytes is our Value?
        /// Some implementations of `StorageDevice` may
        /// copy store the Value in its own memory space.
        /// Review the documentation for your `StorageDevice`
        /// to ensure it does or does not copy the Value.
        u32         ValueSize;
    };

    /// @brief An enumeration containing
    /// the result of an attempted `StorageDevice`
    /// `Symbol` assignment
    ////////////////////////////////////////
    enum class SRError {
        /// Assignment was OK; no errors.
        OK,
        /// Assignment couldn't be made because a `Symbol`
        /// already exists at the given Key.
        SYMBOL_EXISTS,
        /// The Key is invalid and cannot be used to store
        /// a Value. It is likely either too long or
        /// contains invalid or malformed characters.
        INVALID_KEY,
        /// The Value is invalid and cannot be stored in this
        /// `StorageDevice`.
        INVALID_VALUE,
        /// The `StorageDevice` is invalid and/or uninitialised.
        INVALID_STORAGE,
        /// There is not enough memory to allocate space for
        /// this `Symbol`, or other underlying allocations
        /// necessary for this implementation of `StorageDevice`.
        NOT_ENOUGH_SPACE,
    };

    /// @brief An Abstract Base Class for the
    /// purpose of storing `Symbols` for allowing
    /// quick lookup in VM executables
    ///
    /// Note that this is a base class, and
    /// does not provide any default 
    /// implementations for any of the methods.
    /// For implementations provided by this
    /// API, see `FlatStorage`, or `StaticFlatStorage`.
    ////////////////////////////////////////
    class StorageDevice {
        public:
            /// @brief Assign a `Symbol` to a key for VM
            /// executables to lookup and retrieve during
            /// execution.
            /// @param Request A small struct with fields denoting
            /// how this `Symbol` should be stored, created, and accessed.
            /// @return An `SRError` value denoting whether the assignment
            /// was successful, and if not, why it failed. Review the
            /// documentation for `SRError` for more information.
            ////////////////////////////////////////
            virtual SRError AssignSymbol(StorageRequest& Request) noexcept = 0;
            /// @brief Retrieve a `Symbol` from a given Key.
            /// @param Key A null-terminated string used to store and/or
            /// retrieve a `Symbol` at the given Key.
            /// @return A pointer to the resolved `Symbol`. If the
            /// `Symbol` does not exist, or the Key is malformed, then
            /// a `nullptr` will be returned.
            ////////////////////////////////////////
            virtual Symbol* LookupSymbol(const char* Key) noexcept = 0;
            /// @brief Deletes a `Symbol` stored with the given Key.
            /// @param Key A null-terminatd string used to store, retrieve, 
            /// or delete a `Symbol`.
            /// @return Will return true if the `Symbol` did exist at 
            /// the given Key and was successfully deleted.
            /// Will return false the `Symbol` did not exist, OR this 
            /// `StorageDevice` DOES NOT support deletion.
            ////////////////////////////////////////
            virtual bool    DeleteSymbol(const char* Key) noexcept = 0;
    };

}

#endif /* !OCTVM_CORESTORAGE_HPP */
