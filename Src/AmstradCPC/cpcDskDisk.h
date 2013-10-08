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

#pragma pack(push, Structs_Pack_Section)
#pragma pack(1)

    struct SDskTrackInfo
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

    struct SDskDiskInfo
    {
      char szTag[34];              // Should contain "EXTENDED CPC DSK File\r\nDisk-Info\r\n".
      char szCreator[14];          // Name of creator (utility/emulator).
      cpcByte nTrackCount;
      cpcByte nSideCount;
      cpcWord nTrackSize;          // [Standard only] Track size. All tracks are this size.
      cpcByte anTrackSizes[204];   // [Extended only] Each element n contains the high byte of track n length (equivalent to track length/256).
    };

#pragma pack(pop, Structs_Pack_Section)


                            CDskDisk                  ();
    virtual                ~CDskDisk                  ()  { FreeVars(); }

    /** Loads data from the given disk image. */
    bool                    LoadImageFromStream       (kmbInputStream* pStream);

    /** Returns the format of this disk image. */
    virtual EFormat         GetFormat                 () const  { return m_eFormat; }

    /** Returns how many sides the disk has. */
    virtual unsigned        GetSideCount              () const  { return (unsigned)m_diskInfo.nSideCount; }
    /** Returns how many tracks the disk has per side. */
    virtual unsigned        GetTrackCount             () const  { return (unsigned)m_diskInfo.nTrackCount; }

    /** Returns information about the disk. */
    const SDskDiskInfo*     GetDiskInfo               () const  { return &m_diskInfo; }
    /** Returns information about a specific track. */
    const SDskTrackInfo*    GetTrackInfo              (unsigned nSide, unsigned nTrack) const;
    /** Returns information about a sector given its index. */
    virtual const SSectorInfo* GetSectorInfo          (unsigned nSide, unsigned nTrack, unsigned nSector) const;
    /** Returns information about a sector given its ID. */
    virtual const SSectorInfo* GetSectorInfoById      (unsigned nSide, unsigned nTrack, unsigned nSectorId) const;
    /** Returns the data of a sector given its index. */
    virtual const cpcByte*  GetSectorData             (unsigned nSide, unsigned nTrack, unsigned nSector) const;
    /** Returns the data of a sector given its ID. */
    virtual const cpcByte*  GetSectorDataById         (unsigned nSide, unsigned nTrack, unsigned nSectorId) const;


  private:

    struct SDskSector
    {
      SSectorInfo* pInfo;
      cpcByte*     pData;
    };

    struct SDskTrack
    {
      typedef std::vector<SDskSector> TDskSectorList;
      typedef std::map<unsigned, unsigned> TIndexMap;

      SDskTrackInfo* pInfo;
      TDskSectorList lSectors;
      TIndexMap      lIdsToIndex;
    };


    typedef                 CDisk                     inherited;
    typedef                 std::vector<SDskTrack>    TTrackList;


    void                    ResetVars                 ();
    void                    FreeVars                  ();

    const SDskTrack*        GetTrack                  (unsigned nSide, unsigned nTrack) const;

    bool                    ReadImage                 (kmbInputStream* pStream);
    void                    BuildTrackList            ();
    void                    BuildSectorList           (SDskTrack& track);


    EFormat                 m_eFormat;
    SDskDiskInfo            m_diskInfo;
    TTrackList              m_lTracks;
    cpcByte*                m_pRawData;

  };


} //namespace CPC

#endif // _CPCDSKDISK_H_
