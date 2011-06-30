//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCDISK_H_
#define _CPCDISK_H_


class kmbInputStream;


namespace CPC {


  /**
  ** Base class for disks. A CDisk contains raw data of a CPC disk and can be used by the emulator to read data from (no writes yet).
  ** Derived classes load data from disk images in specific formats like DSK.
  */
  class CDisk
  {
  public:

    enum EFormat
    {
      FORMAT_STANDARD_DSK = 0,
      FORMAT_EXTENDED_DSK,
      /*** Add more here ***/

      FORMAT_LAST,
      FORMAT_INVALID = 0x7FFFFFFF
    };

#pragma pack(push, Structs_Pack_Section)
#pragma pack(1)

    struct SSectorInfo
    {
      cpcByte nTrack;              // Track number (equivalent to C parameter in NEC765 commands).
      cpcByte nSide;               // Side number (equivalent to H parameter in NEC765 commands).
      cpcByte nId;                 // Sector ID (equivalent to R parameter in NEC765 commands).
      cpcByte nSize;               // Sector size (equivalent to N parameter in NEC765 commands).
      cpcByte nStatusRegister1;    // FDC status register 1 (equivalent to NEC765 ST1 status register).
      cpcByte nStatusRegister2;    // FDC status register 2 (equivalent to NEC765 ST2 status register).
      cpcWord nDataLength;         // [Extended only] Actual data length in bytes (little endian notation). Special case: When N=6, only 1800h bytes are stored.
    };

#pragma pack(pop, Structs_Pack_Section)


    virtual                ~CDisk                     ()  { FreeVars(); }

    /** Returns the format of this disk image. */
    virtual EFormat         GetFormat                 () const = 0;

    /** Returns how many sides the disk has. */
    virtual unsigned        GetSideCount              () const = 0;
    /** Returns how many tracks the disk has. */
    virtual unsigned        GetTrackCount             () const = 0;

    /** Returns information about a specific sector. */
    virtual const SSectorInfo* GetSectorInfo          (unsigned nSide, unsigned nTrack, unsigned nSector) const = 0;
    /** Returns information about a sector given its ID. */
    virtual const SSectorInfo* GetSectorInfoById      (unsigned nSide, unsigned nTrack, unsigned nSectorId) const = 0;
    /** Returns the data of a specific sector. */
    virtual const cpcByte*  GetSectorData             (unsigned nSide, unsigned nTrack, unsigned nSector) const = 0;
    /** Returns the data of a sector given its ID. */
    virtual const cpcByte*  GetSectorDataById         (unsigned nSide, unsigned nTrack, unsigned nSectorId) const = 0;


  protected:

    // Protected constructor
                            CDisk                     ();


  private:

    void                    ResetVars                 ();
    void                    FreeVars                  ();


  };


} //namespace CPC

#endif // _CPCDISK_H_
