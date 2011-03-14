//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCDISKDRIVE_H_
#define _CPCDISKDRIVE_H_


#include "cpcSubSystem.h"


namespace CPC {

  class CDisk;


  /**
  ** A disk drive connected to the machine.
  */
  class CDiskDrive : public CSubSystem
  {
  public:

                            CDiskDrive                (CMachine *pMachine);
    virtual                ~CDiskDrive                ()  { FreeVars(); }

    /** Resets the subsystem. */
    virtual void            Reset                     ();

    /** Inserts a new disk, or ejects the current one if NULL is specified. */
    void                    SetDisk                   (CDisk* pDisk)  { m_pDisk = pDisk; }
    /** Returns the disk currently inserted, or NULL if no disk is inside. */
    CDisk*                  GetDisk                   ()              { return m_pDisk; }
    /** Returns the disk currently inserted, or NULL if no disk is inside (const version). */
    const CDisk*            GetDisk                   () const        { return m_pDisk; }

    /** [Internal use only] Sets current side and track. */
    void                    _SetCurrentSideAndTrack   (unsigned nSide, unsigned nTrack)  { m_nSide = nSide; m_nTrack = nTrack; }
    /** [Internal use only] Gets current side. */
    unsigned                _GetCurrentSide           () const  { return m_nSide; }
    /** [Internal use only] Gets current track. */
    unsigned                _GetCurrentTrack          () const  { return m_nTrack; }


  private:

    typedef                 CSubSystem                inherited;


    void                    ResetVars                 ();
    void                    FreeVars                  ();


    CDisk*                  m_pDisk;
    unsigned                m_nSide;          // Current side (=head)
    unsigned                m_nTrack;         // Current track

  };


} //namespace CPC

#endif // _CPCDISKDRIVE_H_
