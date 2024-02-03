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

#ifndef OCTVM_EXCEPTIONS_HPP
#define OCTVM_EXCEPTIONS_HPP 1

#include "Common.hpp"
#include "Instructions.hpp"
#include "VPCore.hpp"

namespace Octane {
    
    /// @brief An encapsulation of an internal
    /// Exception that can be thrown by the
    /// VM during runtime or static validation.
    ////////////////////////////////////////
    class Exception {
        public:
            /// @brief An enumeration containing all
            /// possible Exceptions
            ////////////////////////////////////////
            enum ID : u8 {
            /*** STATIC: ***/
                None,
                InvalidRegisterAccess,
                InvalidUnusedRegister,
            /*** STATIC: OR: RUNTIME:*/
                InstructionOverflow,
                InstructionUnderflow,
            /*** RUNTIME: ***/
                DivideByZeroI,
                DivideByZeroU,
                DivideByZeroF,
                DivideByZeroD,
                
                StackOverflow,
                StackUnderflow,
                StackUnset,

                LocalAccessOverflow,
                LocalAccessUnderflow,
                LocalUnset,

                HeapOutOfMemory,
                LocalOutOfMemory,
            };

            /// @brief An explicit enumeration
            /// containing the result of a `HandlerFunc`
            ////////////////////////////////////////
            enum class HandlerResult {
                /// No `Exception` occured.
                NO_EXCEPTION,
                /// The `Exception` was handled gracefully.
                HANDLED,
                /// The `Exception` was ignored, and
                /// the `Instruction` was not executed.
                IGNORED,
                /// The `Exception` was fatal, and the VM
                /// was ordered to halt all execution.
                FATAL
            };
            
            /// @brief A function that handles an
            /// `Exception`.
            ////////////////////////////////////////
            using HandlerFunc = HandlerResult(*)(Exception, ExecState&);

        private:
            /// What is the Type of this Exception?
            ID          m_Type         = ID::None;
            /// Was this Exception thrown from a
            /// VM-bytecode `Function` or a native,
            /// C++ function exposed to the VM?
            bool        m_IsVMFunc     = true;
            /// Is this Exception being thrown in
            /// a Static Evaluation context?
            bool        m_IsStaticEval = false;
            /// The offending `Instruction` which caused
            /// this Exception to be thrown.
            Instruction m_Offender;
        public:
        /// INITIALISATION:
        ////////////////////////////////////////
            Exception(ID eID, bool VMFunc = false, bool StaticEval = false)
                : m_Type(eID),
                  m_IsVMFunc(VMFunc),
                  m_IsStaticEval(StaticEval) {}
            
            Exception(ID eID, Instruction Offender, bool StaticEval = false) 
                : m_Type(eID), 
                  m_IsVMFunc(true),
                  m_IsStaticEval(StaticEval),
                  m_Offender(Offender) {}

        /// QUERY:
        ////////////////////////////////////////
            
            constexpr OctVM_SternInline
            /// @brief If this Exception was thrown
            /// from a VM-bytecode `Function`, this
            /// will return true. Otherwise false if
            /// it was thrown from a native C++
            /// function exposed to the VM.
            ////////////////////////////////////////
            bool IsVMFunc(void) const noexcept
                { return m_IsVMFunc; }

            constexpr OctVM_SternInline
            /// @brief If this Exception was thrown
            /// from a native C++ function exposed to
            /// the VM, this will return true.
            /// Otherwise false, indicating this
            /// Exception was thrown from VM-bytecode.
            ////////////////////////////////////////
            bool IsCFunc(void) const noexcept
                { return !m_IsVMFunc; }
                
            constexpr OctVM_SternInline
            /// @brief Is this Exception being thrown
            /// from a Static Evaluation context?
            /// If true, there are no real Registers
            /// in use and no corrective action to
            /// Registers needs to be taken.
            ////////////////////////////////////////
            bool IsStaticEval(void) const noexcept
                { return m_IsStaticEval; }
                
        /// GETTERS:
        /// --- PISSED OFF NOTE --- @markredmann
        /// What is this bullshit, VS Code?
        /// See Functions.hpp to see why I'm pissed.
        ////////////////////////////////////////

            /// @brief Returns the ID of this Exception
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            ID GetID(void) const noexcept
                { return m_Type; }
            
            /// @brief Returns the offending `Instruction`
            /// that caused this Exception to be thrown.
            ////////////////////////////////////////
            constexpr OctVM_SternInline
            Instruction GetOffendingInstruction(void) const noexcept
                { return m_Offender; }
    };


}

#endif /* !OCTVM_EXCEPTIONS_HPP */
