//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#pragma once

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


        CDskDisk();
        virtual ~CDskDisk() { FreeVars(); }

        /** Loads data from the given disk image. */
        bool LoadImageFromStream(kmbInputStream* pStream);

        /** Returns the format of this disk image. */
        virtual EFormat GetFormat() const override { return m_eFormat; }

        /** Returns how many sides the disk has. */
        virtual unsigned GetSideCount() const override { return (unsigned)m_diskInfo.nSideCount; }
        /** Returns how many tracks the disk has per side. */
        virtual unsigned GetTrackCount() const override { return (unsigned)m_diskInfo.nTrackCount; }

        /** Returns information about the disk. */
        const SDskDiskInfo* GetDiskInfo() const { return &m_diskInfo; }
        /** Returns information about a specific track. */
        const SDskTrackInfo* GetTrackInfo(unsigned sideIndex, unsigned trackIndex) const;
        /** Returns how many sectors the given track has. */
        virtual unsigned GetSectorCount(unsigned sideIndex, unsigned trackIndex) const override;
        /** Returns information about a sector given its index. */
        virtual const SSectorInfo* GetSectorInfo(unsigned sideIndex, unsigned trackIndex, unsigned sectorIndex) const override;
        /** Returns information about a sector given its ID. */
        virtual const SSectorInfo* GetSectorInfoById(unsigned sideIndex, unsigned trackIndex, unsigned sectorId) const override;
        /** Returns the data of a sector given its index. */
        virtual const cpcByte* GetSectorData(unsigned sideIndex, unsigned trackIndex, unsigned sectorIndex) const override;
        /** Returns the data of a sector given its ID. */
        virtual const cpcByte* GetSectorDataById(unsigned sideIndex, unsigned trackIndex, unsigned sectorId) const override;


    private:

        struct SDskSector
        {
            SSectorInfo* pInfo;
            cpcByte* pData;
            unsigned numDatas;          // For a normal sector: 1. For a weak/random sector: may be > 1; pData points at the first copy, the remaining copies are stored one after another. A copy is selected randomly on each read.
            unsigned singleDataLength;  // Size in bytes of a single copy of the data. Total data length is dataLength * numDatas.

            //bool IsWeak() const { return numDatas > 1; }
        };

        struct SDskTrack
        {
            using TDskSectorList = std::vector<SDskSector>;
            using TIndexMap = std::map<unsigned, unsigned>;

            SDskTrackInfo* pInfo;
            TDskSectorList lSectors;
            TIndexMap lIdsToIndex;
        };


        using inherited = CDisk;
        using TTrackList = std::vector<SDskTrack>;


        void ResetVars();
        void FreeVars();

        const SDskTrack* GetDskTrack(unsigned sideIndex, unsigned trackIndex) const;
        const SDskSector* GetDskSectorByIndex(unsigned sideIndex, unsigned trackIndex, unsigned sectorIndex) const;
        const cpcByte* CDskDisk::GetCopyOfSectorData(const CDskDisk::SDskSector& sector) const;

        bool ReadImage(kmbInputStream* pStream);
        void BuildTrackList();
        void BuildSectorList(SDskTrack& track);


        EFormat m_eFormat;
        SDskDiskInfo m_diskInfo;
        TTrackList m_lTracks;
        cpcByte* m_pRawData;

        // Used to pick a different copy of the data every time a weak sector is read.
        // It doesn't matter which copy is picked as long as it is different each time.
        unsigned m_dataSelector;
    };


} //namespace CPC
