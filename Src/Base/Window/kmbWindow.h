//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBWINDOW_H_
#define _KMBWINDOW_H_


/**
** Very simple wrapper of a Windows window.
**
** Important: The static function kmbWindow::RegisterWindowClass must be called once before creating any kmbWindow.
*/
class kmbWindow
{
public:

  enum
  {
    MODIFIERKEY_SHIFT = 0x01,
    MODIFIERKEY_CTRL  = 0x02,
    MODIFIERKEY_ALT   = 0x04,
  };


                            kmbWindow                 ()  { m_bOk = false; }
  virtual                  ~kmbWindow                 ()  { End(); }

  bool                      Init                      (const string& sWindowText, DWORD dwStyle,
                                                       int x, int y, int nWidth, int nHeight,
                                                       HWND hParentOrOwner);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  /** Returns the encapsulated Windows HWND handle of this window. */
  HWND                      GetHWnd                   () const  { return m_hWnd; }

  /** Returns the device context of this window. */
  HDC                       GetDc                     () const  { return ::GetDC(m_hWnd); }

  /** Sends a WM_CLOSE message to this window. The window is free to handle (and therefore destroy) or ignore the message. */
  void                      RequestClose              ();

  /** Sets the text of the window. If for example the window has a title bar, this is the text that will be changed. */
  void                      SetText                   (const string& sText);

  /** Changes the position of the window. */
  void                      SetPosition               (long x, long y);
  /** Changes the size of the window. */
  void                      SetSize                   (long width, long height);
  /** Changes the position and size of the window.
  *** Note that the size is specified in terms of the bottom-right corner position. */
  void                      SetRect                   (const RECT& newRect);
  /** Returns the RECT of the window, which includes its position and its size. */
  void                      GetRect                   (RECT* pRect)        { ::GetWindowRect(m_hWnd, pRect); }
  /** Returns the RECT of the window client area, which includes its position and its size. */
  void                      GetClientRect             (RECT* pClientRect)  { ::GetClientRect(m_hWnd, pClientRect); }

  /** Sets the input focus to this window. A _OnKillFocus event will be sent to the window that currently has the focus
  *** and a _OnSetFocus event will be sent to this window. */
  void                      GrabInputFocus            ();

  /** Captures or releases the mouse. Once the mouse is captured, all mouse events will be received by this window. */
  void                      CaptureMouse              (bool bCapture);

  /** Sets a new mouse cursor and returns the previous one.
  *** The window sets this cursor whenever the mouse is in its client area. */
  HCURSOR                   SetMouseCursor            (HCURSOR hNewCursor);

  /** Invalidates a rectangle of the window. */
  void                      InvalidateRect            (const RECT& rRect, bool bEraseBackground);
  /** Invalidates the whole window. */
  void                      InvalidateAll             (bool bEraseBackground);

  //
  // Message handlers (INTERNAL USE ONLY)
  //

  // Returns 0 to continue the window creation, or -1 to cancel it.
  virtual LRESULT           _OnCreate                 (CREATESTRUCT *pCreateStruct);
  virtual LRESULT           _OnClose                  ();
  virtual LRESULT           _OnDestroy                ();
  virtual LRESULT           _OnControlCommand         (WORD nCommand, WORD nCtrlId, HWND hCtrl);
  virtual LRESULT           _OnMenuCommand            (WORD nItemId, bool bFromAccelerator);
  virtual LRESULT           _OnPaint                  (HDC hDc);
  // prRect is in screen coordinates.
  virtual LRESULT           _OnSizing                 (LPRECT prRect);
  virtual LRESULT           _OnSize                   (int iWidth, int iHeight);
  virtual LRESULT           _OnActivate               (int iType, bool bMinimized, HWND hPreviousWnd);
  virtual LRESULT           _OnActivateApp            (bool bActivated);
  virtual LRESULT           _OnSetFocus               (HWND hPrevWnd);
  virtual LRESULT           _OnKeyDown                (unsigned nVirtualKey);
  virtual LRESULT           _OnKeyUp                  (unsigned nVirtualKey);
  virtual LRESULT           _OnCharKey                (unsigned nChar, bool bIsExtendedKey);
  virtual LRESULT           _OnLButtonDown            (short x, short y, int nModifierKeys);
  virtual LRESULT           _OnLButtonUp              (short x, short y, int nModifierKeys);
  virtual LRESULT           _OnRButtonDown            (short x, short y, int nModifierKeys);
  virtual LRESULT           _OnRButtonUp              (short x, short y, int nModifierKeys);
  virtual LRESULT           _OnMButtonDown            (short x, short y, int nModifierKeys);
  virtual LRESULT           _OnMButtonUp              (short x, short y, int nModifierKeys);
  virtual LRESULT           _OnMouseMove              (short x, short y, int nModifierKeys);
  virtual LRESULT           _OnMouseWheel             (int nDelta, short x, short y, int nModifierKeys);

  // (INTERNAL USE ONLY) Called by the window procedure during window creation.
  void                      _SetHWnd                  (HWND hWnd)  { m_hWnd = hWnd; }

  /** Important: This static function must be called once before creating any kmbWindow. */
  static void               RegisterWindowClass       ();


protected:

  bool                      InitDialog                ();


  bool                      m_bOk;


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();

  bool                      CreateWnd                 (const string& sWindowText, DWORD dwStyle,
                                                       int x, int y, int nWidth, int nHeight,
                                                       HWND hParentOrOwner);
  void                      DestroyWnd                ();


  HWND                      m_hWnd;
  HCURSOR                   m_hMouseCursor;

  static bool               ms_bWindowClassRegistered;

};

#endif // _KMBWINDOW_H_
