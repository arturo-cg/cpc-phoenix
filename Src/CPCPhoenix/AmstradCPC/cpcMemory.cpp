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
    unsigned nRamBlockCount;                              // How many 16Kb RAM blocks are present. It must be 4 (64Kb) or 8 (128Kb).

    bool     bHasAmsdosRom;                               // Whether the AMSDOS ROM is present or not.
    string   sRomFileNames[CMemory::MAX_NUM_ROM_BLOCKS];  // Files that contain each ROM content.
  };

  static SMemoryProfile s_aMemoryProfiles[CMachine::MODEL_LAST] =
  {
    { 4/*64Kb RAM*/,  false, { "OS_464.ROM",  "BASIC_464.ROM",  ""                } },    // MODEL_464
    { 4/*64Kb RAM*/,  true,  { "OS_664.ROM",  "BASIC_664.ROM",  "AMSDOS_664.ROM"  } },    // MODEL_664
    { 8/*128Kb RAM*/, true,  { "OS_6128.ROM", "BASIC_6128.ROM", "AMSDOS_6128.ROM" } },    // MODEL_6128
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

    // Create the ROM blocks
    kmbFileInputStream romStream;
    unsigned i;
    for (i = 0; i < MAX_NUM_ROM_BLOCKS; i++)
    {
      ERomBlockIndex eRomIndex;
      eRomIndex = (ERomBlockIndex) i;
      if ( (eRomIndex != ROMINDEX_AMSDOS) || memoryProfile.bHasAmsdosRom )
      {
        if ( romStream.Init("Roms/" + memoryProfile.sRomFileNames[eRomIndex]) )
        {
          m_apRomBlocks[eRomIndex] = new CMemoryBlock( &romStream );
        }
        else
        {
          m_apRomBlocks[eRomIndex] = NULL;
        }
      }
    }

    // Create the RAM blocks
    for (i = 0; i < memoryProfile.nRamBlockCount; i++)
    {
      m_apRamBlocks[i] = new CMemoryBlock();
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CMemory::ResetVars()
  {
    unsigned i;

    for(i=0; i < MAX_NUM_ROM_BLOCKS; i++)
    {
      m_apRomBlocks[i] = NULL;
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
    unsigned i;

    for(i=0; i < MAX_NUM_ROM_BLOCKS; i++)
    {
      delete m_apRomBlocks[i];
    }

    for(i=0; i < MAX_NUM_RAM_BLOCKS; i++)
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
  CMemoryBlock* CMemory::GetRomBlock(ERomBlockIndex eIndex)
  {
    KMASSERT( (eIndex >= 0) && (eIndex < MAX_NUM_ROM_BLOCKS) );
    return m_apRomBlocks[eIndex];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  const CMemoryBlock* CMemory::GetRomBlock(ERomBlockIndex eIndex) const
  {
    KMASSERT( (eIndex >= 0) && (eIndex < MAX_NUM_ROM_BLOCKS) );
    return m_apRomBlocks[eIndex];
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
