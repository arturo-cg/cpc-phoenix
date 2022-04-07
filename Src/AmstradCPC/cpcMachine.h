//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCMACHINE_H_
#define _CPCMACHINE_H_


#include "cpcMemory.h"

class kmbInputStream;

namespace CPC {

    class CCpu;
    class CCpuToCpcInterface;
    class CGateArray;
    class CCrtc;
    class CPpi;
    class CPsg;
    class CFdc;
    class CKeyboard;
    class CKeyStateProvider;
    class CDiskDrive;
    class CVideoOutput;
    class CSoundOutput;
    class Snapshot;

    struct MachineSpecifications
    {
        //unsigned numDiskDrives;       // TODO - Number of disk drives connected to the machine.
        MemorySpecifications memorySpecifications;   // Memory specifications (number of additional 64KB RAM pages, ROM banks that are present).
    };


    /**
    ** This is the main class in the emulator. It represents the emulated machine (Amstrad CPC)
    ** and contains all the sub-systems (CPU, memory, Gate Array, etc.) that make up the machine.
    */
    class CMachine
    {
    public:

        static const unsigned DRIVE_COUNT = 2;


                                CMachine(const MachineSpecifications& machineSpecifications, CKeyStateProvider* pKeyStateProvider);
        virtual                ~CMachine() { FreeVars(); }

        /** Returns the CPU subsystem. */
        CCpu*                   GetCpu() { return m_pCpu; }
        const CCpu*             GetCpu() const { return m_pCpu; }
        /** Returns the memory subsystem. */
        CMemory*                GetMemory() { return m_pMemory; }
        const CMemory*          GetMemory() const { return m_pMemory; }
        /** Returns the Gate Array subsystem. */
        CGateArray*             GetGateArray() { return m_pGateArray; }
        const CGateArray*       GetGateArray() const { return m_pGateArray; }
        /** Returns the CRTC subsystem. */
        CCrtc*                  GetCrtc() { return m_pCrtc; }
        const CCrtc*            GetCrtc() const { return m_pCrtc; }
        /** Returns the 8255 PPI subsystem. */
        CPpi*                   GetPpi() { return m_pPpi; }
        const CPpi*             GetPpi() const { return m_pPpi; }
        /** Returns the AY-3-8912 Programmable Sound Generator (PSG) subsystem. */
        CPsg*                   GetPsg() { return m_pPsg; }
        const CPsg*             GetPsg() const { return m_pPsg; }
        /** Returns the 765 FDC (Floppy Disk Controller) subsystem. */
        CFdc*                   GetFdc() { return m_pFdc; }
        const CFdc*             GetFdc() const { return m_pFdc; }
        /** Returns the keyboard subsystem. */
        CKeyboard*              GetKeyboard() { return m_pKeyboard; }
        const CKeyboard*        GetKeyboard() const { return m_pKeyboard; }
        /** Returns the specified disk drive. */
        CDiskDrive*             GetDiskDrive(unsigned nDrive) { return (nDrive < DRIVE_COUNT ? m_pDiskDrives[nDrive] : NULL); }
        const CDiskDrive*       GetDiskDrive(unsigned nDrive) const { return (nDrive < DRIVE_COUNT ? m_pDiskDrives[nDrive] : NULL); }
        /** Sets the video output subsystem, or removes it if NULL is specified. This object is created and destroyed by the front-end. */
        void                    SetVideoOutput(CVideoOutput* pVideoOutput) { m_pVideoOutput = pVideoOutput; }
        /** Returns the video output subsystem. */
        CVideoOutput*           GetVideoOutput() { return m_pVideoOutput; }
        const CVideoOutput*     GetVideoOutput() const { return m_pVideoOutput; }
        /** Sets the sound output subsystem, or removes it if NULL is specified. This object is created and destroyed by the front-end. */
        void                    SetSoundOutput(CSoundOutput* pSoundOutput) { m_pSoundOutput = pSoundOutput; }
        /** Returns the sound output subsystem. */
        CSoundOutput*           GetSoundOutput() { return m_pSoundOutput; }
        const CSoundOutput*     GetSoundOutput() const { return m_pSoundOutput; }

        /** Reads a byte from the specified port. */
        cpcByte                 ReadByteFromPort(cpcWord nPort);
        /** Writes a byte in the specified port. */
        void                    WriteByteToPort(cpcWord nPort, cpcByte nValue);

        /** Resets the machine. */
        void                    Reset();

        /** Runs the emulated machine for the specified period of time.
        *** Time is in cycles of a 4Mhz clock. */
        void                    Run(unsigned num4MhzCycles);

        /** Applies the specified snapshot to the machine. RAM content is read from the specified stream. */
        void                    ApplySnapshot(const Snapshot& snapshot, kmbInputStream& ramDumpInputStream);

        /** Static utility method that fills in the specs for a standard Amstrad CPC 464. */
        static void             GetStandardCpc464Specifications(MachineSpecifications* outSpecifications);
        /** Static utility method that fills in the specs for a standard Amstrad CPC 664. */
        static void             GetStandardCpc664Specifications(MachineSpecifications* outSpecifications);
        /** Static utility method that fills in the specs for a standard Amstrad CPC 6128. */
        static void             GetStandardCpc6128Specifications(MachineSpecifications* outSpecifications);


    private:

        void                    ResetVars();
        void                    FreeVars();


        CCpu*                   m_pCpu;
        CCpuToCpcInterface*     m_pCpuToCpcInterface;
        CMemory*                m_pMemory;
        CGateArray*             m_pGateArray;
        CCrtc*                  m_pCrtc;
        CPpi*                   m_pPpi;
        CPsg*                   m_pPsg;
        CFdc*                   m_pFdc;
        CKeyboard*              m_pKeyboard;
        CDiskDrive*             m_pDiskDrives[DRIVE_COUNT];
        CVideoOutput*           m_pVideoOutput;
        CSoundOutput*           m_pSoundOutput;

        unsigned                m_accumulated4MhzCycles;
    };


} //namespace CPC

#endif // _CPCMACHINE_H_
