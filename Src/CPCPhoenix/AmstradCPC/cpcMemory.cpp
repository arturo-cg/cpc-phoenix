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


  struct SMemoryProfile
  {
    unsigned nRamBlockCount;                                      // How many 16Kb RAM blocks are present. It must be 4 (64Kb) or 8 (128Kb).
    string   sLowerRomFileName;                                   // Lower ROM file.
    string   sUpperRomFileNames[CMemory::UPPER_ROM_BLOCK_COUNT];  // Upper ROM files.
  };

  static SMemoryProfile s_aMemoryProfiles[CMachine::MODEL_LAST] =
  {
    { 4/*64Kb RAM*/,  "OS_464.ROM",  { "BASIC_464.ROM",  "",            "", "", "", "", "", "",                "", "", "", "", "", "", "", "" } },    // MODEL_464
    { 4/*64Kb RAM*/,  "OS_664.ROM",  { "BASIC_664.ROM",  "",            "", "", "", "", "", "AMSDOS_664.ROM",  "", "", "", "", "", "", "", "" } },    // MODEL_664
    { 8/*128Kb RAM*/, "OS_6128.ROM", { "BASIC_6128.ROM", "",            "", "", "", "", "", "AMSDOS_6128.ROM", "", "", "", "", "", "", "", "" } },    // MODEL_6128
    { 8/*128Kb RAM*/, "OS_6128.ROM", { "BASIC_6128.ROM", "maxam15.rom", "", "", "", "", "", "AMSDOS_6128.ROM", "", "", "", "", "", "", "", "" } },    // MODEL_6128_MAXAM
  };


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMemory::CMemory(CMachine *pMachine) : inherited( pMachine )
  {
    // Reset members
    ResetVars();

    // Get the memory profile for the current machine model
    const SMemoryProfile& memoryProfile = s_aMemoryProfiles[pMachine->GetModel()];

    // Create the lower ROM block
    kmbFileInputStream romStream;
    if ( romStream.Init("Roms/" + memoryProfile.sLowerRomFileName) )
    {
      m_pLowerRomBlock = new CMemoryBlock( memoryProfile.sLowerRomFileName, &romStream );
    }
    else
    {
      m_pLowerRomBlock = NULL;
      KMASSERTM( m_pLowerRomBlock != NULL, ("Could not load lower ROM block file '%s'.", memoryProfile.sLowerRomFileName.c_str()) );
    }

    // Create the upper ROM blocks
    unsigned i;
    for (i = 0; i < UPPER_ROM_BLOCK_COUNT; i++)
    {
      if ( !memoryProfile.sUpperRomFileNames[i].empty() )
      {
        if ( romStream.Init("Roms/" + memoryProfile.sUpperRomFileNames[i]) )
        {
          m_apUpperRomBlocks[i] = new CMemoryBlock( memoryProfile.sUpperRomFileNames[i], &romStream );
        }
        else
        {
          m_apUpperRomBlocks[i] = NULL;
          KMASSERTM( m_apUpperRomBlocks[i] != NULL, ("Could not load upper ROM block file '%s'.", memoryProfile.sUpperRomFileNames[i].c_str()) );
        }
      }
      else
      {
        // Put the same as Upper ROM 0 (usually Basic)
        m_apUpperRomBlocks[i] = m_apUpperRomBlocks[0];
      }
    }

    // Create the RAM blocks
    for (i = 0; i < memoryProfile.nRamBlockCount; i++)
    {
      char szLabel[10];
      _snprintf_s( szLabel, sizeof(szLabel), "RAM%d", i );
      m_apRamBlocks[i] = new CMemoryBlock( szLabel );
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CMemory::ResetVars()
  {
    unsigned i;

    m_pLowerRomBlock = NULL;

    for(i=0; i < UPPER_ROM_BLOCK_COUNT; i++)
    {
      m_apUpperRomBlocks[i] = NULL;
    }

    for(i=0; i < MAX_NUM_RAM_BLOCKS; i++)
    {
      m_apRamBlocks[i] = NULL;
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CMemory::FreeVars()
  {
    delete m_pLowerRomBlock;
    m_pLowerRomBlock = NULL;

    unsigned i;
    for (i = UPPER_ROM_BLOCK_COUNT-1; i < UPPER_ROM_BLOCK_COUNT; i--)   // Note: Delete from back to front
    {
      if ( (i == 0) || (m_apUpperRomBlocks[i] != m_apUpperRomBlocks[0]) )     // To avoid deleting Basic ROM multiple times
      {
        delete m_apUpperRomBlocks[i];
        m_apUpperRomBlocks[i] = NULL;
      }
    }

    for (i = 0; i < MAX_NUM_RAM_BLOCKS; i++)
    {
      delete m_apRamBlocks[i];
    }
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
  CMemoryBlock* CMemory::GetUpperRomBlock(cpcByte nIndex)
  {
    KMASSERT( (nIndex >= 0) && (nIndex < UPPER_ROM_BLOCK_COUNT) );
    return m_apUpperRomBlocks[nIndex];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMemoryBlock* CMemory::GetRamBlock(int i)
  {
    KMASSERT( (i >= 0) && (i < MAX_NUM_RAM_BLOCKS) );
    return m_apRamBlocks[i];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  const CMemoryBlock* CMemory::GetRamBlock(int i) const
  {
    KMASSERT( (i >= 0) && (i < MAX_NUM_RAM_BLOCKS) );
    return m_apRamBlocks[i];
  }

} //namespace CPC
