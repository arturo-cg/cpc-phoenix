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
    virtual unsigned        GetSideCount              () const  { return (unsigned)m_diskInfo.nSideCount; }
    /** Returns how many tracks the disk has per side. */
    virtual unsigned        GetTrackCount             () const  { return (unsigned)m_diskInfo.nTrackCount; }

    /** Returns information about the disk. */
    virtual const SDiskInfo* GetDiskInfo              () const  { return &m_diskInfo; }
    /** Returns information about a specific track. */
    virtual const STrackInfo* GetTrackInfo            (unsigned nSide, unsigned nTrack) const;
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

      STrackInfo*    pInfo;
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
    SDiskInfo               m_diskInfo;
    TTrackList              m_lTracks;
    cpcByte*                m_pRawData;

  };


} //namespace CPC

#endif // _CPCDSKDISK_H_
