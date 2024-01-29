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

#include "Headers/Instructions.hpp"

namespace Octane {

    //////////////// NOTE: /////////////////
    /// This list is in order from lowest
    /// numeric ID to highest. This ordered list
    /// MUST: NEVER: EVER: BE: MODIFIED:
    /// An ordered list in THIS SPECIFIC ORDER
    /// is compliant with OctISA specification
    /// and this ordered list can be found in
    /// the 1.0.0 specification manual.
    ////////////////////////////////////////
    static const char* OpcodeNames[] = {
        "nop",
        "chrono",
        "seek",
        "jmp",
        "jmpis0",
        "jmpnot0",
        "jmpeq",
        "jmpneq",
        "jmplt",
        "jmpgt",
        "jmplteq",
        "jmpgteq",
        "call",
        "corecall",
        "spawn",
        "spawnanon",
        "merge",
        "muop",
        "cvop",
        "ret",
        "clr",
        "mov",
        "movimm",
        "movimm32",
        "movimm64",
        "movimmf",
        "movimmd",
        "pushreg",
        "pushgen",
        "pusharg",
        "pushall",
        "pushmem",
        "popreg",
        "popgen",
        "poparg",
        "popall",
        "popmem",
        "memset",
        "memcpy",
        "offset",
        "requestbytes",
        "releasebytes",
        "requestlocal",
        "droplocal",
        "eload",
        "p2g",
        "gload8",
        "gload16",
        "gload32",
        "gload64",
        "gsave8",
        "gsave16",
        "gsave32",
        "gsave64",
        "pload8",
        "pload16",
        "pload32",
        "pload64",
        "psave8",
        "psave16",
        "psave32",
        "psave64",
        "cmpis0",
        "cmpnot0",
        "cmpeq",
        "cmpneq",
        "cmplt",
        "cmpgt",
        "cmplteq",
        "cmpgteq",
        "cmplti",
        "cmpgti",
        "cmplteqi",
        "cmpgteqi",
        "cmpltf",
        "cmpgtf",
        "cmplteqf",
        "cmpgteqf",
        "cmpltd",
        "cmpgtd",
        "cmplteqd",
        "cmpgteqd",
        "land",
        "lor",
        "lnot",
        "inc",
        "dec",
        "i2f",
        "u2f",
        "i2d",
        "u2d",
        "f2i",
        "f2u",
        "f2d",
        "d2i",
        "d2u",
        "d2f",
        "pow",
        "powi",
        "powf",
        "powd",
        "sqrt",
        "sqrtf",
        "sqrtd",
        "add", // Kept as OctASM mneomics without leading "b" for C++ impl.
        "sub", // Kept as OctASM mneomics without leading "b" for C++ impl.
        "mul", // Kept as OctASM mneomics without leading "b" for C++ impl.
        "div", // Kept as OctASM mneomics without leading "b" for C++ impl.
        "mod",
        "addimm",
        "subimm",
        "mulimm",
        "divimm",
        "modimm",
        "idiv",
        "imod",
        "idivimm",
        "imodimm",
        "fadd",
        "fsub",
        "fmul",
        "fdiv",
        "fmod",
        "dadd",
        "dsub",
        "dmul",
        "ddiv",
        "dmod",
        "and",
        "or",
        "xor",
        "not",
        "shl",
        "shr",
        "andimm",
        "orimm",
        "xorimm",
        "notimm",
        "shlimm",
        "shrimm",
    };

    const char* Instruction::GetStringName(Instruction::Opcode ID)
    {
        if (ID >= Instruction::COUNT_OF_INSTRUCTIONS) {
            return "INVALID";
        }

        return OpcodeNames[ID];
    }

}
