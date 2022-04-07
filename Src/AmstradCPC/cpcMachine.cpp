//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcMachine.h"
#include "cpcCpu.h"
#include "cpcCpuToCpcInterface.h"
#include "cpcMemory.h"
#include "cpcGateArray.h"
#include "cpcCrtc.h"
#include "cpcPpi.h"
#include "cpcPsg.h"
#include "cpcFdc.h"
#include "cpcKeyboard.h"
#include "cpcDiskDrive.h"
#include "cpcVideoOutput.h"
#include "cpcSoundOutput.h"
#include "Snapshot.h"
#include "Stream/kmbInputStream.h"


namespace CPC {



    //----------------------------------------------------------------------------
    /**
    **
    */
    CMachine::CMachine(const MachineSpecifications& machineSpecifications, CKeyStateProvider* pKeyStateProvider)
    {
        ResetVars();

        // Create the sub-systems
        m_pCpuToCpcInterface = new CCpuToCpcInterface(this);
        m_pCpu = new CCpu(this);
        m_pCpu->SetCpuInterface(m_pCpuToCpcInterface);
        m_pCpu->SetInterruptVector(0xFF);       // When the Z80 is in interrupt mode 0, this corresponds to a RST 38h instruction.
        m_pMemory = new CMemory(this, machineSpecifications.memorySpecifications);
        m_pGateArray = new CGateArray(this);
        m_pCrtc = new CCrtc(this);
        m_pPpi = new CPpi(this);
        m_pPsg = new CPsg(this);
        m_pFdc = new CFdc(this);
        m_pKeyboard = new CKeyboard(this, pKeyStateProvider);
        m_pDiskDrives[0] = new CDiskDrive(this);
        m_pDiskDrives[1] = new CDiskDrive(this);
        m_pVideoOutput = NULL;       // This object is provided by the front-end
        m_pSoundOutput = NULL;       // This object is provided by the front-end
    }

    //----------------------------------------------------------------------------
    /**
    ** ResetVars
    */
    void CMachine::ResetVars()
    {
        m_pCpu = NULL;
        m_pCpuToCpcInterface = NULL;
        m_pMemory = NULL;
        m_pGateArray = NULL;
        m_pCrtc = NULL;
        m_pPpi = NULL;
        m_pPsg = NULL;
        m_pFdc = NULL;
        m_pKeyboard = NULL;
        m_pDiskDrives[0] = NULL;
        m_pDiskDrives[1] = NULL;
        m_pVideoOutput = NULL;
        m_pSoundOutput = NULL;
        m_accumulated4MhzCycles = 0;
    }

    //----------------------------------------------------------------------------
    /**
    ** FreeVars
    */
    void CMachine::FreeVars()
    {
        delete m_pDiskDrives[1]; m_pDiskDrives[1] = NULL;
        delete m_pDiskDrives[0]; m_pDiskDrives[0] = NULL;
        delete m_pKeyboard; m_pKeyboard = NULL;
        delete m_pFdc; m_pFdc = NULL;
        delete m_pPsg; m_pPsg = NULL;
        delete m_pPpi; m_pPpi = NULL;
        delete m_pCrtc; m_pCrtc = NULL;
        delete m_pGateArray; m_pGateArray = NULL;
        delete m_pMemory; m_pMemory = NULL;
        delete m_pCpu; m_pCpu = NULL;
        delete m_pCpuToCpcInterface; m_pCpuToCpcInterface = NULL;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    cpcByte CMachine::ReadByteFromPort(cpcWord nPort)
    {
        cpcByte nRet;
        if (!GetFdc()->RespondToReadPortRequest(nPort, &nRet))
        {
            if (!GetPpi()->RespondToReadPortRequest(nPort, &nRet))
            {
                //if ( !GetDEVICE3()->RespondToReadPortRequest(nPort, &nRet) )
                {
                    // No device has responded to the request - Return default value
                    // TODO - What value does the real CPC return in this case?
                    nRet = 0xFF;
                }
            }
        }

        return nRet;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CMachine::WriteByteToPort(cpcWord nPort, cpcByte nValue)
    {
        // The Amstrad CPC doesn't decode the port address fully. This implies that: (1) a device can respond to
        // more than one port address and (2) several devices can respond to a single port address.
        GetGateArray()->RespondToWritePortRequest(nPort, nValue);
        GetCrtc()->RespondToWritePortRequest(nPort, nValue);
        GetPpi()->RespondToWritePortRequest(nPort, nValue);
        GetPsg()->RespondToWritePortRequest(nPort, nValue);
        GetFdc()->RespondToWritePortRequest(nPort, nValue);
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CMachine::Reset()
    {
        GetCpu()->Reset();
        GetMemory()->Reset();
        GetGateArray()->Reset();
        GetCrtc()->Reset();
        GetPpi()->Reset();
        GetPsg()->Reset();
        GetFdc()->Reset();

        if (GetSoundOutput() != NULL)
        {
            GetSoundOutput()->Reset();
        }

        m_accumulated4MhzCycles = 0;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CMachine::Run(unsigned num4MhzCycles)
    {
        for (unsigned i = 0; i < num4MhzCycles; i++)
        {
            // CPU (4Mhz clock)
            GetCpu()->Run(1);

            // Time for a 1Mhz cycle?
            // 1Mhz cycle every 4th 4Mhz cycle.
            m_accumulated4MhzCycles = (m_accumulated4MhzCycles + 1) % 4;
            if (m_accumulated4MhzCycles == 0)
            {
                // CRTC (1Mhz clock)
                GetCrtc()->Run(1);

                // Gate-Array (1Mhz clock)
                GetGateArray()->Run(1);

                // Monitor.
                GetVideoOutput()->Run();

                // PSG (1Mhz clock)
                GetPsg()->Run(1);
            }
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPC::CMachine::ApplySnapshot(const Snapshot& snapshot, kmbInputStream& ramDumpInputStream)
    {
        Reset();

        m_pCpu->SetRegisters(snapshot.GetCpuRegisters());
        m_pGateArray->ApplySnapshot(snapshot.GetGateArray());
        m_pCrtc->ApplySnapshot(snapshot.GetCrtc());

        // Load the snapshot's RAM dump into the machine's RAM.
        m_pMemory->LoadRam(ramDumpInputStream);
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*static*/ void CMachine::GetStandardCpc464Specifications(MachineSpecifications* outSpecifications)
    {
        KMASSERT(outSpecifications != nullptr);     // The outSpecifications parameter must point to a valid instance of Specifications.

                                                    // Memory specifications.
        MemorySpecifications* memorySpecifications = &outSpecifications->memorySpecifications;
        memorySpecifications->numAdditionalRamPages = 0;            // Additional 64KB RAM pages: none, it only has the built-in 64KB RAM.
        memorySpecifications->lowerRomFileName = "OS_464.ROM";      // Lower ROM bank: Firmware v1
        memorySpecifications->upperRomFileNames = {
            { 0, "BASIC_464.ROM" },                                 // Upper ROM bank 0: BASIC
            { 7, "AMSDOS.ROM" },                                    // Upper ROM bank 7: AMSDOS
        };
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*static*/ void CMachine::GetStandardCpc664Specifications(MachineSpecifications* outSpecifications)
    {
        KMASSERT(outSpecifications != nullptr);     // The outSpecifications parameter must point to a valid instance of Specifications.

                                                    // Memory specifications.
        MemorySpecifications* memorySpecifications = &outSpecifications->memorySpecifications;
        memorySpecifications->numAdditionalRamPages = 0;            // Additional 64KB RAM pages: none, it only has the built-in 64KB RAM.
        memorySpecifications->lowerRomFileName = "OS_664.ROM";      // Lower ROM bank: Firmware v2
        memorySpecifications->upperRomFileNames = {
            { 0, "BASIC_664.ROM" },                                 // Upper ROM bank 0: BASIC
            { 7, "AMSDOS.ROM" },                                    // Upper ROM bank 7: AMSDOS
        };
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*static*/ void CMachine::GetStandardCpc6128Specifications(MachineSpecifications* outSpecifications)
    {
        KMASSERT(outSpecifications != nullptr);     // The outSpecifications parameter must point to a valid instance of Specifications.

                                                    // Memory specifications.
        MemorySpecifications* memorySpecifications = &outSpecifications->memorySpecifications;
        memorySpecifications->numAdditionalRamPages = 1;            // Additional 64KB RAM pages: one, built-in 64KB + 64KB RAM expansion (128KB in total).
        memorySpecifications->lowerRomFileName = "OS_6128.ROM";     // Lower ROM bank: Firmware v3
        memorySpecifications->upperRomFileNames = {
            { 0, "BASIC_6128.ROM" },                                // Upper ROM bank 0: BASIC
            { 7, "AMSDOS.ROM" },                                    // Upper ROM bank 7: AMSDOS
        };
    }

} //namespace CPC
