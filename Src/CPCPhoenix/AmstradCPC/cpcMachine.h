//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCMACHINE_H_
#define _CPCMACHINE_H_


namespace CPC {


  class CCpu;
  class CMemory;
  class CGateArray;
  class CCrtc;
  class CPpi;
  class CDisplay;


  /**
  ** This is the main class in the emulator. It represents the emulated machine (Amstrad CPC)
  ** and contains all the sub-systems (CPU, memory, Gate Array, etc.) that make up the machine.
  */
  class CMachine
  {
  public:

    enum EModel
    {
      MODEL_464 = 0,   // 64Kb RAM, 32Kb ROM (OS v1, BASIC 1.0), tape drive
      MODEL_664,       // 64Kb RAM, 48Kb ROM (OS v2, BASIC 1.1, AMSDOS), disc drive
      MODEL_6128,      // 128Kb RAM, 48Kb ROM (OS v3, BASIC 1.1, AMSDOS), disc drive

      MODEL_LAST,
      MODEL_INVALID = 0xFFFFFFFF
    };


                            CMachine                  (EModel eType);
    virtual                ~CMachine                  ()  { FreeVars(); }

    /** Returns the model of the emulated machine. */
    EModel                  GetModel                  () const  { return m_eModel; }

    /** Returns the current frame count. */
    unsigned                GetFrameCount             () const;

    /** Returns the CPU subsystem. */
    CCpu*                   GetCpu                    ()        { return m_pCpu; }
    const CCpu*             GetCpu                    () const  { return m_pCpu; }
    /** Returns the memory subsystem. */
    CMemory*                GetMemory                 ()        { return m_pMemory; }
    const CMemory*          GetMemory                 () const  { return m_pMemory; }
    /** Returns the Gate Array subsystem. */
    CGateArray*             GetGateArray              ()        { return m_pGateArray; }
    const CGateArray*       GetGateArray              () const  { return m_pGateArray; }
    /** Returns the CRTC subsystem. */
    CCrtc*                  GetCrtc                   ()        { return m_pCrtc; }
    const CCrtc*            GetCrtc                   () const  { return m_pCrtc; }
    /** Returns the 8255 PPI subsystem. */
    CPpi*                   GetPpi                    ()        { return m_pPpi; }
    const CPpi*             GetPpi                    () const  { return m_pPpi; }
    /** Returns the display subsystem. */
    CDisplay*               GetDisplay                ()        { return m_pDisplay; }
    const CDisplay*         GetDisplay                () const  { return m_pDisplay; }

    /** Reads a byte from the specified port. */
    cpcByte                 ReadByteFromPort          (cpcWord nPort);
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


    EModel                  m_eModel;

    CCpu*                   m_pCpu;
    CMemory*                m_pMemory;
    CGateArray*             m_pGateArray;
    CCrtc*                  m_pCrtc;
    CPpi*                   m_pPpi;
    CDisplay*               m_pDisplay;

  };


} //namespace CPC

#endif // _CPCMACHINE_H_
