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

#ifndef OCTVM_COMMON_HPP
#define OCTVM_COMMON_HPP 1

// Obvious check
#ifndef __cplusplus
    #error [COMPAT ERROR] : (!defined(__cplusplus)) : \
    OctaneVM is a C++ framework. Attempt compilation on a C++ compiler \
    such as Clang++
#endif

#include <cstdint>
#include <cstddef>
#include <climits>

/// Check Compatible C++ Versions
/// OctaneVM is built for C++17, but is
/// supported on C++11, and no earlier.
/// Define other version-specific
/// macros here as well.
////////////////////////////////////////

#if   (__cplusplus >= 202002L) // C++20
    #define OctVM_WarnDiscard(WarningText) [[nodiscard(WarningText)]]
#elif (__cplusplus >= 201703L) // C++17
    #define OctVM_WarnDiscard(WarningText) [[nodiscard]]
#elif (__cplusplus >= 201402L) // C++14
    #define OctVM_WarnDiscard(WarningText) // Not supported in C++14
#elif (__cplusplus >= 201103L) // C++11 - Earliest Supported Version!
    #define OctVM_WarnDiscard(WarningText) // Not supported in C++11
#else
    #error [COMPAT ERROR] : (__cplusplus < 201103L) : \
    OctaneVM is intended for use with C++17 or newer, however it supports \
    C++11. C++ versions earlier than this are not supported. Upgrade \
    your compiler, or switch standard version to be -std=c++11 or higher.
#endif

#ifndef OCTVM_DONT_PERFORM_TESTS
/// --- REQUIRED TESTS ---
//  If these do NOT pass, OctaneVM will NOT supported on the current platform. 

    #if CHAR_BIT != 8
        #error \
        [COMPAT ERROR] : (CHAR_BIT != 8) : \
        OctaneVM is NOT supported on platforms lacking octets.
    #endif

    #if !defined(UINT8_MAX) || !defined(INT8_MAX)
        #error \
        [COMPAT ERROR] : (!defined(UINT8_MAX) || !defined(INT8_MAX)) : \
        OctaneVM is NOT supported on platforms lacking fixed-width \
        8-bit integers.
    #endif

    #if !defined(UINT16_MAX) || !defined(INT16_MAX)
        #error \
        [COMPAT ERROR] : (!defined(UINT16_MAX) || !defined(INT16_MAX)) : \
        OctaneVM is NOT supported on platforms lacking fixed-width \
        16-bit integers.
    #endif

    #if !defined(UINT32_MAX) || !defined(INT32_MAX)
        #error \
        [COMPAT ERROR] : (!defined(UINT32_MAX) || !defined(INT32_MAX)) : \
        OctaneVM is NOT supported on platforms lacking fixed-width \
        32-bit integers.
    #endif

    #if !defined(UINT64_MAX) || !defined(INT64_MAX)
        #error \
        [COMPAT ERROR] : (!defined(UINT64_MAX) || !defined(INT64_MAX)) : \
        OctaneVM is NOT supported on platforms lacking fixed-width \
        64-bit integers.
    #endif

    #if (__SIZEOF_DOUBLE__ != 8) || (__SIZEOF_FLOAT__ !=4)
        #error \
        [COMPAT ERROR] : ((__SIZEOF__DOUBLE !=8) || (__SIZEOF_FLOAT__ !=4)) : \
        OctaneVM is NOT supported on platforms without fixed-width \
        floating point numbers.
    #endif

/** ! --- REQUIRED TESTS --- **/
#endif /* !OCTVM_DONT_PERFORM_TESTS */

// If the compiler does not support __attribute__((always_inline))
#ifdef OCTVM_NO_STERN_INLINE
    // With Stern Inline disabled, the Compiler
    // can decide to ignore our stern suggestion.
    #define OctVM_SternInline inline
#else  /* !OCTVM_NO_STERN_INLINE */
    // Sternly tells the compiler to inline the given function.
    // On Compilers where __attribute__((always_inline)) is
    // not supported, this will default to a standard inline,
    // which is a suggestion rather than an order.
    #define OctVM_SternInline inline __attribute__((always_inline))
#endif /* OCTVM_NO_STERN_INLINE */


/// @brief The OctaneVM Namespace.
////////////////////////////////////////
namespace Octane {
    /// Common Types
    ////////////////////////////////////////
    
    using byte = std::uint8_t;
    using i8   = std::int8_t;
    using i16  = std::int16_t;
    using i32  = std::int32_t;
    using i64  = std::int64_t;
    using u8   = std::uint8_t;
    using u16  = std::uint16_t;
    using u32  = std::uint32_t;
    using u64  = std::uint64_t;
    using f32  = float;  // Guaranteed by above checks
    using f64  = double; // Guaranteed by above checks

    /// Common Functions
    ////////////////////////////////////////
    
    /// @brief Returns a String of the boolean.
    /// @param Value the Boolean to string-ify.
    /// @return A const char* containing
    /// either "True" or "False".
    ////////////////////////////////////////
    static constexpr inline // The compiler can decide on this one.
    const char* BoolStr(const bool Value) noexcept
        { return ( Value ? "True" : "False" ); }

    //////////////// TODO: /////////////////
    /// Define these functions.
    ////////////////////////////////////////
    
    /// @brief Computes a simple SDBM numerical
    /// hash of the given block.
    /// @param Data The block of memory to compute a hash of.
    /// @param DataLenThe length, in bytes, to 
    /// compute from the given memory address. 
    /// If blank, or zero, the address will be 
    /// treated as a null-terminated string.
    /// @return The computed SDBM numerical hash.
    /// If NULL, a hash could not be computed, 
    /// because either the address was NULL, 
    /// contents were NULL, or it contained 
    /// an immmediate null byte while operating 
    /// in String mode.
    ////////////////////////////////////////
    extern u32  QuickSDBM   (const void* Data, u32 DataLen = 0)      noexcept;
    
    /// @brief Returns the length of
    /// a null-terminated String.
    /// @param String The String to retrieve 
    /// the length from.
    /// @return The number of bytes that make 
    /// up the string, up until the null terminator.
    ////////////////////////////////////////
    extern u32  QuickStrLen (const char* String)                     noexcept;
    
    /// @brief Compares the contents of the 
    /// two memory Addresses.
    /// @param A The primary Address
    /// @param B The Address to compare against
    /// @param ALen The length of Address A.
    /// If left blank or set to 0, the
    /// Address will be treated as a null-terminated
    /// String and will run until it hits a
    /// NULL byte.
    /// @param BLen The length of Address B.
    /// If left blank or set to 0, the
    /// Address will be treated as a null-terminated
    /// String and will run until it hits a
    /// NULL byte.
    /// @return True if both Addresses
    /// contain equal memory, otherwise false.
    ////////////////////////////////////////
    extern bool QuickCmp    (const void* A,      const void* B, 
                             const u32 ALen = 0, const u32 BLen = 0) noexcept;
    
}

#endif /* !OCTVM_COMMON_HPP */
