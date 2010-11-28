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

  typedef                   kmbWindow                 Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  BITMAPINFO                m_backBufferDibInfo;
  unsigned char*            m_pBackBufferDibBits;

};

#endif // _DISPLAYWINDOW_H_
