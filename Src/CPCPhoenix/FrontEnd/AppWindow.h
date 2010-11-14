//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _APPWINDOW_H_
#define _APPWINDOW_H_


#include "Window/kmbWindow.h"

namespace CPC
{
  enum ECpcKey;
}


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


  /** The application notifies that the settings have changed. */
  void                      OnApplicationSettingsChanged ();

  /** Updates the window with the current content of the emulated display. */
  void                      UpdateDisplayImage        ();

  /** Returns the accelerators used by this window. */
  HACCEL                    GetAccelerators           () const  { return m_hAccelerators; }


  //
  // Message handlers (INTERNAL USE ONLY)
  //

  // Returns 0 to continue the window creation, or -1 to cancel it.
  virtual LRESULT           _OnClose                  ();
  virtual LRESULT           _OnPaint                  (HDC hDc);
  virtual LRESULT           _OnMenuCommand            (WORD nItemId, bool bFromAccelerator);
  virtual LRESULT           _OnKeyDown                (unsigned nVirtualKey);
  virtual LRESULT           _OnKeyUp                  (unsigned nVirtualKey);


private:

  typedef                   kmbWindow                 Super;


  void                      ResetVars                 ();
  void                      FreeVars                  ();

  CPC::ECpcKey              MapKeyHostToCpc           (unsigned nVirtualKey);


  HMENU                     m_hMainMenu;
  HACCEL                    m_hAccelerators;

  HDC                       m_BackBufferDC;
  HBITMAP                   m_BackBufferBitmap;
  unsigned char*            m_pBackBuffer;

};

#endif // _APPWINDOW_H_
