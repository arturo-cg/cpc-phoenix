//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCDSKDISK_H_
#define _CPCDSKDISK_H_


#include "cpcDisk.h"


namespace CPC {


  /**
  ** A CDisk-derived class that can read disk images in DSK format (only Extended DSK format for now, Standard DSK in the future maybe...).
  */
  class CDskDisk : public CDisk
  {
  public:

                            CDskDisk                  ();
    virtual                ~CDskDisk                  ()  { FreeVars(); }

    /** Loads data from the given disk image. */
    bool                    LoadImage                 (kmbInputStream* pStream);

    /** Returns the format of this disk image. */
    virtual EFormat         GetFormat                 () const  { return m_eFormat; }

    /** Returns how many sides the disk has. */
    virtual unsigned        GetSideCount              () const  { return (unsigned)m_diskHeader.nSideCount; }
    /** Returns how many tracks the disk has per side. */
    virtual unsigned        GetTrackCount             () const  { return (unsigned)m_diskHeader.nTrackCount; }


  private:

#pragma pack(push, Dsk_Structs_Pack_Section)
#pragma pack(1)

    struct SDskSectorInfo
    {
      cpcByte nTrack;              // Track number (equivalent to C parameter in NEC765 commands).
      cpcByte nSide;               // Side number (equivalent to H parameter in NEC765 commands).
      cpcByte nId;                 // Sector ID (equivalent to R parameter in NEC765 commands).
      cpcByte nSize;               // Sector size (equivalent to N parameter in NEC765 commands).
      cpcByte nStatusRegister1;    // FDC status register 1 (equivalent to NEC765 ST1 status register).
      cpcByte nStatusRegister2;    // FDC status register 2 (equivalent to NEC765 ST2 status register).
      cpcWord nDataLength;         // Actual data length in bytes (little endian notation). Special case: When N=6, only 1800h bytes are stored.
    };

    struct SDskTrackHeader
    {
      char szTag[13];              // Should contain "Track-Info\r\n".
      cpcByte _unused1[3];
      cpcByte nTrackNumber;
      cpcByte nSideNumber;
      cpcByte _unused2[2];
      cpcByte nSectorSize;
      cpcByte nSectorCount;
      cpcByte nGapLength;
      cpcByte nFillerByte;
    };

    struct SDskDiskHeader
    {
      char szTag[34];              // Should contain "EXTENDED CPC DSK File\r\nDisk-Info\r\n".
      char szCreator[14];          // Name of creator (utility/emulator).
      cpcByte nTrackCount;
      cpcByte nSideCount;
      cpcByte _unused[2];          // Used only in standard DSK format.
      cpcByte anTrackSizes[204];   // Each element n contains the high byte of track n length (equivalent to track length/256).
    };

#pragma pack(pop, Dsk_Structs_Pack_Section)

    struct SDskSector
    {
      SDskSectorInfo* pHeader;
      cpcByte*        pData;
    };

    struct SDskTrack
    {
      typedef std::vector<SDskSector> TDskSectorList;

      SDskTrackHeader* pHeader;
      TDskSectorList   lSectors;
    };


    typedef                 CDisk                     inherited;
    typedef                 std::vector<SDskTrack>    TTrackList;


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    const SDskTrack&        GetTrack                  (unsigned nSide, unsigned nTrack) const;

    bool                    ReadImage                 (kmbInputStream* pStream);
    void                    BuildTrackList            ();
    void                    BuildSectorList           (SDskTrack& track);


    EFormat                 m_eFormat;
    SDskDiskHeader          m_diskHeader;
    TTrackList              m_lTracks;
    cpcByte*                m_pRawData;

  };


} //namespace CPC

#endif // _CPCDSKDISK_H_
