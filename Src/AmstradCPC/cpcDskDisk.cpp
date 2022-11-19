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
        m_eFormat = FORMAT_INVALID;
        m_diskInfo.szTag[0] = '\0';
        m_diskInfo.nSideCount = 0;
        m_diskInfo.nTrackCount = 0;
        m_lTracks.clear();
        m_pRawData = NULL;
        m_dataSelector = 0;
    }

    //----------------------------------------------------------------------------
    /**
    ** FreeVars
    */
    void CDskDisk::FreeVars()
    {
        m_lTracks.clear();
        delete[] m_pRawData;
    }

    //----------------------------------------------------------------------------
    /**
    ** FreeVars
    */
    bool CDskDisk::LoadImageFromStream(kmbInputStream* pStream)
    {
        bool bRet = false;

        KMASSERTM((pStream != NULL) && pStream->IsOk(), ("Invalid stream"));
        if ((pStream != NULL) && pStream->IsOk())
        {
            // Read the disk header
            if (pStream->Read(&m_diskInfo))
            {
                unsigned nTotalLength = 0;

                if (strncmp(m_diskInfo.szTag, "EXTENDED CPC DSK", 16/*char_count*/) == 0)
                {
                    m_eFormat = FORMAT_EXTENDED_DSK;

                    // Calculate total image length, excluding disk header
                    cpcByte i;
                    for (i = 0; i < (m_diskInfo.nTrackCount * m_diskInfo.nSideCount); i++)
                    {
                        nTotalLength += unsigned(m_diskInfo.anTrackSizes[i]) << 8;
                    }
                }
                else if (strncmp(m_diskInfo.szTag, "MV - CPC", 8/*char_count*/) == 0)
                {
                    m_eFormat = FORMAT_STANDARD_DSK;
                    nTotalLength = m_diskInfo.nTrackCount * m_diskInfo.nSideCount * m_diskInfo.nTrackSize;
                }
                else
                {
                    // ERROR - Unknown format
                    m_eFormat = FORMAT_INVALID;
                }

                if (m_eFormat != FORMAT_INVALID)
                {
                    // Allocate buffer for the image and load it
                    m_pRawData = new cpcByte[nTotalLength];
                    if (pStream->Read(m_pRawData, nTotalLength))
                    {
                        // Build track and sector lists, i.e. assign pointers to each track and sector in m_pRawData
                        BuildTrackList();
                        bRet = true;
                    }
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

        m_lTracks.reserve(nTotalTrackCount);

        cpcByte* pCurrOffset;
        pCurrOffset = m_pRawData;

        unsigned i;
        for (i = 0; i < nTotalTrackCount; i++)
        {
            // Register current track
            SDskTrack newTrack;
            newTrack.pInfo = NULL;
            newTrack.lSectors.clear();
            newTrack.lIdsToIndex.clear();

            if ((m_eFormat == FORMAT_STANDARD_DSK) ||                                        // If it's standard format...
                ((m_eFormat == FORMAT_EXTENDED_DSK) && (m_diskInfo.anTrackSizes[i] > 0)))   // If it's extended format and track size is non-zero...
            {
                // The track exists
                newTrack.pInfo = (SDskTrackInfo*)pCurrOffset;

                // Build sector list
                BuildSectorList(newTrack);
            }
            else
            {
                // The track does NOT exist
            }

            // Advance pointer to next track
            pCurrOffset += (m_eFormat == FORMAT_STANDARD_DSK ? unsigned(m_diskInfo.nTrackSize) :
                                                               unsigned(m_diskInfo.anTrackSizes[i]) << 8);

            m_lTracks.push_back(newTrack);
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

        track.lSectors.reserve(track.pInfo->nSectorCount);

        SSectorInfo* sectorInfos = (SSectorInfo*)((cpcByte*)track.pInfo + sizeof(SDskTrackInfo));
        uint8_t* sectorData = (uint8_t*)track.pInfo + 256;

        unsigned i;
        for (i = 0; i < track.pInfo->nSectorCount; i++)
        {
            SDskSector newSector;
            newSector.pInfo = &sectorInfos[i];
            newSector.pData = sectorData;

            // Determine sector data length and number of copies.
            unsigned N = (newSector.pInfo->nSize & 0x07);
            newSector.singleDataLength = 256 << (N - 1);
            unsigned totalDataLength = (m_eFormat == FORMAT_EXTENDED_DSK ? newSector.pInfo->nDataLength : newSector.singleDataLength);
            newSector.numDatas = totalDataLength / newSector.singleDataLength;

            track.lSectors.push_back(newSector);
            track.lIdsToIndex.insert(SDskTrack::TIndexMap::value_type(newSector.pInfo->nId, i));

            // Advance pointer to next sector data.
            sectorData += totalDataLength;
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ const CDskDisk::SDskTrackInfo* CDskDisk::GetTrackInfo(unsigned sideIndex, unsigned trackIndex) const
    {
        const SDskTrackInfo* ret = nullptr;

        const SDskTrack* track;
        track = GetDskTrack(sideIndex, trackIndex);
        if (track != nullptr)
        {
            ret = track->pInfo;
        }

        return ret;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ unsigned CDskDisk::GetSectorCount(unsigned sideNumber, unsigned trackNumber) const
    {
        unsigned ret = 0;

        const SDskTrack* track = GetDskTrack(sideNumber, trackNumber);
        if (track != nullptr)
        {
            ret = track->lSectors.size();
        }

        return ret;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ const CDisk::SSectorInfo* CDskDisk::GetSectorInfo(unsigned sideIndex, unsigned trackIndex, unsigned sectorIndex) const
    {
        const SSectorInfo* ret = nullptr;

        const SDskTrack* track;
        track = GetDskTrack(sideIndex, trackIndex);
        if (track != nullptr)
        {
            if (sectorIndex < track->pInfo->nSectorCount)
            {
                ret = track->lSectors[sectorIndex].pInfo;
            }
        }

        return ret;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ const CDisk::SSectorInfo* CDskDisk::GetSectorInfoById(unsigned sideIndex, unsigned trackIndex, unsigned sectorId) const
    {
        const SSectorInfo* ret = nullptr;

        const SDskTrack* track;
        track = GetDskTrack(sideIndex, trackIndex);
        if (track != nullptr)
        {
            SDskTrack::TIndexMap::const_iterator iter;
            iter = track->lIdsToIndex.find(sectorId);
            ret = (iter != track->lIdsToIndex.end() ? track->lSectors[iter->second].pInfo : nullptr);
        }

        return ret;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ const cpcByte* CDskDisk::GetSectorData(unsigned sideIndex, unsigned trackIndex, unsigned sectorIndex) const
    {
        const cpcByte* ret = nullptr;

        const SDskTrack* track;
        track = GetDskTrack(sideIndex, trackIndex);
        if (track != nullptr)
        {
            if (sectorIndex < track->pInfo->nSectorCount)
            {
                const SDskSector& sector = track->lSectors[sectorIndex];
                ret = GetCopyOfSectorData(sector);
            }
        }

        return ret;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ const cpcByte* CDskDisk::GetSectorDataById(unsigned sideIndex, unsigned trackIndex, unsigned sectorId) const
    {
        const cpcByte* ret = nullptr;

        const SDskTrack* track;
        track = GetDskTrack(sideIndex, trackIndex);
        if (track != nullptr)
        {
            SDskTrack::TIndexMap::const_iterator iter;
            iter = track->lIdsToIndex.find(sectorId);
            if (iter != track->lIdsToIndex.end())
            {
                const SDskSector& sector = track->lSectors[iter->second];
                ret = GetCopyOfSectorData(sector);
            }
        }

        return ret;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    const CDskDisk::SDskTrack* CDskDisk::GetDskTrack(unsigned sideIndex, unsigned trackIndex) const
    {
        const SDskTrack* ret = nullptr;
        if ((sideIndex < m_diskInfo.nSideCount) && (trackIndex < m_diskInfo.nTrackCount))
        {
            unsigned index = (trackIndex * m_diskInfo.nSideCount) + sideIndex;
            ret = &m_lTracks[index];
        }
        return ret;
    }

    ////----------------------------------------------------------------------------
    ///**
    //**
    //*/
    //const CDskDisk::SDskSector* CDskDisk::GetDskSectorByIndex(unsigned sideIndex, unsigned trackIndex, unsigned sectorIndex) const
    //{
    //    const SDskSector* ret = nullptr;

    //    const SDskTrack* track;
    //    track = GetDskTrack(sideIndex, trackIndex);
    //    if (track != nullptr)
    //    {
    //        if (sectorIndex < track->pInfo->nSectorCount)
    //        {
    //            ret = &track->lSectors[sectorIndex];
    //        }
    //    }

    //    return ret;
    //}

    //----------------------------------------------------------------------------
    /**
    **
    */
    const cpcByte* CDskDisk::GetCopyOfSectorData(const CDskDisk::SDskSector& sector) const
    {
        unsigned dataIndex = m_dataSelector % sector.numDatas;
        cpcByte* ret = sector.pData + (sector.singleDataLength * dataIndex);

        // Ugly hack to be able to modify m_dataSelector from a const method.
        CDskDisk* nonConstThis = const_cast<CDskDisk*>(this);
        nonConstThis->m_dataSelector++;

        return ret;
    }

} //namespace CPC
