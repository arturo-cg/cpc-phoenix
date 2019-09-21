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



namespace CPC {



    //----------------------------------------------------------------------------
    /**
    **
    */
    CMachine::CMachine(EModel eType, CKeyStateProvider* pKeyStateProvider)
    {
        ResetVars();

        m_eModel = eType;

        // Create the sub-systems
        m_pCpuToCpcInterface = new CCpuToCpcInterface(this);
        m_pCpu = new CCpu(this);
        m_pCpu->SetCpuInterface(m_pCpuToCpcInterface);
        m_pCpu->SetInterruptVector(0xFF);       // When the Z80 is in interrupt mode 0, this corresponds to a RST 38h instruction.
        m_pMemory = new CMemory(this);
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
        m_eModel = MODEL_INVALID;
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
        m_fAccumulatedCpuCycles = 0.f;
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
        if (!GetPpi()->RespondToReadPortRequest(nPort, &nRet))
        {
            if (!GetFdc()->RespondToReadPortRequest(nPort, &nRet))
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
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CMachine::Run(unsigned nNum1MhzCycles)
    {
        // CPU (4Mhz clock)
        GetCpu()->Run(nNum1MhzCycles * 4);

        // CRTC (1Mhz clock)
        GetCrtc()->Run(nNum1MhzCycles);

        // Gate-Array (1Mhz clock)
        GetGateArray()->Run(nNum1MhzCycles);

        // Monitor.
        GetVideoOutput()->Run();

        // PSG (1Mhz clock)
        GetPsg()->Run(nNum1MhzCycles);
    }

} //namespace CPC
