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
    // ROM Select port --> Bit 13 set to 0
    //

    if( !(nPort & 0x2000) )
    {
        // If nValue == 0 --> BASIC ROM
        // If nValue == 7 --> AMSDOS ROM
        // If nValue == other --> BASIC ROM
//***************************** TODO - TODO - TODO *************************************
//***************************** TODO - TODO - TODO *************************************
//        SetCurrentExpansionRom( XXXXXXXXXXXXXXXXXX );
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
