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

    static const int        MAX_NUM_RAM_BLOCKS    = 8;
    static const cpcByte    UPPER_ROM_BLOCK_COUNT = 16;


                            CMemory                   (CMachine *pMachine);
    virtual                ~CMemory                   ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Returns the lower ROM block. There is only one, and it is always the Operating System (or firmware).
    *** There are three different versions: v1 for the CPC 464, v2 for the CPC 664 and v3 for the CPC 6128. */
    CMemoryBlock*           GetLowerRomBlock          ()  { return m_pLowerRomBlock; }
    /** Returns the requested upper ROM block. At index 0 is the Basic ROM and always exists. On the CPC 664 and 6128,
    *** Amsdos ROM is at index 7. Other indices may contain another ROM or not. If they don't, Basic ROM is returned instead. */
    CMemoryBlock*           GetUpperRomBlock          (cpcByte nIndex);

    /** Returns the requested RAM block. The index can go from 0 to MAX_NUM_RAM_BLOCKS.
    *** The first four are the 64k primary RAM page, the last four the 64k secondary RAM page. */
    CMemoryBlock*           GetRamBlock               (int i);
    /** Returns the requested RAM block. The index can go from 0 to MAX_NUM_RAM_BLOCKS (const version).
    *** The first four are the 64k primary RAM page, the last four the 64k secondary RAM page. */
    const CMemoryBlock*     GetRamBlock               (int i) const;


  private:

    typedef                 CSubSystem                inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();


    /** The lower ROM block (operating system / firmware ROM). */
    CMemoryBlock*           m_pLowerRomBlock;
    /** The upper ROM blocks present in the machine. */
    CMemoryBlock*           m_apUpperRomBlocks[UPPER_ROM_BLOCK_COUNT];
    /** The RAM blocks present in the machine. */
    CMemoryBlock*           m_apRamBlocks[MAX_NUM_RAM_BLOCKS];

  };


} //namespace CPC

#endif // _CPCMEMORY_H_
