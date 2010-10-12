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


  struct TRamConfigItem
  {
    unsigned nBlockIndex;
    bool     bFromSecondaryPage;
  };

  struct TRamConfigEntry
  {
    TRamConfigItem Range[4];
  };

  static TRamConfigEntry s_aRamConfigTable[] =
  {
    // RAM_CONFIG_0_1_2_3
    { 0, false, 1, false, 2, false, 3, false },
    // RAM_CONFIG_0_1_2_3s
    { 0, false, 1, false, 2, false, 3, true  },
    // RAM_CONFIG_0s_1s_2s_3s
    { 0, true , 1, true , 2, true , 3, true  },
    // RAM_CONFIG_0_3_2_3s
    { 0, false, 3, false, 2, false, 3, true  },
    // RAM_CONFIG_0_0s_2_3
    { 0, false, 0, true , 2, false, 3, false },
    // RAM_CONFIG_0_1s_2_3
    { 0, false, 1, true , 2, false, 3, false },
    // RAM_CONFIG_0_2s_2_3
    { 0, false, 2, true , 2, false, 3, false },
    // RAM_CONFIG_0_3s_2_3
    { 0, false, 3, true , 2, false, 3, false },
  };



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

    // Determine visible read/write blocks
    UpdateVisibleBlocks();
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

    m_nSecondaryPage   = 0;
    m_eRamConfig       = RAM_CONFIG_0_1_2_3;
    m_bLowerRomVisible = true;
    m_bUpperRomVisible = false;
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
    m_nSecondaryPage   = 0;
    m_eRamConfig       = RAM_CONFIG_0_1_2_3;
    m_bLowerRomVisible = true;
    m_bUpperRomVisible = false;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMemory::RespondToWritePortRequest(cpcWord nPort, cpcByte nValue)
  {
    //
    // RAM configuration port --> Bit 15 == 0
    // ROM Select port --> Bit 13 == 0
    //

    // RAM configuration port?
    if( !(nPort & 0x8000) )      // If bit 15 is cleared...
    {
      if( ((nValue & 0xC0) >> 6) == 4)
      {
        // Bits 2-0 define one of the eight possible RAM configurations
        // Note: If we wanted to emulate expansion RAMs other than the CPC6128 built-in one, we would have
        //       to look into bits 4,3 which contain the secondary 64k page to use.
        GetMachine()->GetMemory()->SetRamConfiguration( 0/*nSecondaryPage*/, (TRamConfig) (nValue&0x03) );
      }
    }

    if( !(nPort & 0x2000) )    // If bit 13 is cleared...
    {
      // This selects the upper ROM in use (range &C000-&FFFF), but it still needs to be made visible through the Gate Array.
      // Lower ROM (range &0000-&3fff) cannot be changed, OS ROM is the only choice.

      // If nValue == 0 --> BASIC ROM
      // If nValue == 7 --> AMSDOS ROM
      // If nValue == other --> BASIC ROM
      //***************************** TODO - TODO - TODO *************************************
      //***************************** TODO - TODO - TODO *************************************
      //        SelectUpperRom( XXXXXXXXXXXXXXXXXX );
      //***************************** TODO - TODO - TODO *************************************
      //***************************** TODO - TODO - TODO *************************************
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMemory::SetRamConfiguration(unsigned nSecondaryPage, TRamConfig eConfig)
  {
    m_nSecondaryPage = nSecondaryPage;
    m_eRamConfig     = eConfig;

    UpdateVisibleBlocks();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMemory::SetRomVisibility(bool bLowerRomVisible, bool bUpperRomVisible)
  {
    m_bLowerRomVisible = bLowerRomVisible;
    m_bUpperRomVisible = bUpperRomVisible;

    UpdateVisibleBlocks();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMemory::UpdateVisibleBlocks()
  {
    TRamConfigEntry &config = s_aRamConfigTable[ m_eRamConfig ];
    unsigned         i;

    // Write blocks
    for(i=0; i < 4; i++)
    {
      if( !config.Range[i].bFromSecondaryPage )
      {
        // From primary page
        m_apVisibleWriteBlocks[i] = m_apRamBlocks[ config.Range[i].nBlockIndex ];
      }
      else
      {
        // From secondary page
        m_apVisibleWriteBlocks[i] = m_apRamBlocks[ config.Range[i].nBlockIndex + (4/* * m_nSecondaryPage*/) ];
      }
    }

    // Read blocks
    m_apVisibleReadBlocks[0] = (m_bLowerRomVisible ? m_apRomBlocks[0] : m_apVisibleWriteBlocks[0]);
    m_apVisibleReadBlocks[1] = m_apVisibleWriteBlocks[1];
    m_apVisibleReadBlocks[2] = m_apVisibleWriteBlocks[2];
    m_apVisibleReadBlocks[3] = (m_bUpperRomVisible ? m_apRomBlocks[1] : m_apVisibleWriteBlocks[3]);
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CMemory::ReadByte(cpcWord nAddress) const
  {
    // Bits 15,14 of nAddress determine which one of the four visible blocks to use
    // Bits 13-0 of nAddress determine the offset into the selected block
    return m_apVisibleReadBlocks[GET_ADDRESS_BLOCK(nAddress)]->ReadByte( GET_ADDRESS_OFFSET(nAddress) );
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMemory::WriteByte(cpcWord nAddress, cpcByte nValue)
  {
    // Bits 15,14 of nAddress determine which one of the four visible blocks to use
    // Bits 13-0 of nAddress determine the offset into the selected block
    m_apVisibleWriteBlocks[GET_ADDRESS_BLOCK(nAddress)]->WriteByte( GET_ADDRESS_OFFSET(nAddress), nValue );
  }

} //namespace CPC
