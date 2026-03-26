//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcKeyboard.h"
#include "cpcMachine.h"
#include "cpcPsg.h"
#include "cpcKeyStateProvider.h"



namespace CPC {


  /*static*/ const ECpcKey CKeyboard::s_aeKeyboardMatrix[CKeyboard::KEYS_PER_LINE][CKeyboard::LINE_COUNT] = {
    //        Line 0               Line 1              Line 2                 Line 3            Line 4         Line 5        Line 6    Line 7    Line 8            Line 9
    //        ------               ------              ------                 ------            ------         ------        ------    ------    ------            ------
    /*Bit 0*/ CPCKEY_CURSOR_UP,    CPCKEY_CURSOR_LEFT, CPCKEY_CLR,            CPCKEY_ARROW,     CPCKEY_0,      CPCKEY_8,     CPCKEY_6, CPCKEY_4, CPCKEY_1,         CPCKEY_JOY_UP,
    /*Bit 1*/ CPCKEY_CURSOR_RIGHT, CPCKEY_COPY,        CPCKEY_BRACKET_OPEN,   CPCKEY_MINUS,     CPCKEY_9,      CPCKEY_7,     CPCKEY_5, CPCKEY_3, CPCKEY_2,         CPCKEY_JOY_DOWN,
    /*Bit 2*/ CPCKEY_CURSOR_DOWN,  CPCKEY_NUM_7,       CPCKEY_RETURN,         CPCKEY_AT,        CPCKEY_O,      CPCKEY_U,     CPCKEY_R, CPCKEY_E, CPCKEY_ESC,       CPCKEY_JOY_LEFT,
    /*Bit 3*/ CPCKEY_NUM_9,        CPCKEY_NUM_8,       CPCKEY_BRACKET_CLOSE,  CPCKEY_P,         CPCKEY_I,      CPCKEY_Y,     CPCKEY_T, CPCKEY_W, CPCKEY_Q,         CPCKEY_JOY_RIGHT,
    /*Bit 4*/ CPCKEY_NUM_6,        CPCKEY_NUM_5,       CPCKEY_NUM_4,          CPCKEY_SEMICOLON, CPCKEY_L,      CPCKEY_H,     CPCKEY_G, CPCKEY_S, CPCKEY_TAB,       CPCKEY_JOY_FIRE1,
    /*Bit 5*/ CPCKEY_NUM_3,        CPCKEY_NUM_1,       CPCKEY_SHIFT,          CPCKEY_COLON,     CPCKEY_K,      CPCKEY_J,     CPCKEY_F, CPCKEY_D, CPCKEY_A,         CPCKEY_JOY_FIRE1,
    /*Bit 6*/ CPCKEY_NUM_ENTER,    CPCKEY_NUM_2,       CPCKEY_INVERTED_SLASH, CPCKEY_SLASH,     CPCKEY_M,      CPCKEY_N,     CPCKEY_B, CPCKEY_C, CPCKEY_CAPS_LOCK, CPCKEY_INVALID,
    /*Bit 7*/ CPCKEY_NUM_PERIOD,   CPCKEY_NUM_0,       CPCKEY_CTRL,           CPCKEY_PERIOD,    CPCKEY_COMMA,  CPCKEY_SPACE, CPCKEY_V, CPCKEY_X, CPCKEY_Z,         CPCKEY_DEL,
  };

  /*static*/ unsigned CKeyboard::s_anKeyToLineLookUp[CPCKEY_LAST] = {
    //
    // Initialized by code from the data in s_aeKeyboardMatrix
    //
  };



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CKeyboard::CKeyboard(CMachine *pMachine, CKeyStateProvider* pKeyStateProvider) : inherited( pMachine )
  {
    // Reset members
    ResetVars();
    m_pKeyStateProvider = pKeyStateProvider;

    // Initialize the key-to-line look-up table
    InitKeyToLineLookUpTable();
  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CKeyboard::ResetVars()
  {
    m_nSelectedLine = 0;
  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CKeyboard::FreeVars()
  {
    //...
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*virtual*/ void CKeyboard::Reset()
  {
    ResetVars();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  /*static*/ void CKeyboard::InitKeyToLineLookUpTable()
  {
    // Build a table that tells what keyboard matrix line is each key in, for fast look-up when the front end changes the state of a key.
    unsigned nLine;
    unsigned nBit;
    for (nBit = KEYS_PER_LINE-1; nBit < KEYS_PER_LINE; nBit--)
    {
      for (nLine = 0; nLine < LINE_COUNT; nLine++)
      {
        ECpcKey eCurrKey;
        eCurrKey = s_aeKeyboardMatrix[nBit][nLine];
        if (eCurrKey != CPCKEY_INVALID)
        {
          s_anKeyToLineLookUp[eCurrKey] = nLine;
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CKeyboard::SetSelectedLine(int nLine)
  {
    m_nSelectedLine = nLine;

    // Update the line status stored in the PSG (the keyboard matrix is connected to its I/O Port A)
    WriteSelectedLineStatusToPsg();
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CKeyboard::GetSelectedLineStatus() const
  {
    cpcByte nRet = 0;
    if (m_nSelectedLine < LINE_COUNT)
    {
      unsigned nBit;
      for (nBit = KEYS_PER_LINE-1; nBit < KEYS_PER_LINE; nBit--)
      {
        ECpcKey eCurrentKey;
        eCurrentKey = s_aeKeyboardMatrix[nBit][m_nSelectedLine];

        if (eCurrentKey != CPCKEY_INVALID)
        {
          nRet |= (m_pKeyStateProvider->GetKeyState(eCurrentKey) << nBit);
        }
      }
    }
    else
    {
      nRet = 0xFF;     // Value returned by the real CPC when a non-existing line is scanned.
    }

    return nRet;
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CKeyboard::WriteSelectedLineStatusToPsg()
  {
    GetMachine()->GetPsg()->SetIOPortRegisterValue( GetSelectedLineStatus() );
  }

} //namespace CPC
