#ifndef _EMULATORWND_H_
#define _EMULATORWND_H_


class CEmulatorWnd : public CWnd
{
public:

                              CEmulatorWnd              ();
    virtual                  ~CEmulatorWnd              ();

//************************************* PRUEBAS *********************************************************
//************************************* PRUEBAS *********************************************************
    int m_nColorBG;
//************************************* PRUEBAS *********************************************************
//************************************* PRUEBAS *********************************************************


protected:

    virtual BOOL              PreCreateWindow           (CREATESTRUCT& cs);

    // Generated message map functions
    afx_msg int               OnCreate                  (LPCREATESTRUCT lpCreateStruct);
    afx_msg void              OnPaint                   ();
    DECLARE_MESSAGE_MAP()


private:

    CDC                       m_BackBufferDC;
    CBitmap                   m_BackBufferBmp;
    CSize                     m_BackBufferSize;

};

#endif //_EMULATORWND_H_
