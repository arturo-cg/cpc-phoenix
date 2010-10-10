//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCMACHINE_H_
#define _CPCMACHINE_H_


namespace CPC {


class CCpu;
class CMemory;
class CGateArray;
class CCrtc;
class CVideoOutput;


/**
** This is the main class in the emulator. It represents the emulated machine (Amstrad CPC)
** and contains all the sub-systems (CPU, memory, keyboard, etc.) that make up the machine.
*/
class CMachine
{
public:

    enum TModel
    {
        CPC_464 = 0,   // 64Kb RAM, 32Kb ROM, tape drive
        CPC_664,       // 64Kb RAM, 48Kb ROM, disc drive
        CPC_6128,      // 128Kb RAM, 48Kb ROM, disc drive

        CPC_LAST,
        CPC_INVALID = 0xFFFFFFFF
    };


                            CMachine                  (TModel eType);
    virtual                ~CMachine                  ()  { FreeVars(); }

    /** Returns the model of the emulated machine. */
    TModel                  GetModel                  () const  { return m_eModel; }

    /** Registers a new frame listener in this machine. */
    void                    RegisterVideoOutput     (CVideoOutput *pVideoOutput);

    /** Returns the CPU subsystem. */
    CCpu                   *GetCpu                    ()        { return m_pCpu; }
    const CCpu             *GetCpu                    () const  { return m_pCpu; }
    /** Returns the memory subsystem. */
    CMemory                *GetMemory                 ()        { return m_pMemory; }
    const CMemory          *GetMemory                 () const  { return m_pMemory; }
    /** Returns the Gate Array subsystem. */
    CGateArray             *GetGateArray              ()        { return m_pGateArray; }
    const CGateArray       *GetGateArray              () const  { return m_pGateArray; }
    /** Returns the CRTC subsystem. */
    CCrtc                  *GetCrtc                   ()        { return m_pCrtc; }
    const CCrtc            *GetCrtc                   () const  { return m_pCrtc; }

    /** Reads a byte from the specified port. */
    cpcByte                 ReadByteFromPort          (cpcWord nPort) const;
    /** Writes a byte in the specified port. */
    void                    WriteByteToPort           (cpcWord nPort, cpcByte nValue);

    /** Resets the machine. */
    void                    Reset                     ();

    /** Runs the emulated machine for the specified period of time.
    *** Time must be in microseconds (1.000.000 usec = 1 sec) */
    void                    Run                       (unsigned nMicroSecs);


private:

    void                    ResetVars                 ();
    void                    FreeVars                  ();


    TModel                  m_eModel;

    CCpu                   *m_pCpu;
    CMemory                *m_pMemory;
    CGateArray             *m_pGateArray;
    CCrtc                  *m_pCrtc;

    unsigned                m_nTimeFromLastFrame;
    CVideoOutput         *m_pVideoOutput;

};


} //namespace CPC

#endif // _CPCMACHINE_H_
