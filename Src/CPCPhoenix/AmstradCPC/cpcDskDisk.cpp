//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcDskDisk.h"
#include "Stream/kmbInputStream.h"



namespace CPC {


  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CDskDisk::CDskDisk() : inherited()
  {
    // Reset members
    ResetVars();

    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CDskDisk::ResetVars()
  {
    m_eFormat                = FORMAT_INVALID;
    m_diskHeader.szTag[0]    = '\0';
    m_diskHeader.nSideCount  = 0;
    m_diskHeader.nTrackCount = 0;
    m_lTracks.clear();
    m_pRawData               = NULL;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CDskDisk::FreeVars()
  {
    m_lTracks.clear();
    delete [] m_pRawData;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  bool CDskDisk::LoadImage(kmbInputStream* pStream)
  {
    bool bRet = false;

    KMASSERTM( (pStream != NULL) && pStream->IsOk(), ("Invalid stream") );
    if ( (pStream != NULL) && pStream->IsOk() )
    {
      // Read the disk header
      if ( pStream->Read(&m_diskHeader) )
      {
        if (_mbsnbcmp((const unsigned char*)m_diskHeader.szTag, (const unsigned char*)"EXTENDED CPC DSK", 16/*char_count*/) == 0)
        {
          m_eFormat = FORMAT_EXTENDED_DSK;

          // Calculate total image length, excluding disk header
          unsigned nTotalLength = 0;
          cpcByte i;
          for (i = 0; i < (m_diskHeader.nTrackCount * m_diskHeader.nSideCount); i++)
          {
            nTotalLength += unsigned(m_diskHeader.anTrackSizes[i]) << 8;
          }

          // Allocate buffer for the image and load it
          m_pRawData = new cpcByte[nTotalLength];
          if ( pStream->Read(m_pRawData, nTotalLength) )
          {
            // Build track and sector lists, i.e. assign pointers to each track and sector in m_pRawData
            BuildTrackList();
            bRet = true;
          }
        }
        else
        {
          // ERROR - Unknown format
        }
      }
    }

    return bRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CDskDisk::BuildTrackList()
  {
    m_lTracks.clear();

    unsigned nTotalTrackCount;
    nTotalTrackCount = unsigned(m_diskHeader.nTrackCount) * unsigned(m_diskHeader.nSideCount);

    m_lTracks.reserve( nTotalTrackCount );

    cpcByte* pCurrOffset;
    pCurrOffset = m_pRawData;

    unsigned i;
    for (i = 0; i < nTotalTrackCount; i++)
    {
      // Register current track
      SDskTrack newTrack;
      if (m_diskHeader.anTrackSizes[i] > 0)
      {
        // The track exists
        newTrack.pHeader = (SDskTrackHeader*) pCurrOffset;
#ifdef KM_DEBUG_BUILD
        // Sanity check to make sure we are locating tracks correctly
        if (strcmp(newTrack.pHeader->szTag, "Track-Info\r\n") != 0)
        {
          KMASSERTM( false, ("A track has not been located correctly.") );
        }
#endif //KM_DEBUG_BUILD

        // Build sector list
        BuildSectorList( newTrack );
      }
      else
      {
        // The track does NOT exist
        newTrack.pHeader = NULL;
        newTrack.lSectors.clear();
      }
      m_lTracks.push_back( newTrack );

      // Advance pointer to next track
      pCurrOffset += unsigned(m_diskHeader.anTrackSizes[i]) << 8;
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CDskDisk::BuildSectorList(SDskTrack& track)
  {
    track.lSectors.reserve( track.pHeader->nSectorCount );

    SDskSectorInfo* pSectorInfos;
    pSectorInfos =  (SDskSectorInfo*) ( (cpcByte*)track.pHeader + sizeof(SDskTrackHeader) );

    cpcByte* pDataOffset;
    pDataOffset = (cpcByte*) &pSectorInfos[track.pHeader->nSectorCount];   // Sectors data go right after the last sector header.

    unsigned i;
    for (i = 0; i < track.pHeader->nSectorCount; i++)
    {
      SDskSector newSector;
      newSector.pHeader = &pSectorInfos[i];
      newSector.pData   = pDataOffset;
      track.lSectors.push_back( newSector );
      // Advance pointer to next sector data
      pDataOffset += unsigned(newSector.pHeader->nDataLength);
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  const CDskDisk::SDskTrack& CDskDisk::GetTrack(unsigned nSide, unsigned nTrack) const
  {
    KMASSERT( nSide < m_diskHeader.nSideCount );
    KMASSERT( nTrack < m_diskHeader.nTrackCount );

    unsigned nIndex;
    nIndex = (nTrack * m_diskHeader.nSideCount) + nTrack;

    return m_lTracks[nIndex];
  }

} //namespace CPC
