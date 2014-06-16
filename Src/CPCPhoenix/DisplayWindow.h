//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _DISPLAYWINDOW_H_
#define _DISPLAYWINDOW_H_


#include "Window/kmbWindow.h"


/**
** 
*/
class DisplayWindow : public kmbWindow
{
public:

                            DisplayWindow             ()  { }
  virtual                  ~DisplayWindow             ()  { End( false ); }

  bool                      Init                      (const RECT& placement, kmbWindow* pParent);
  virtual void              End                       (bool bIncludedSuper = true);


  /** Updates the window with the current content of the emulated display. */
  void                      UpdateDisplayImage        ();


  //
  // Message handlers (INTERNAL USE ONLY)
  //

  // Returns 0 to continue the window creation, or -1 to cancel it.
  virtual LRESULT           _OnPaint                  (HDC hDc);


private:

  static const unsigned     BUFFER_COUNT = 1;

  typedef                   kmbWindow                 Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  BITMAPINFO                m_bufferDibInfo[BUFFER_COUNT];
  unsigned char*            m_pBufferDibBits[BUFFER_COUNT];
  unsigned                  m_nBackBuffer;
  unsigned                  m_nFrontBuffer;

};

#endif // _DISPLAYWINDOW_H_
