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

    static const int        MAX_NUM_RAM_BLOCKS = 8;

    enum ERomBlockIndex
    {
      ROMINDEX_OS     = 0,
      ROMINDEX_BASIC  = 1,
      ROMINDEX_AMSDOS = 2,

      MAX_NUM_ROM_BLOCKS
    };


                            CMemory                   (CMachine *pMachine);
    virtual                ~CMemory                   ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Returns the requested ROM block. */
    CMemoryBlock*           GetRomBlock               (ERomBlockIndex eIndex);
    /** Returns the requested ROM block (const version). */
    const CMemoryBlock*     GetRomBlock               (ERomBlockIndex eIndex) const;

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


    /** The ROM blocks present in the machine.
    *** Index 0: Operating System, index 1: BASIC, index 2: AMSDOS */
    CMemoryBlock           *m_apRomBlocks[MAX_NUM_ROM_BLOCKS];
    /** The RAM blocks present in the machine. */
    CMemoryBlock           *m_apRamBlocks[MAX_NUM_RAM_BLOCKS];

  };


} //namespace CPC

#endif // _CPCMEMORY_H_
