//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCMEMORY_H_
#define _CPCMEMORY_H_


#include <unordered_map>
#include "cpcSubSystem.h"


namespace CPC {


    struct MemorySpecifications
    {
        using IntToStringMap = unordered_map<int, string>;

        unsigned numAdditionalRamPages;       // How many *additional* 64Kb RAM pages are present. Standard CPC 464 & 664 have none (only the built-in 64KB RAM); standard CPC 6128 has one (128KB in total). RAM expansion devices can be connected to provide additional 64KB pages.
        string lowerRomFileName;              // Lower ROM bank file.
        IntToStringMap upperRomFileNames;     // Upper ROM bank files, indexed by their ROM IDs.
    };

    class CMemoryBlock;


    /**
    **
    */
    class CMemory : public CSubSystem
    {
    public:

        CMemory(CMachine* pMachine, const MemorySpecifications& memorySpecifications);
        virtual ~CMemory() { FreeVars(); }

        /** Resets the subsystem. */
        virtual void Reset();

        /** Returns the lower ROM block. There is only one, and it is always the Operating System (or firmware).
        *** There are three different versions: v1 for the CPC 464, v2 for the CPC 664 and v3 for the CPC 6128. */
        const CMemoryBlock* GetLowerRomBlock() const { return m_pLowerRomBlock; }
        /** Returns the requested upper ROM block. At index 0 is the Basic ROM and always exists. On the CPC 664 and 6128,
        *** Amsdos ROM is at index 7. Other indices may contain another ROM or not. If they don't, Basic ROM is returned instead. */
        const CMemoryBlock* GetUpperRomBlock(cpcByte nIndex) const;

        /** Returns the requested RAM block. The index can go from 0 to MAX_NUM_RAM_BANKS.
        *** The first four are the 64k primary RAM page, the last four the 64k secondary RAM page. */
        CMemoryBlock* GetRamBlock(int i);
        /** Returns the requested RAM block. The index can go from 0 to MAX_NUM_RAM_BANKS (const version).
        *** The first four are the 64k primary RAM page, the last four the 64k secondary RAM page. */
        const CMemoryBlock* GetRamBlock(int i) const;

    private:

        typedef CSubSystem inherited;

        using IntToMemoryBlockMap = unordered_map<int, CMemoryBlock*>;
        using MemoryBlockList = vector<CMemoryBlock*>;

        void ResetVars();
        void FreeVars();

        CMemoryBlock* CreateRomBankFromFile(string fileName) const;

        /** The lower ROM block (operating system / firmware ROM). */
        CMemoryBlock* m_pLowerRomBlock;
        /** The upper ROM blocks present in the machine, indexed by their ROM IDs. */
        IntToMemoryBlockMap m_upperRomBlocks;
        /** The RAM blocks present in the machine. */
        MemoryBlockList m_ramBlocks;

    };

} //namespace CPC

#endif // _CPCMEMORY_H_
