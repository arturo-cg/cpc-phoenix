//-------------------------------------------------------------------------------------------
// File:        Memory.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcMemory.h"
#include "cpcMemoryBlock.h"
#include "cpcMachine.h"


#define GET_ADDRESS_BLOCK(addr)   ((addr & 0xC000) >> 14)
#define GET_ADDRESS_OFFSET(addr)  (addr & 0x3FFF)


namespace CPC {


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMemory::CMemory(CMachine *pMachine) : inherited( pMachine )
  {
    unsigned nNumRomBlocks;
    unsigned nNumRamBlocks;
    unsigned i;

    // Reset members
    ResetVars();

    // Determine number of RAM and ROM blocks
    switch( pMachine->GetModel() )
    {
    case CMachine::CPC_464:
      nNumRomBlocks = 2;
      nNumRamBlocks = 4;
      break;

    case CMachine::CPC_664:
      nNumRomBlocks = 3;
      nNumRamBlocks = 4;
      break;

    case CMachine::CPC_6128:
      nNumRomBlocks = 3;
      nNumRamBlocks = 8;
      break;

    default:
      ASSERT( false );
      nNumRomBlocks = 0;
      nNumRamBlocks = 0;
    }

    // Create the ROM blocks
    for(i=0; i < nNumRomBlocks; i++)
    {
      m_apRomBlocks[i] = new CMemoryBlock();
    }

    // Create the RAM blocks
    for(i=0; i < nNumRamBlocks; i++)
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
    ASSERT( (eIndex >= 0) && (eIndex < MAX_NUM_ROM_BLOCKS) );
    return m_apRomBlocks[eIndex];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  const CMemoryBlock* CMemory::GetRomBlock(ERomBlockIndex eIndex) const
  {
    ASSERT( (eIndex >= 0) && (eIndex < MAX_NUM_ROM_BLOCKS) );
    return m_apRomBlocks[eIndex];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMemoryBlock* CMemory::GetRamBlock(int i)
  {
    ASSERT( (i >= 0) && (i < MAX_NUM_RAM_BLOCKS) );
    return m_apRamBlocks[i];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  const CMemoryBlock* CMemory::GetRamBlock(int i) const
  {
    ASSERT( (i >= 0) && (i < MAX_NUM_RAM_BLOCKS) );
    return m_apRamBlocks[i];
  }

} //namespace CPC
