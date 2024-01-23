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

#ifndef OCTVM_THREADING_PRIMITIVES_HPP
#define OCTVM_THREADING_PRIMITIVES_HPP 1

#include <thread>
#include <mutex>

namespace Octane {
    /// @brief Mutual Exclusion Lock. 
    /// Wrapper for the one provided by the
    /// Standard Template Library.
    ////////////////////////////////////////
    class Mutex {
        private:
            /// The internal Mutex
            std::mutex m_Mutex;
            /// Is the Mutex locked?
            bool       m_Locked = false;
        public:
            constexpr OctVM_SternInline
            bool IsLocked(void) const noexcept
                { return m_Locked; }
            
            OctVM_SternInline
            void Lock(void) noexcept
                { m_Mutex.lock();   m_Locked = true;}
            OctVM_SternInline
            void Unlock(void) noexcept
                { m_Mutex.unlock(); m_Locked = false;}
    };
    
    /// @brief RAII-based Mutex locker.
    ////////////////////////////////////////
    class RAIIMutex {
        private:
            // A Reference to the Mutex to lock.
            Mutex& m_Mutex;
        public:

            /// @brief Initialises this Locker.
            /// @param _Mutex A Reference to the
            /// Mutex to lock and unlock on scope
            /// exit.
            /// @param AutoLock Should the Mutex
            /// lock immediately?
            ////////////////////////////////////////
            OctVM_SternInline 
            RAIIMutex(Mutex& _Mutex, bool AutoLock = true) noexcept
            : m_Mutex{_Mutex} {
                if ( AutoLock )
                    m_Mutex.Lock();
            }
            
            OctVM_SternInline ~RAIIMutex(void)
                { Unlock(); }

            /// @brief Manually lock the stored Mutex.
            ////////////////////////////////////////
            OctVM_SternInline void Lock(void) noexcept {
                if ( !m_Mutex.IsLocked() )
                    m_Mutex.Lock();
            }

            /// @brief Manually unlock the stored Mutex.
            ////////////////////////////////////////
            OctVM_SternInline void Unlock(void) noexcept {
                if ( m_Mutex.IsLocked() )
                    m_Mutex.Unlock();
            }
    };
}

#endif /* !OCTVM_THREADING_PRIMITIVES_HPP */
