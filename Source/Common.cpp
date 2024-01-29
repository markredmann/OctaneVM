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

#include "Headers/Common.hpp"

namespace Octane {
    u32 QuickSDBM(const void* _Data, u32 DataLen) noexcept
    {
        if(!_Data) return 0;

        u32       Hash = 0;
        const u8* Data = (const u8*)_Data;

        if(DataLen)
            while( DataLen--)
                Hash = *(Data++) + (Hash << 6) + (Hash << 16) - Hash;
        else
            while( *Data )
            Hash = *(Data++) + (Hash << 6) + (Hash << 16) - Hash;

        return Hash;
    }

    u32 QuickStrLen(const char* String) noexcept
    {
        u32 Len = 0;
            while( *String++ )
                Len++;
        return Len;
    }

    bool QuickCmp(const void* _A, const void* _B, u32 Len) noexcept
    {
        const u8* A = (const u8*)_A;
        const u8* B = (const u8*)_B;

        if ( Len ) {
            while ( Len-- )
                if ( *(A++) != *(B++) )
                    return false;
        } else {
            while ( true )
                if ( *(A++) != *(B++) )
                    return false;
                else if ( *A == 0 || *B == 0)
                    return true;
        }

        return true;
    }

    //////////////// TODO: /////////////////
    /// Probably just replace this with a
    /// direct call to memcpy(), as it would
    /// likely be faster and more portable.
    ////////////////////////////////////////
    
    void QuickCopy   (const void* _Src, void* _Dest, u32 Len) noexcept
    {
        const u8* Src  = (const u8*)_Src;
        u8*       Dest = (u8*)_Dest;
        const u8* End = Src + Len;

        while ( Src != End )
            *(Dest++) = *(Src++);
    }

    void QuickSet    (u8 Value, void* _Dest, u32 Len) noexcept
    {
        u8*       Dest = (u8*)_Dest;
        const u8* End = Dest + Len;

        while ( Dest != End )
            *(Dest++) = Value;
    }

}
