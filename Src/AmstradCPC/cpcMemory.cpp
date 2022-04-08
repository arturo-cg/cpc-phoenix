//-------------------------------------------------------------------------------------------
// File:        Memory.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcMemory.h"
#include "cpcMemoryBlock.h"
#include "cpcMachine.h"
#include "Stream/kmbFileInputStream.h"


#define GET_ADDRESS_BLOCK(addr)   ((addr & 0xC000) >> 14)
#define GET_ADDRESS_OFFSET(addr)  (addr & 0x3FFF)


namespace CPC {


    //----------------------------------------------------------------------------
    /**
    **
    */
    CMemory::CMemory(CMachine* pMachine, const MemorySpecifications& memorySpecifications) : inherited(pMachine)
    {
        // Reset members.
        ResetVars();

        // Create and load the lower ROM bank.
        m_pLowerRomBlock = CreateRomBankFromFile(memorySpecifications.lowerRomFileName);
        KMASSERTM(m_pLowerRomBlock != NULL, ("Could not load lower ROM bank file '%s'.", memorySpecifications.lowerRomFileName.c_str()));

        // Create and load the upper ROM banks.
        KMASSERT(m_upperRomBlocks.empty());
        for (const MemorySpecifications::IntToStringMap::value_type& kvp : memorySpecifications.upperRomFileNames)
        {
            int id = kvp.first;
            string fileName = kvp.second;
            // Load the ROM file.
            CMemoryBlock* romBank = CreateRomBankFromFile(fileName);
            KMASSERTM(romBank != NULL, ("Could not load ROM file '%s' for Upper ROM ID '%d'.", fileName.c_str(), id));
            if (romBank != nullptr)
            {
                // Add the ROM bank.
                m_upperRomBlocks.insert({ id, romBank });
            }
        }

        KMASSERTM(m_upperRomBlocks.find(0) != m_upperRomBlocks.end(), ("The machine must have the BASIC ROM at ID 0."));

        // Create the RAM banks.
        // Each 64KB RAM page contains 4 x 16KB RAM banks.
        int numRamBanks = (1/*built-in RAM page*/ + memorySpecifications.numAdditionalRamPages) * 4/*banks per page*/;
        m_ramBlocks.reserve(numRamBanks);
        for (int i = 0; i < numRamBanks; i++)
        {
            char label[10];
            _snprintf_s(label, sizeof(label), "RAM%d", i);
            m_ramBlocks.push_back(new CMemoryBlock(label));
        }
    }

    //----------------------------------------------------------------------------
    /**
    ** ResetVars
    */
    void CMemory::ResetVars()
    {
        m_pLowerRomBlock = NULL;
        m_upperRomBlocks.clear();
        m_ramBlocks.clear();
    }

    //----------------------------------------------------------------------------
    /**
    ** FreeVars
    */
    void CMemory::FreeVars()
    {
        delete m_pLowerRomBlock;
        m_pLowerRomBlock = NULL;

        for (IntToMemoryBlockMap::value_type& kvp : m_upperRomBlocks)
        {
            delete kvp.second;
        }
        m_upperRomBlocks.clear();

        for (CMemoryBlock* ramBlock : m_ramBlocks)
        {
            delete ramBlock;
        }
        m_ramBlocks.clear();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ void CMemory::Reset()
    {
        //...
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    CMemoryBlock* CMemory::CreateRomBankFromFile(string fileName) const
    {
        CMemoryBlock* ret = nullptr;
        // Open the ROM file.
        kmbFileInputStream romStream;
        if (romStream.Init("Roms/" + fileName))
        {
            // Read the ROM data.
            ret = new CMemoryBlock(fileName, &romStream);
        }

        return ret;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    const CMemoryBlock* CMemory::GetUpperRomBlock(cpcByte nIndex) const
    {
        // Returns the Upper ROM with the specified ID. If it doesn't exist, the ROM with ID 0 (which is supposed to be BASIC) is returned.
        IntToMemoryBlockMap::const_iterator iterUpperRom = m_upperRomBlocks.find(nIndex);
        if (iterUpperRom == m_upperRomBlocks.end())
        {
            iterUpperRom = m_upperRomBlocks.find(0);
        }

        return iterUpperRom->second;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    unsigned CMemory::GetNumRamPages() const
    {
        return (m_ramBlocks.size() >> 2);       // Note that m_ramBlocks.size() is always a multiple of 4.
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    bool CMemory::RamPageExists(unsigned ramPage) const
    {
        return (ramPage < GetNumRamPages());
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    CMemoryBlock* CMemory::GetRamBlock(int i)
    {
        KMASSERT((i >= 0) && (i < (int)m_ramBlocks.size()));
        return m_ramBlocks[i];
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    const CMemoryBlock* CMemory::GetRamBlock(int i) const
    {
        KMASSERT((i >= 0) && (i < (int)m_ramBlocks.size()));
        return m_ramBlocks[i];
    }

} //namespace CPC
