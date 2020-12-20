//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcFdc.h"
#include "cpcMachine.h"
#include "cpcDiskDrive.h"



// HU parameter:  Bits 0,1=Unit/Drive Number, Bit 2=Physical Head Number, other bits zero
#define DECODE_HU \
  m_nDesiredDrive =  (m_anParameters[0]) & 0x03; \
  m_nDesiredSide  = ((m_anParameters[0]) & 0x04) >> 2;


namespace CPC {


    struct SCommandInfo
    {
        unsigned nParameterCount;
        unsigned nResultCount;
    };

    static const SCommandInfo s_aCommandInfos[] =
    {
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 8, 7 },  /*COMMAND_READ_TRACK*/
      { 2, 0 },  /*COMMAND_SPECIFY_SPD_DMA*/
      { 1, 1 },  /*COMMAND_SENSE_DRIVE_STATE*/
      { 8, 7 },  /*COMMAND_WRITE_SECTORS*/
      { 8, 7 },  /*COMMAND_READ_SECTORS*/
      { 1, 0 },  /*COMMAND_RECALIBRATE_SEEK*/
      { 0, 2 },  /*COMMAND_SENSE_INT_STATE*/
      { 8, 7 },  /*COMMAND_WRITE_DELETED_SECTORS*/
      { 1, 7 },  /*COMMAND_READ_ID*/
      { 0, 0 },  /*Not used*/
      { 8, 7 },  /*COMMAND_READ_DELETED_SECTORS*/
      { 5, 7 },  /*COMMAND_FORMAT_TRACK*/
      { 0, 0 },  /*Not used*/
      { 2, 0 },  /*COMMAND_SEEK_TRACK_N*/
      { 0, 0 },  /*Not used*/
      { 8, 7 },  /*COMMAND_SCAN_EQUAL*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 8, 7 },  /*COMMAND_SCAN_LOW_OR_EQUAL*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 0, 0 },  /*Not used*/
      { 8, 7 },  /*COMMAND_SCAN_HIGH_OR_EQUAL*/
    };


    //----------------------------------------------------------------------------
    /**
    **
    */
    CFdc::CFdc(CMachine* pMachine) : inherited(pMachine)
    {
        // Reset members
        ResetVars();

        //...
    }

    //----------------------------------------------------------------------------
    /**
    ** ResetVars
    */
    void CFdc::ResetVars()
    {
        m_eCurrentPhase = PHASE_NONE;
        m_eCurrentCommand = COMMAND_INVALID;
        m_nParameterCount = 0;
        m_nCurrentResult = 0;
        m_nCurrentDataDir = DIRECTION_TO_FDC;
        m_seekEnd = false;
        m_pDataPointer = NULL;
        m_nBytesToTransfer = 0;
    }

    //----------------------------------------------------------------------------
    /**
    ** FreeVars
    */
    void CFdc::FreeVars()
    {
        //...
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ void CFdc::Reset()
    {
        ResetVars();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ bool CFdc::RespondToReadPortRequest(cpcWord nPort, cpcByte* pnValue)
    {
        bool bRet = false;

        //
        // 765 FDC port --> Bits 10,7 == 0
        //

        // 765 FDC port?
        if (!(nPort & 0x0480))
        {
            //
            // Bits 8,0 of nPort define the specific register.
            //

            int nRegister;
            nRegister = ((nPort & 0x0100) >> 7) | (nPort & 0x0001);
            switch (nRegister)
            {
            case 2:   *pnValue = ReadMainStatusRegister(); break;
            case 3:   *pnValue = ReadDataRegister(); break;
            default:  *pnValue = 0xFF;
            }

            bRet = true;       // Indicates the device has responded to the port read request.
        }

        return bRet;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    /*virtual*/ void CFdc::RespondToWritePortRequest(cpcWord nPort, cpcByte nValue)
    {
        //
        // 765 FDC port --> Bits 10,7 == 0
        //

        // 765 FDC port?
        if (!(nPort & 0x0480))
        {
            //
            // Bits 8,0 of nPort define the specific register.
            //

            int nRegister;
            nRegister = ((nPort & 0x0100) >> 7) | (nPort & 0x0001);
            switch (nRegister)
            {
            case 0:   TurnMotorOn((nValue & 0x0001) ? true : false); break;
            case 1:   TurnMotorOn((nValue & 0x0001) ? true : false); break;
            case 2:   WriteDataRegister(nValue); break;
            case 3:   WriteDataRegister(nValue); break;
            }
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::TurnMotorOn(bool bOn)
    {
        // We just ignore it in the emulator
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    cpcByte CFdc::ReadMainStatusRegister() const
    {
        cpcByte nBit6;
        nBit6 = (m_nCurrentDataDir == DIRECTION_TO_FDC ? 0 : 1);

        cpcByte nBit5;
        nBit5 = (m_eCurrentPhase == PHASE_EXECUTION ? 1 : 0);

        cpcByte nBit4;
        nBit4 = (m_eCurrentPhase != PHASE_NONE ? 1 : 0);

        return (1 << 7) |   // Bit 7     RQM Request For Master (1=ready for next byte) (see bit 6 for direction)
            (nBit6 << 6) |   // Bit 6     DIO Data Input/Output (0=CPU->FDC, 1=FDC->CPU) (see bit 7)
            (nBit5 << 5) |   // Bit 5     EXM Execution Mode (still in execution-phase, non_DMA_only)
            (nBit4 << 4) |   // Bit 4     CB  FDC Busy (still in command-, execution- or result-phase)
            (0 << 3) |   // Bits 3..0 DB  FDD0..3 Busy (seek/recalib active, until succesful sense intstat)
            (0 << 2) |   // ...
            (0 << 1) |   // ...
            (0 << 0);    // ...
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    cpcByte CFdc::ReadStatusRegister0() const
    {
        const CDiskDrive* pDrive;
        pDrive = GetMachine()->GetDiskDrive(m_nDesiredDrive);

        cpcByte nBit7;
        cpcByte nBit6;
        cpcByte nBit3;
        if ((pDrive == NULL) ||                                         // If drive doesn't exist...
            (pDrive->GetDisk() == NULL) ||                              // If no disk inserted in drive...
            (m_nDesiredSide >= pDrive->GetDisk()->GetSideCount()))      // If inserted disk doesn't have as many sides (i.e. requested second side of a single-sided disk)...
        {
            nBit7 = 1;
            nBit6 = 1;
            nBit3 = 1;    // Drive not ready
        }
        else
        {
            nBit7 = 0;
            nBit6 = 0;
            nBit3 = 0;    // Ok
        }

        cpcByte nBit1, nBit0;
        nBit1 = (m_nDesiredDrive & 0x02) >> 1;
        nBit0 = m_nDesiredDrive & 0x01;

        return (nBit7 << 7) |             // Bits 7,6  IC  Interrupt Code (0=OK, 1=aborted:readfail/OK if EN, 2=unknown cmd ...
               (nBit6 << 6) |             //               or senseint with no int occured, 3=aborted:disc removed etc.)
               (m_seekEnd << 5) |         // Bit 5     SE  Seek End (Set if seek-command completed)
               (0 << 4) |                 // Bit 4     EC  Equipment Check (drive failure or recalibrate failed (retry))
               (nBit3 << 3) |             // Bit 3     NR  Not Ready (drive not ready or non-existing 2nd head selected)
               (m_nDesiredSide << 2) |    // Bit 2     HD  Head Adress (head during interrupt)
               (m_nDesiredDrive & 0x03);  // Bits 1,0  US  Unit Select (driveno during interrupt)
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    cpcByte CFdc::ReadStatusRegister3() const
    {
        const CDiskDrive* drive;
        drive = GetMachine()->GetDiskDrive(m_nDesiredDrive);

        cpcByte bit5 = 0;
        cpcByte bit4 = 0;
        cpcByte bit3 = 0;
        if (drive != NULL)          // If drive exists...
        {
            bit5 = 1;
            bit4 = (drive->_GetCurrentTrack() == 0 ? 1 : 0);    // At track 0?
            bit3 = (drive->GetDisk() != nullptr && drive->GetDisk()->GetSideCount() >= 2 ? 1 : 0);    // Two-sided disk?
        }

        return (0 << 7) |                   // Status of the Fault signal from the FDD.
               (0 << 6) |                   // Status of the Write Protected signal from the FDD.
               (bit5 << 5) |                // Status of the Ready signal from the FDD.
               (bit4 << 4) |                // Status of the Track 0 signal from the FDD.
               (bit3 << 3) |                // Status of the Two Side signal from the FDD.
               (m_nDesiredSide << 2) |      // Status of the Side Select signal to the FDD.
               (m_nDesiredDrive & 0x03);    // Status of the Unit Select 1,0 signals to the FDD.
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    cpcByte CFdc::ReadDataRegister()
    {
        cpcByte nRet = 0;

        if (m_eCurrentPhase == PHASE_EXECUTION)
        {
            //
            // A command like READ_SECTORS is executing.
            //

            switch (m_eCurrentCommand)
            {
            case COMMAND_READ_SECTORS:  nRet = ReadDataRegister_ReadSectors(); break;
            }
        }
        else if (m_eCurrentPhase == PHASE_RESULT)
        {
            //
            // Result bytes written by the last executed command to m_anResult are sent to the CPU.
            //

            nRet = m_anResult[m_nCurrentResult];
            m_nCurrentResult++;

            // Check whether all result bytes have been sent
            if (m_nCurrentResult >= s_aCommandInfos[m_eCurrentCommand].nResultCount)
            {
                ExitResultPhase();
            }
        }

        return nRet;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::WriteDataRegister(cpcByte nValue)
    {
        if (m_eCurrentPhase == PHASE_NONE)
        {
            EnterCommandPhase();
        }

        if (m_eCurrentPhase == PHASE_COMMAND)
        {
            //
            // In this phase, the command and its parameters are gathered from the CPU and written to m_anParameters.
            //

            // The first byte written is the command.
            // Remaining bytes written are the parameters.
            if (m_eCurrentCommand == COMMAND_INVALID)
            {
                m_eCurrentCommand = (ECommand)(nValue & 0x1F);
                m_nParameterCount = 0;
            }
            else
            {
                m_anParameters[m_nParameterCount] = nValue;
                m_nParameterCount++;
            }

            // If all parameters for the selected command have been written, execute the command
            if (m_nParameterCount >= s_aCommandInfos[m_eCurrentCommand].nParameterCount)    // If this was the last parameter...
            {
                // Execute the command
                m_eCurrentPhase = PHASE_EXECUTION;

                switch (m_eCurrentCommand)
                {
                case COMMAND_SPECIFY_SPD_DMA:     ExecuteCommand_SpecifySpdDma(); break;
                case COMMAND_SENSE_DRIVE_STATE:   ExecuteCommand_SenseDriveState(); break;
                case COMMAND_READ_SECTORS:        ExecuteCommand_ReadSectors(); break;
                case COMMAND_RECALIBRATE_SEEK_0:  ExecuteCommand_RecalibrateSeek0(); break;
                case COMMAND_SENSE_INT_STATE:     ExecuteCommand_SenseIntState(); break;
                case COMMAND_READ_ID:             ExecuteCommand_ReadId(); break;
                case COMMAND_SEEK_TRACK_N:        ExecuteCommand_SeekTrackN(); break;
                default:                          KMASSERTM(false, ("Unknown FDC command: %d", m_eCurrentCommand));
                }
            }
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::EnterCommandPhase()
    {
        // Enter Command phase
        m_eCurrentPhase = PHASE_COMMAND;
        m_eCurrentCommand = COMMAND_INVALID;
        m_nCurrentDataDir = DIRECTION_TO_FDC;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::EnterResultPhase()
    {
        // Enter Result phase
        m_eCurrentPhase = PHASE_RESULT;
        m_nCurrentResult = 0;
        m_nCurrentDataDir = DIRECTION_TO_CPU;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::ExitResultPhase()
    {
        m_eCurrentPhase = PHASE_NONE;
        m_nCurrentDataDir = DIRECTION_TO_FDC;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::DecodeTypicalParameters(STypicalParameters* pParams)
    {
        if (pParams != NULL)
        {
            DECODE_HU;
            pParams->nTrackId = m_anParameters[1];
            pParams->nSideId = m_anParameters[2];
            pParams->nFirstSectorId = m_anParameters[3];
            pParams->nSectorSize = m_anParameters[4];
            pParams->nLastSectorId = m_anParameters[5];
            pParams->nGap = m_anParameters[6];
            pParams->nSectorLen = m_anParameters[7];
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::ExecuteCommand_SpecifySpdDma()
    {
        EnterResultPhase();
        ExitResultPhase();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CPC::CFdc::ExecuteCommand_SenseDriveState()
    {
        DECODE_HU;

        EnterResultPhase();

        m_anResult[0] = ReadStatusRegister3();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::ExecuteCommand_RecalibrateSeek0()
    {
        // Seek to physical track 0
        DECODE_HU;

        CDiskDrive* pDrive;
        pDrive = GetMachine()->GetDiskDrive(m_nDesiredDrive);
        if (pDrive != NULL)
        {
            pDrive->_SetCurrentSideAndTrack(m_nDesiredSide, 0);
            m_seekEnd = true;
        }

        EnterResultPhase();
        ExitResultPhase();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::ExecuteCommand_SeekTrackN()
    {
        // Seek to the specified track
        DECODE_HU;

        CDiskDrive* pDrive;
        pDrive = GetMachine()->GetDiskDrive(m_nDesiredDrive);
        if (pDrive != NULL)
        {
            pDrive->_SetCurrentSideAndTrack(m_nDesiredSide, m_anParameters[1]);
            m_seekEnd = true;
        }

        EnterResultPhase();
        ExitResultPhase();
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::ExecuteCommand_SenseIntState()
    {
        EnterResultPhase();

        CDiskDrive* pDrive;
        pDrive = GetMachine()->GetDiskDrive(m_nDesiredDrive);

        m_anResult[0] = ReadStatusRegister0();
        m_anResult[1] = (pDrive != NULL ? pDrive->_GetCurrentTrack() : 0);

        // Reset 'Seek End' flag (returned in Status Register 0).
        // TODO: Check whether this is how this flag actually works or not.
        m_seekEnd = false;
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::ExecuteCommand_ReadId()
    {
        // Read the first correct ID on the track
        DECODE_HU;

        EnterResultPhase();

        CDiskDrive* pDrive;
        pDrive = GetMachine()->GetDiskDrive(m_nDesiredDrive);
        if ((pDrive != NULL) && (pDrive->GetDisk() != NULL))
        {
            const CDisk::SSectorInfo* pSectorInfo;
            pSectorInfo = pDrive->GetDisk()->GetSectorInfo(m_nDesiredSide, pDrive->_GetCurrentTrack(), 0);
            KMASSERT(pSectorInfo != NULL);

            m_anResult[0] = ReadStatusRegister0();
            m_anResult[1] = pSectorInfo->nStatusRegister1;
            m_anResult[2] = pSectorInfo->nStatusRegister2;
            m_anResult[3] = pSectorInfo->nTrack;
            m_anResult[4] = pSectorInfo->nSide;
            m_anResult[5] = pSectorInfo->nId;
            m_anResult[6] = pSectorInfo->nSize;
        }
        else
        {
            m_anResult[0] = ReadStatusRegister0();
            m_anResult[1] = 0;
            m_anResult[2] = 0;
            m_anResult[3] = 0;
            m_anResult[4] = 0;
            m_anResult[5] = 0;
            m_anResult[6] = 0;
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    void CFdc::ExecuteCommand_ReadSectors()
    {
        STypicalParameters params;
        DecodeTypicalParameters(&params);

        CDiskDrive* pDrive;
        pDrive = GetMachine()->GetDiskDrive(m_nDesiredDrive);
        if ((pDrive != NULL) && (pDrive->GetDisk() != NULL))
        {
            pDrive->_SetCurrentSideAndTrack(m_nDesiredSide, params.nTrackId);

            m_pSectorInfo = pDrive->GetDisk()->GetSectorInfoById(m_nDesiredSide, params.nTrackId, params.nFirstSectorId);
            KMASSERT(m_pSectorInfo != NULL);
            m_pDataPointer = pDrive->GetDisk()->GetSectorDataById(m_nDesiredSide, params.nTrackId, params.nFirstSectorId);
            KMASSERT(m_pDataPointer != NULL);
            m_nBytesToTransfer = params.nSectorSize << 8;
            KMASSERT(m_nBytesToTransfer > 0);

            m_nCurrentDataDir = DIRECTION_TO_CPU;
        }
    }

    //----------------------------------------------------------------------------
    /**
    **
    */
    cpcByte CFdc::ReadDataRegister_ReadSectors()
    {
        cpcByte nRet;

        // Read next byte
        nRet = *m_pDataPointer;
        m_pDataPointer++;
        m_nBytesToTransfer--;

        // Check whether we are done reading
        if (m_nBytesToTransfer == 0)
        {
            EnterResultPhase();

            m_anResult[0] = ReadStatusRegister0();
            m_anResult[1] = m_pSectorInfo->nStatusRegister1;
            m_anResult[2] = m_pSectorInfo->nStatusRegister2;
            m_anResult[3] = m_pSectorInfo->nTrack;
            m_anResult[4] = m_pSectorInfo->nSide;
            m_anResult[5] = m_pSectorInfo->nId;
            m_anResult[6] = m_pSectorInfo->nSize;
        }

        return nRet;
    }

} //namespace CPC
