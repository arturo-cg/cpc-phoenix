//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _APPWINDOW_H_
#define _APPWINDOW_H_


#include "Window/kmbWindow.h"


/**
** 
*/
class AppWindow : public kmbWindow
{
public:

                            AppWindow                 ()  { }
  virtual                  ~AppWindow                 ()  { End( false ); }

  bool                      Init                      ();
  virtual void              End                       (bool bIncludedSuper = true);


  void                      UpdateDisplayImage        ();

  //
  // Message handlers (INTERNAL USE ONLY)
  //

  // Returns 0 to continue the window creation, or -1 to cancel it.
  virtual LRESULT           _OnClose                  ();
  virtual LRESULT           _OnPaint                  (HDC hDc);
  virtual LRESULT           _OnKeyDown                (unsigned nVirtualKey);


private:

  typedef                   kmbWindow                 Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  HDC                       m_BackBufferDC;
  HBITMAP                   m_BackBufferBitmap;
  unsigned char*            m_pBackBuffer;

};

#endif // _APPWINDOW_H_
