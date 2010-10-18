#ifndef _EMULATORWND_H_
#define _EMULATORWND_H_


class CPC::CMachine;


class CEmulatorWnd : public CWnd
{
public:

  CEmulatorWnd              ();
  virtual                  ~CEmulatorWnd              ();

  void                      SetEmulatedMachine        (CPC::CMachine* pMachine)  { m_pEmulatedMachine = pMachine; }

  void                      UpdateDisplayImage        ();


protected:

  virtual BOOL              PreCreateWindow           (CREATESTRUCT& cs);

  // Generated message map functions
  afx_msg int               OnCreate                  (LPCREATESTRUCT lpCreateStruct);
  afx_msg void              OnPaint                   ();
  DECLARE_MESSAGE_MAP()


private:

  CPC::CMachine*            m_pEmulatedMachine;

  CDC                       m_BackBufferDC;
  CBitmap                   m_BackBufferBitmap;
  unsigned char*            m_pBackBuffer;

};

#endif //_EMULATORWND_H_
