//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcDiskDrive.h"
#include "cpcDisk.h"



namespace CPC {


    CDiskDrive::CDiskDrive(CMachine* pMachine) : inherited(pMachine)
    {
        // Reset members
        ResetVars();

        //...
    }

    void CDiskDrive::ResetVars()
    {
        m_disk = nullptr;
        m_track = 0;
    }

    void CDiskDrive::FreeVars()
    {
        //...
    }

    /*virtual*/ void CDiskDrive::Reset()
    {
        ResetVars();
    }

    const CDisk::SSectorInfo* CDiskDrive::GetSectorInfoByIndex(unsigned side, unsigned sectorIndex) const
    {
        return (m_disk != nullptr ? m_disk->GetSectorInfo(side, m_track, sectorIndex) : nullptr);
    }

    const CDisk::SSectorInfo* CDiskDrive::GetSectorInfoById(unsigned side, unsigned sectorId) const
    {
        return (m_disk != nullptr ? m_disk->GetSectorInfoById(side, m_track, sectorId) : nullptr);
    }

    const cpcByte* CDiskDrive::GetSectorDataById(unsigned side, unsigned sectorId) const
    {
        return (m_disk != nullptr ? m_disk->GetSectorDataById(side, m_track, sectorId) : nullptr);
    }

} //namespace CPC
