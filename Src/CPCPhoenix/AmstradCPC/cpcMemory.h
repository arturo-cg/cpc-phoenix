//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCMEMORY_H_
#define _CPCMEMORY_H_


#include "cpcSubSystem.h"


namespace CPC {


  class CMemoryBlock;


  /**
  ** 
  */
  class CMemory : public CSubSystem
  {
  public:

    enum TRamConfig
    {
      RAM_CONFIG_0_1_2_3     = 0,
      RAM_CONFIG_0_1_2_3s    = 1,
      RAM_CONFIG_0s_1s_2s_3s = 2,
      RAM_CONFIG_0_3_2_3s    = 3,
      RAM_CONFIG_0_0s_2_3    = 4,
      RAM_CONFIG_0_1s_2_3    = 5,
      RAM_CONFIG_0_2s_2_3    = 6,
      RAM_CONFIG_0_3s_2_3    = 7,
    };


    CMemory                   (CMachine *pMachine);
    virtual                ~CMemory                   ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** We are notified that another subsytem is trying to write a byte to us.
    *** Usually it's the CPU through an OUT instruction. */
    virtual void            RespondToWritePortRequest (cpcWord nPort, cpcByte nValue);

    /** Sets the RAM configuration. */
    void                    SetRamConfiguration       (unsigned nSecondaryPage, TRamConfig eConfig);
    /** Sets the ROM visibility. */
    void                    SetRomVisibility          (bool bLowerRomVisible, bool bUpperRomVisible);

    /** Reads a byte from the specified address. */
    cpcByte                 ReadByte                  (cpcWord nAddress) const;
    /** Writes a byte in the specified address. */
    void                    WriteByte                 (cpcWord nAddress, cpcByte nValue);


  private:

    typedef                 CSubSystem                inherited;


    enum
    {
      MAX_NUM_ROM_BLOCKS = 3,
      MAX_NUM_RAM_BLOCKS = 8,
    };


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    void                    UpdateVisibleBlocks       ();


    /** The ROM blocks present in the machine. */
    CMemoryBlock           *m_apRomBlocks[MAX_NUM_ROM_BLOCKS];
    /** The RAM blocks present in the machine. */
    CMemoryBlock           *m_apRamBlocks[MAX_NUM_RAM_BLOCKS];

    /** The blocks that are visible for read operations (can be either ROM or RAM blocks). */
    CMemoryBlock           *m_apVisibleReadBlocks[4];
    /** The blocks that are visible for write operations (always RAM blocks). */
    CMemoryBlock           *m_apVisibleWriteBlocks[4];

    /** Which secondary 64K RAM page to use. */
    unsigned                m_nSecondaryPage;
    /** The RAM configuration. */
    TRamConfig              m_eRamConfig;

    /** Whether the lower ROM (Operating System ROM) is visible in the range &0000-&3FFF or not. */
    bool                    m_bLowerRomVisible;
    /** Whether the upper ROM (BASIC or expansion ROM) is visible in the range &C000-&FFFF or not. */
    bool                    m_bUpperRomVisible;

  };


} //namespace CPC

#endif // _CPCMEMORY_H_
