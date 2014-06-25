//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _WINVIDEOOUTPUT_H_
#define _WINVIDEOOUTPUT_H_


#include "cpcVideoOutput.h"


/**
** This class implements the CPC::CVideoOutput interface to provide video output functionality to the emulator using the Windows SDK.
*/
class CWinVideoOutput : public CPC::CVideoOutput
{
public:

  struct SOutput
  {
    unsigned nWidth;
    unsigned nHeight;
    const BITMAPINFO* pDibInfo;
    const unsigned char* pDibBits;
  };


                          CWinVideoOutput           (CPC::CMachine* pMachine);
  virtual                ~CWinVideoOutput           ()  { FreeVars(); }

  bool                    Init                      ();
  void                    End                       ();

  void                    GetOutput                 (SOutput* pOutput) const;


protected:

  /** From CPC::CVideoOutput */
  virtual const SBufferProperties& GetBufferProperties () const;
  virtual unsigned char*  GetBuffer                 ();
  virtual void            OnBufferComplete          ();


private:

  typedef                 CPC::CVideoOutput         inherited;

  static const unsigned   BUFFER_COUNT = 2;
  static const unsigned   DEBUG_COLOR_ARGB = 0x00FF00FF/*Magenta*/;

  void                    ResetVars                 ();
  void                    FreeVars                  ();

  void                    ClearBackBuffer           (unsigned nRgb);


  SBufferProperties       m_bufferProperties;
  BITMAPINFO              m_bufferDibInfo[BUFFER_COUNT];
  unsigned char*          m_pBufferDibBits[BUFFER_COUNT];
  unsigned                m_nBackBuffer;
  unsigned                m_nFrontBuffer;

};

#endif // _WINVIDEOOUTPUT_H_
