//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCDISKDRIVE_H_
#define _CPCDISKDRIVE_H_


#include "cpcSubSystem.h"
#include "cpcDisk.h"

namespace CPC {


    /**
    ** A disk drive connected to the machine.
    */
    class CDiskDrive : public CSubSystem
    {
    public:

        CDiskDrive(CMachine* pMachine);
        virtual ~CDiskDrive() { FreeVars(); }

        /** Resets the subsystem. */
        virtual void Reset();

        /** Inserts a new disk, or ejects the current one if NULL is specified. */
        void SetDisk(CDisk* disk) { m_disk = disk; }
        /** Returns the disk currently inserted, or NULL if no disk is inside. */
        CDisk* GetDisk() { return m_disk; }
        /** Returns the disk currently inserted, or NULL if no disk is inside (const version). */
        const CDisk* GetDisk() const { return m_disk; }

        /** Moves the R/W head to the given track. */
        void SetTrack(unsigned track) { m_track = track; }
        /** Gets the track where the R/W head is currently at. */
        unsigned GetTrack() const { return m_track; }

        /** Reads the info of the sector at the given index on the current side and track. */
        virtual const CDisk::SSectorInfo* GetSectorInfoByIndex(unsigned side, unsigned sectorIndex) const;
        /** Reads the info of the sector with the given ID on the current side and track. */
        virtual const CDisk::SSectorInfo* GetSectorInfoById(unsigned side, unsigned sectorId) const;
        /** Reads the data of the sector with the given ID on the current side and track. */
        virtual const cpcByte* GetSectorDataById(unsigned side, unsigned sectorId) const;


    private:

        typedef CSubSystem inherited;


        void ResetVars();
        void FreeVars();


        CDisk* m_disk;
        unsigned m_track;         // Current track
    };


} //namespace CPC

#endif // _CPCDISKDRIVE_H_
