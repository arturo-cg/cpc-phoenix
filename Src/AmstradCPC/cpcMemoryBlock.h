//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCMEMORYBLOCK_H_
#define _CPCMEMORYBLOCK_H_


class kmbInputStream;


namespace CPC {


  /**
  ** Represents a 16K memory chunk, which is the smallest memory size
  ** the CPC hardware works with (at a macro level).
  ** @par
  ** This class is used for both RAM and ROM memories. This is not a
  ** problem since the CPC hardware makes impossible to write into ROM.
  */
  class CMemoryBlock
  {
  public:

                            CMemoryBlock              (const string& sLabel, kmbInputStream* pContentStream = NULL);
    virtual                ~CMemoryBlock              ()  { FreeVars(); }

    /** Returns the memory block label. The label is only used as a debug helper, it is not used in any way by the simulation. */
    const string&           GetLabel                  () const  { return m_sLabel; }

    /** Reads a byte from this memory block.
    *** Valid range is &0000-&4000. Note that bits 15,14 of nAddress are ignored. */
    cpcByte                 ReadByte                  (cpcWord nAddress) const            { return m_anBytes[nAddress & LAST_BYTE]; }

    /** Writes a byte into this memory block.
    *** Valid range is &0000-&4000. Note that bits 15,14 of nAddress are ignored. */
    void                    WriteByte                 (cpcWord nAddress, cpcByte nValue)  { m_anBytes[nAddress & LAST_BYTE] = nValue; }

    /** Fills the whole memory block with the provided content.
    *** This is used primarily for ROM blocks to write their content on creation, but it could be used in the future for a save state feature. */
    bool                    FillContent               (kmbInputStream* pContentStream);


  private:

    static const unsigned   MEMORY_BLOCK_LENGTH = 0x4000; /*16384 bytes = 16K*/
    static const unsigned   LAST_BYTE           = MEMORY_BLOCK_LENGTH - 1;


    void                    ResetVars                 ();
    void                    FreeVars                  ();


    string                  m_sLabel;
    cpcByte                 m_anBytes[MEMORY_BLOCK_LENGTH];

  };

} //namespace CPC

#endif // _CPCMEMORYBLOCK_H_
