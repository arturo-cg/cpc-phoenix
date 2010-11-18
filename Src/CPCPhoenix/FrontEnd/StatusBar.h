//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _STATUSBAR_H_
#define _STATUSBAR_H_


class kmbWindow;


/**
** 
*/
class StatusBar
{
public:

                            StatusBar                 ()  { m_bOk = false; }
  virtual                  ~StatusBar                 ()  { End(); }

  bool                      Init                      (kmbWindow* pParentWnd);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  HWND                      GetHWnd                   () const  { return m_hWnd; }

  void                      SetInsertedDiskName       (const string& sDiskName);
  void                      SetEmulationSpeed         (float fSpeed);


private:

  enum EPart
  {
    PART_INSERTEDDISK = 0,
    PART_EMULATIONSPEED,

    PART_LAST,
    PART_INVALID = 0x7FFFFFFF
  };


  void                      ResetVars                 ();
  void                      FreeVars                  ();


  bool                      m_bOk;

  HWND                      m_hWnd;

};

#endif // _STATUSBAR_H_
