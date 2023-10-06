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

#ifndef OCTVM_COMMON_HH
#define OCTVM_COMMON_HH 1

#include <cstdint>
#include <cstddef>
#include <climits>

#ifndef OCTVM_DONT_PERFORM_TESTS
/// --- REQUIRED TESTS ---
//  If these do NOT pass, OctaneVM will NOT supported on the current platform. 
    #if defined(DEMODEFINE)
        #error \
        [COMPAT ERROR] : (CHAR_BIT != 8) : \
        OctaneVM is NOT supported on platforms lacking octets.
    #endif

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


/// @brief The OctaneVM Namespace.
////////////////////////////////////////
namespace Octane {

    /// Common Types
    ////////////////////////////////////////
    
    using byte = std::byte;
    using i8   = std::int8_t;
    using i16  = std::int16_t;
    using i32  = std::int32_t;
    using i64  = std::int64_t;
    using u8   = std::uint8_t;
    using u16  = std::uint16_t;
    using u32  = std::uint32_t;
    using u64  = std::uint64_t;
}

#endif /* !OCTVM_COMMON_HH */
