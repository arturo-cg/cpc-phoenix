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
    m_eFormat              = FORMAT_INVALID;
    m_diskInfo.szTag[0]    = '\0';
    m_diskInfo.nSideCount  = 0;
    m_diskInfo.nTrackCount = 0;
    m_lTracks.clear();
    m_pRawData             = NULL;
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
      if ( pStream->Read(&m_diskInfo) )
      {
        if (_mbsnbcmp((const unsigned char*)m_diskInfo.szTag, (const unsigned char*)"EXTENDED CPC DSK", 16/*char_count*/) == 0)
        {
          m_eFormat = FORMAT_EXTENDED_DSK;

          // Calculate total image length, excluding disk header
          unsigned nTotalLength = 0;
          cpcByte i;
          for (i = 0; i < (m_diskInfo.nTrackCount * m_diskInfo.nSideCount); i++)
          {
            nTotalLength += unsigned(m_diskInfo.anTrackSizes[i]) << 8;
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
    nTotalTrackCount = unsigned(m_diskInfo.nTrackCount) * unsigned(m_diskInfo.nSideCount);

    m_lTracks.reserve( nTotalTrackCount );

    cpcByte* pCurrOffset;
    pCurrOffset = m_pRawData;

    unsigned i;
    for (i = 0; i < nTotalTrackCount; i++)
    {
      // Register current track
      SDskTrack newTrack;
      if (m_diskInfo.anTrackSizes[i] > 0)
      {
        // The track exists
        newTrack.pInfo = (SDskTrackInfo*) pCurrOffset;

        // Build sector list
        BuildSectorList( newTrack );
      }
      else
      {
        // The track does NOT exist
        newTrack.pInfo = NULL;
        newTrack.lSectors.clear();
        newTrack.lIdsToIndex.clear();
      }
      m_lTracks.push_back( newTrack );

      // Advance pointer to next track
      pCurrOffset += unsigned(m_diskInfo.anTrackSizes[i]) << 8;
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CDskDisk::BuildSectorList(SDskTrack& track)
  {
    track.lSectors.clear();
    track.lIdsToIndex.clear();

    track.lSectors.reserve( track.pInfo->nSectorCount );

    SSectorInfo* pSectorInfos;
    pSectorInfos = (SSectorInfo*) ( (cpcByte*)track.pInfo + sizeof(SDskTrackInfo) );

    cpcByte* pDataOffset;
    pDataOffset = (cpcByte*)track.pInfo + 256;

    unsigned i;
    for (i = 0; i < track.pInfo->nSectorCount; i++)
    {
      SDskSector newSector;
      newSector.pInfo = &pSectorInfos[i];
      newSector.pData = pDataOffset;

      track.lSectors.push_back( newSector );
      track.lIdsToIndex.insert( SDskTrack::TIndexMap::value_type(newSector.pInfo->nId, i) );

      // Advance pointer to next sector data
      pDataOffset += unsigned(newSector.pInfo->nDataLength);
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ const CDskDisk::SDskTrackInfo* CDskDisk::GetTrackInfo(unsigned nSide, unsigned nTrack) const
  {
    const SDskTrackInfo* pRet = NULL;

    const SDskTrack* pTrack;
    pTrack = GetTrack( nSide, nTrack );
    if (pTrack != NULL)
    {
      pRet = pTrack->pInfo;
    }

    return pRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ const CDisk::SSectorInfo* CDskDisk::GetSectorInfo(unsigned nSide, unsigned nTrack, unsigned nSector) const
  {
    const SSectorInfo* pRet = NULL;

    const SDskTrack* pTrack;
    pTrack = GetTrack( nSide, nTrack );
    if (pTrack != NULL)
    {
      if (nSector < pTrack->pInfo->nSectorCount)
      {
        pRet = pTrack->lSectors[nSector].pInfo;
      }
    }

    return pRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ const CDisk::SSectorInfo* CDskDisk::GetSectorInfoById(unsigned nSide, unsigned nTrack, unsigned nSectorId) const
  {
    const SSectorInfo* pRet = NULL;

    const SDskTrack* pTrack;
    pTrack = GetTrack( nSide, nTrack );
    if (pTrack != NULL)
    {
      SDskTrack::TIndexMap::const_iterator iter;
      iter = pTrack->lIdsToIndex.find( nSectorId );
      pRet = ( iter!=pTrack->lIdsToIndex.end() ? pTrack->lSectors[iter->second].pInfo : NULL );
    }

    return pRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ const cpcByte* CDskDisk::GetSectorData(unsigned nSide, unsigned nTrack, unsigned nSector) const
  {
    const cpcByte* pRet = NULL;

    const SDskTrack* pTrack;
    pTrack = GetTrack( nSide, nTrack );
    if (pTrack != NULL)
    {
      if (nSector < pTrack->pInfo->nSectorCount)
      {
        pRet = pTrack->lSectors[nSector].pData;
      }
    }

    return pRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ const cpcByte* CDskDisk::GetSectorDataById(unsigned nSide, unsigned nTrack, unsigned nSectorId) const
  {
    const cpcByte* pRet = NULL;

    const SDskTrack* pTrack;
    pTrack = GetTrack( nSide, nTrack );
    if (pTrack != NULL)
    {
      SDskTrack::TIndexMap::const_iterator iter;
      iter = pTrack->lIdsToIndex.find( nSectorId );
      pRet = ( iter!=pTrack->lIdsToIndex.end() ? pTrack->lSectors[iter->second].pData : NULL );
    }

    return pRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  const CDskDisk::SDskTrack* CDskDisk::GetTrack(unsigned nSide, unsigned nTrack) const
  {
    const SDskTrack* pRet = NULL;
    if ( (nSide < m_diskInfo.nSideCount) && (nTrack < m_diskInfo.nTrackCount) )
    {
      unsigned nIndex;
      nIndex = (nTrack * m_diskInfo.nSideCount) + nSide;
      pRet = &m_lTracks[nIndex];
    }
    return pRet;
  }

} //namespace CPC
