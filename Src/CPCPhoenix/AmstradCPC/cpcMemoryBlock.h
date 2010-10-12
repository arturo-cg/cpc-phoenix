//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCMEMORYBLOCK_H_
#define _CPCMEMORYBLOCK_H_


namespace CPC {


  /**
  ** Represents a 16K memory chunk, which is the smallest memory size
  ** the CPC hardware works with (at a macro level).
  ** @par
  ** This class is used for both RAM and ROM memories to avoid declaring
  ** Read and Write methods as virtual. This is not a problem since
  ** the CPC hardware makes impossible to write into ROM.
  */
  class CMemoryBlock
  {
  public:

    CMemoryBlock              ();
    virtual                ~CMemoryBlock              ()  { FreeVars(); }

    /** Reads a byte from this memory block.
    *** Note that bits 15,14 of nAddress are ignored. */
    cpcByte                 ReadByte                  (cpcWord nAddress) const;

    /** Writes a byte into this memory block.
    *** Note that bits 15,14 of nAddress are ignored. */
    void                    WriteByte                 (cpcWord nAddress, cpcByte nValue);


  private:

    enum
    {
      MEMORY_BLOCK_SIZE = 0x4000, /*16384 bytes = 16K*/
      LAST_BYTE         = MEMORY_BLOCK_SIZE - 1,
    };


    void                    ResetVars                 ();
    void                    FreeVars                  ();


    cpcByte                 m_anBytes[MEMORY_BLOCK_SIZE];

  };

} //namespace CPC

#endif // _CPCMEMORYBLOCK_H_
