//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCSUBSYSTEM_H_
#define _CPCSUBSYSTEM_H_


namespace CPC {


  class CMachine;


  /**
  ** Represents a sub-system of the machine, such as the CPU, the memory or the sound hardware.
  ** Each concrete sub-system will be implemented in a class derived from this one.
  */
  class CSubSystem
  {
  public:

    virtual                ~CSubSystem                ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     () = 0;

    /** We are notified that the machine is trying to read a byte from this subsystem.
    *** Usually it's the CPU through an IN instruction. */
    virtual bool            RespondToReadPortRequest  (cpcWord nPort, cpcByte* pnValue)  { return false; }
    /** We are notified that the machine is trying to write a byte to this subsystem.
    *** Usually it's the CPU through an OUT instruction. */
    virtual void            RespondToWritePortRequest (cpcWord nPort, cpcByte nValue)    { }


  protected:

    // Constructor is protected to avoid instantiating this class
                            CSubSystem                (CMachine* pMachine);

    void                    ResetVars                 ();
    void                    FreeVars                  ();

    /** Returns the machine this subsystem is part of. */
    inline CMachine*        GetMachine                ()        { return m_pMachine; }
    inline const CMachine*  GetMachine                () const  { return m_pMachine; }


    CMachine*               m_pMachine;


  private:

  };


} //namespace CPC

#endif // _CPCSUBSYSTEM_H_
