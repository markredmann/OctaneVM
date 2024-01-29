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

#ifndef OCTVM_INSTRUCTIONS_HPP
#define OCTVM_INSTRUCTIONS_HPP 1

#include "Common.hpp"

namespace Octane {

    union Instruction {
    /// INTERNALS: AND: METHODS:
    ////////////////////////////////////////
        enum Opcode : u8;
        using Width = u32;
        static const char* GetStringName(Opcode ID);
        constexpr static const u8 UNUSED_REG = 0xFF;
    /// INSTRUCTION: VARIANTS:
    ////////////////////////////////////////
        struct _any
        {
            Opcode Op;
        };

        struct _oneparam : _any
        {
            u8 rX;
        };

        struct _dualparam : _any
        {
            u8 rX, rY;
        };

        struct _triparam : _any
        {
            u8 rX, rY, rZ;
        };

        struct _imm16 : _any
        {
            u8  rX;
            u16 Imm;
        };

        struct _imm16_alt : _any
        {
            u8  rX_rY;
            u16 Imm;
        };

        struct _memaccess : _any
        {
            u8 rX_rY;
            u8 rZ;
            u8 Scale;
        };
        struct _memaccess_priv : _any
        {
            u8 rX, rY, Scale;
        };

    /// INSTRUCTION: IMPLEMENTATION:
    ////////////////////////////////////////
        Width           RawInt;
        u8              RawBytes[(sizeof(Width))];
        _any            Any;
        _oneparam       OneParam;
        _dualparam      DualParam;
        _triparam       TriParam;
        _imm16          Imm16;
        _imm16_alt      Imm16Alt;
        _triparam       Imm32;
        _triparam       Imm64;
        _memaccess      MemAccess;
        _memaccess_priv MemAccessPriv;
        _triparam       Optional32;
    
    /// OPCODES:
    ////////////////////////////////////////

        //////////////// NOTE: /////////////////
        /// This list is in order from lowest
        /// numeric ID to highest. This ordered list
        /// MUST: NEVER: EVER: BE: MODIFIED:
        /// An ordered list in THIS SPECIFIC ORDER
        /// is compliant with OctISA specification
        /// and this ordered list can be found in
        /// the 1.0.0 specification manual.
        ////////////////////////////////////////
        enum Opcode : u8 {
            /*** GENERIC: ***/
            nop, chrono,
            /*** CONTROLFLOW: ***/
            seek, jmp, jmpis0, jmpnot0, jmpeq, jmpneq, jmplt, jmpgt, jmplteq,
            jmpgteq, call, corecall, spawn, spawnanon, merge, muop, cvop, ret,
            /*** REGISTERS: ***/
            clr, mov, movimm, movimm32, movimm64, movimmf, movimmd,
            /*** STACK: ***/
            pushreg, pushgen, pusharg, pushall, pushmem, popreg, popgen,
            poparg, popall, popmem,
            /*** MEMORY: - GENERIC: ***/
            memset, memcpy, offset, requestbytes, releasebytes, requestlocal,
            droplocal, eload, p2g, 
            /*** MEMORY: - GLOBAL: ***/
            gload8, gload16, gload32, gload64,
            gsave8, gsave16, gsave32, gsave64,
            /*** MEMORY: - PRIVATE: ***/
            pload8, pload16, pload32, pload64, 
            psave8, psave16, psave32, psave64,
            /*** COMPARISON: ***/
            cmpis0, cmpnot0, cmpeq, cmpneq, cmplt, cmpgt, cmplteq, cmpgteq,
            cmplti, cmpgti, cmplteqi, cmpgteqi, cmpltf, cmpgtf, cmplteqf,
            cmpgteqf, cmpltd, cmpgtd, cmplteqd, cmpgteqd,
            /*** LOGICAL: ***/
            land, lor, lnot,
            /*** ARITHMETIC: ***/
            inc, dec, i2f, u2f, i2d, u2d, f2i, f2u, f2d, d2i, d2u, d2f, pow,
            powi, powf, powd, sqrt, sqrtf, sqrtd, add, sub, mul, div, mod,
            addimm, subimm, mulimm, divimm, modimm, idiv, imod, idivimm,
            imodimm, fadd, fsub, fmul, fdiv, fmod, dadd, dsub, dmul, ddiv, dmod,
            /*** BITWISE: ***/
            /// NOTE: These start with "b" ONLY for C++ implementation, as
            /// "and|or|xor|not" are reserved keywords in C++..
            /// The true mnemonics do not have "b" prepended to them, and
            /// MUST NOT be parsed as such as per OctASM specification.
            band, bor, bxor, bnot, shl, shr, bandimm, borimm, bxorimm, bnotimm,
            shlimm, shrimm,
            /*** METADATA: ***/
            COUNT_OF_INSTRUCTIONS
        };
    };

}

#endif /* !OCTVM_INSTRUCTIONS_HPP */
