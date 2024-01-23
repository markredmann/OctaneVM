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

#include <iomanip>
#include <iostream>
#include "Headers/FlatStorage.hpp"

using std::cout;

namespace Octane {

/// ASSIGNTOIDX:
////////////////////////////////////////
bool FlatStorage::AssignToIDX(FSSymbol** Map, u32 MapSize, FSSymbol* Sym)
noexcept
{
    u32 IDX = Sym->KeyHash % MapSize;
    // Does a Symbol already exist at this Location?
    if ( Map[IDX] ) {
        FSSymbol* Slot = Map[IDX];
        // Make sure that these two Symbols are NOT identical
        // If they are, return false.
        if ( (Slot->KeyHash == Sym->KeyHash)
                            && QuickCmp(Slot->Key, Sym->Key) )
            return false;
        
        while (true) {
            if ( Slot->CollisonNext ) {
                Slot = Slot->CollisonNext;
            } else {
                Slot->CollisonNext = Sym;
                return true;
            }
        }
    }
    // The slot is free to use
    Map[IDX] = Sym;

    return true;
}


/// INITMAP:
////////////////////////////////////////
bool FlatStorage::InitMap(void) noexcept
{
    // Allocate our map and zero the data
    m_Map = m_Allocator->Request<FSSymbol*>
        ( MAP_BASESIZE, SYSTEM_ALLOC_FLAGS, nullptr );
    if ( !m_Map )
        return false;
    
    m_MapSize  = MAP_BASESIZE;
    m_MapUsage = 0;

    return true;
}

/// GROWMAP:
////////////////////////////////////////
    bool FlatStorage::GrowMap(void) noexcept
    {
        // Allocate a new, larger map alongside our current map.
        u32  NewMapSize = m_MapSize + MAP_STEPSIZE;
        auto NewMap = m_Allocator->Request<FSSymbol*>
            ( NewMapSize, SYSTEM_ALLOC_FLAGS, nullptr );
        
        if ( !NewMap )
            return false;
        
        // Loop through our current map and transfer our values.
        for( u32 i = 0; i < m_MapSize; i++ ) {
            FSSymbol* Symbol = m_Map[i];
            FSSymbol* CollisionNext;

            while ( Symbol ) {
                CollisionNext = Symbol->CollisonNext;
                Symbol->CollisonNext = nullptr;

                AssignToIDX(NewMap, NewMapSize, Symbol);

                Symbol = CollisionNext;
            }
        }


        // Free our old map and set our new one in its place.
        // Note: This is only freeing the array of Symbols,
        // not the Symbols themselves. For this. see `FreeMap`
        m_Allocator->Release<FSSymbol*>(m_Map);
        m_Map = NewMap;
        m_MapSize = NewMapSize;

        return true;
    }

/// FREEMAP:
////////////////////////////////////////
    void FlatStorage::FreeMap(void) noexcept
    {
        // Iterate through every Symbol and free it
        for ( u32 i = 0; i < m_MapSize; i++ ) {
            FSSymbol* Symbol = m_Map[i];
            FSSymbol* CollisionNext;

            while ( Symbol ) {
                CollisionNext = Symbol->CollisonNext;

                m_Allocator->Release<char>(Symbol->Key);
                m_Allocator->Release<FSSymbol>(Symbol);

                Symbol = CollisionNext;
            }
        }

        // Free the map itself and null our values
        m_Allocator->Release<FSSymbol*>(m_Map);
        m_Map      = nullptr;
        m_MapSize  = 0;
        m_MapUsage = 0;
    }

/// FLATSTORAGE:
////////////////////////////////////////

/// LOG:
////////////////////////////////////////

void FlatStorage::Log(bool LogEmpty) noexcept
{
    cout << "FlatStorage(" << this << ") :\n";
    cout << "    Allocator : " << m_Allocator << '\n';
    cout << "    Map Size  : " << m_MapSize   << '\n';
    cout << "    Map Usage : " << m_MapUsage  << '\n';
    for ( u32 i = 0; i < m_MapSize; i++ ) {
        cout << "    [" << std::setfill('0') << std::setw(4) << i << "] : ";
        FSSymbol* Symbol = m_Map[i];
        int Depth = 0;
        if ( Symbol )
            while ( Symbol ) {
                const char* endtext =
                (Symbol->CollisonNext ? " (COLLISIONS!)\n    [^^^^] >" : "\n");
                for ( int i = 0; i < Depth; i ++ )
                    cout << "    ";
                cout << '"' << Symbol->Key << '"' << endtext;
                Symbol = Symbol->CollisonNext;
                Depth++;
            }
        else if ( LogEmpty )
            cout << "(empty)\n";
    }
}

/// INIT:
////////////////////////////////////////
MemoryError FlatStorage::Init(CoreAllocator* Allocator) noexcept
{
    m_Allocator = Allocator;
    m_LastError = SRError::OK;
    
    if ( ! InitMap() )
        return m_Allocator->GetLastError();
    return MEMORY_OK;
}

/// ASSIGNSYMBOL:
////////////////////////////////////////
Symbol* FlatStorage::AssignSymbol(StorageRequest& Request)  noexcept
{
    // Ensure this StorageDevice and the key are valid 
    if ( !m_Map || !m_Allocator )
        { m_LastError = SRError::INVALID_STORAGE;
          return nullptr;}
        
    
    u32 KeyLen = QuickStrLen(Request.Key);
    if ( KeyLen > FSSymbol::MAX_KEY_SIZE )
        { m_LastError = SRError::INVALID_KEY; 
          return nullptr; }
    
    // If we don't have enough (estimated) space, grow our Map
    if ( m_MapUsage + 1 >= m_MapSize ) {
        if ( !GrowMap() )
            { m_LastError = SRError::NOT_ENOUGH_SPACE;
              return nullptr; }
    }

    // Allocate and initalise our new Symbol
    FSSymbol* Symbol = m_Allocator->Request<FSSymbol>();
    if ( !Symbol ) // Most likely cause of a null ret is OOM.
        { m_LastError = SRError::NOT_ENOUGH_SPACE;
          return nullptr; }
    
    Symbol->Type         = Request.Type;
    Symbol->ExtendedType = Request.ExtendedType;
    Symbol->Value        = Request.Value;
    Symbol->CollisonNext = nullptr;
    // Allocate a copy of our new Key
    Symbol->Key = m_Allocator->Request<char>(KeyLen + 1);
    Symbol->KeyHash = QuickSDBM(Request.Key, KeyLen);
    if ( !Symbol->Key ) {
        m_Allocator->Release<FSSymbol>(Symbol);
        m_LastError = SRError::NOT_ENOUGH_SPACE;
        return nullptr;
    }
    QuickCopy(Request.Key, Symbol->Key, KeyLen + 1);

    // Finally, attempt to assign our Symbol to an Index
    bool IsNewEntry = AssignToIDX(m_Map, m_MapSize, Symbol);
    if ( !IsNewEntry ) { // Symbol already existed
        m_Allocator->Release<char>(Symbol->Key);
        m_Allocator->Release<FSSymbol>(Symbol);
        m_LastError = SRError::SYMBOL_EXISTS;
        return nullptr;
    }
    
    // All clear!
    m_MapUsage++;
    m_LastError = SRError::OK;
    return Symbol;
}

Symbol* FlatStorage::LookupSymbol(const char* Key) noexcept
{
    // Ensure both our Key and StorageDevice are valid
    if ( !Key || !m_Map )
        return nullptr;
    
    u32 KeyLen  = QuickStrLen(Key);
    u64 KeyHash = QuickSDBM(Key, KeyLen);
    
    // Lookup and validate
    u32 IDX = KeyHash % m_MapSize;
    FSSymbol* Slot = m_Map[IDX];
    if ( !Slot )
        return nullptr;

    // See if its in a CollisionNext or the top level
    while (Slot) {
        if ( Slot->KeyHash == KeyHash 
                           && QuickCmp(Slot->Key, Key, KeyLen) )
            return Slot;

        Slot = Slot->CollisonNext;
    }

    // How did this happen? Supreme mega error if this is ever reached
    return nullptr;
}

bool    FlatStorage::DeleteSymbol(const char* Key) noexcept
{
    // Sanity check inputs
    if ( !Key || !m_Map  || m_MapUsage )
        return false;

    u32 KeyLen  = QuickStrLen(Key);
    u32 KeyHash = QuickSDBM(Key, KeyLen);
    u32 IDX = KeyHash % m_MapSize;

    FSSymbol* Root = m_Map[IDX];
    FSSymbol* DeletionSymbol = Root;
    // If its the very first one, replace [IDX] with its
    // CollisionNext, then delete it.
    if ( Root->KeyHash == KeyHash 
                       && QuickCmp(Root->Key, Key, KeyLen) )
        m_Map[IDX] = Root->CollisonNext;
    else {
    // Walk the Next tree and find which one is our requested Symbol
        while (true) {
            if ( !Root->CollisonNext )
                return false; // Something went horribly.. horribly wrong...
            
            // Its the next one!
            if ( Root->CollisonNext->KeyHash == KeyHash 
                 && QuickCmp(Root->CollisonNext->Key, Key, KeyLen) )
            {
                DeletionSymbol = Root->CollisonNext;
                Root->CollisonNext = DeletionSymbol->CollisonNext;
                break;
            }

            // Continue (still searching)
            Root = Root->CollisonNext;
        }

    }

    m_Allocator->Release<char>(DeletionSymbol->Key);
    m_Allocator->Release<FSSymbol>(DeletionSymbol);

    m_MapUsage--;
    return true;
}

}
