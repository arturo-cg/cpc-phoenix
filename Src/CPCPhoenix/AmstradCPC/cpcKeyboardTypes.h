//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCKEYBOARDTYPES_H_
#define _CPCKEYBOARDTYPES_H_


namespace CPC {


  /**
  ** Amstrad CPC keys.
  */
  enum ECpcKey
  {
    // Main keyboard section
    CPCKEY_ESC = 0,
    CPCKEY_1,
    CPCKEY_2,
    CPCKEY_3,
    CPCKEY_4,
    CPCKEY_5,
    CPCKEY_6,
    CPCKEY_7,
    CPCKEY_8,
    CPCKEY_9,
    CPCKEY_0,
    CPCKEY_MINUS,
    CPCKEY_ARROW,          // Up arrow symbol, pounds/pesetas symbol
    CPCKEY_CLR,
    CPCKEY_DEL,
    CPCKEY_TAB,
    CPCKEY_Q,
    CPCKEY_W,
    CPCKEY_E,
    CPCKEY_R,
    CPCKEY_T,
    CPCKEY_Y,
    CPCKEY_U,
    CPCKEY_I,
    CPCKEY_O,
    CPCKEY_P,
    CPCKEY_AT,             // "@"
    CPCKEY_BRACKET_OPEN,   // "["
    CPCKEY_RETURN,         // The big one, on the main keyboard section
    CPCKEY_CAPS_LOCK,
    CPCKEY_A,
    CPCKEY_S,
    CPCKEY_D,
    CPCKEY_F,
    CPCKEY_G,
    CPCKEY_H,
    CPCKEY_J,
    CPCKEY_K,
    CPCKEY_L,
    CPCKEY_COLON,          // ":"
    CPCKEY_SEMICOLON,      // ";"
    CPCKEY_BRACKET_CLOSE,  // "]"
    CPCKEY_SHIFT,          // Either left or right shift
    CPCKEY_Z,
    CPCKEY_X,
    CPCKEY_C,
    CPCKEY_V,
    CPCKEY_B,
    CPCKEY_N,
    CPCKEY_M,
    CPCKEY_COMMA,
    CPCKEY_PERIOD,
    CPCKEY_SLASH,          // "/"
    CPCKEY_INVERTED_SLASH, // "\"
    CPCKEY_SPACE,
    CPCKEY_CTRL,

    // Cursor keys
    CPCKEY_CURSOR_UP,
    CPCKEY_CURSOR_DOWN,
    CPCKEY_CURSOR_LEFT,
    CPCKEY_CURSOR_RIGHT,
    CPCKEY_COPY,

    // Numeric pad
    CPCKEY_NUM_0,          // On the numeric pad
    CPCKEY_NUM_1,          // On the numeric pad
    CPCKEY_NUM_2,          // On the numeric pad
    CPCKEY_NUM_3,          // On the numeric pad
    CPCKEY_NUM_4,          // On the numeric pad
    CPCKEY_NUM_5,          // On the numeric pad
    CPCKEY_NUM_6,          // On the numeric pad
    CPCKEY_NUM_7,          // On the numeric pad
    CPCKEY_NUM_8,          // On the numeric pad
    CPCKEY_NUM_9,          // On the numeric pad
    CPCKEY_NUM_PERIOD,     // On the numeric pad
    CPCKEY_NUM_ENTER,      // The small one, on the numeric pad

    // Joystick 0
    CPCKEY_JOY_UP,
    CPCKEY_JOY_DOWN,
    CPCKEY_JOY_LEFT,
    CPCKEY_JOY_RIGHT,
    CPCKEY_JOY_FIRE1,
    CPCKEY_JOY_FIRE2,

    CPCKEY_LAST,
    CPCKEY_INVALID = 0x7FFFFFFF
  };


  /**
  ** CPC key states.
  */
  enum ECpcKeyState
  {
    CPCKEYSTATE_PRESSED  = 0,       // Don't change their values, as they match those of the real CPC and are used as-is by the emulator.
    CPCKEYSTATE_RELEASED = 1,
  };


} //namespace CPC

#endif // _CPCKEYBOARDTYPES_H_
