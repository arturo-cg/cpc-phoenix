//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#ifndef _KMBTEXTMSBREADER_H_
#define _KMBTEXTMSBREADER_H_


#include "kmbMsb.h"

class kmbInputStream;


/**
** 
*/
class kmbTextMsbReader
{
public:

                            kmbTextMsbReader          ()  { m_bOk = false; }
  virtual                  ~kmbTextMsbReader          ()  { End(); }

  bool                      Init                      (kmbInputStream* pStream);
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  kmbMsbPtr&                GetRootMsb                ()  { return m_ptrRootMsb; }

  unsigned                  GetLastTokenLine          () const  { return m_uTokenLine; }
  unsigned                  GetLastTokenColumn        () const  { return m_uTokenColumn; }


private:

  enum TTokenType
  {
    TOK_BRACKET_OPEN = 0,   // [
    TOK_BRACKET_CLOSE,      // ]
    TOK_PARENTHESIS_OPEN,   // (
    TOK_PARENTHESIS_CLOSE,  // )

    TOK_EQUAL,              // =

    TOK_INTEGER,            // Integer number
    TOK_REAL,               // Real number
    TOK_STRING,             // String
    TOK_TAG,                // Tag name

    TOK_EOF,                // End of File marker

    TOK_ERROR               // Error
  };


  void                      ResetVars                 ();
  void                      FreeVars                  ();

  //
  // Lexical parser methods
  //

  void                      ReadToken                 ();

  void                      ReadChar                  ();
  void                      ParseTag                  ();
  void                      ParseString               ();
  void                      ParseNumber               (bool bIsNegative);
  void                      ParseComment              ();


  //
  // Syntactical parser methods
  //

  bool                      sMsb                      (kmbMsbPtr *pptrMsb);
  bool                      sTaggedSect               (const kmbMsbPtr &ptrTaggedMsb);
  bool                      sEnumSect                 (const kmbMsbPtr &ptrEnumMsb);


  bool                      m_bOk;

  kmbMsbPtr                 m_ptrRootMsb;        // The root Msb in the file

  //
  // Lexical parser variables
  //

  kmbInputStream*           m_pStream;
  char                      m_cCurrentChar;

  bool                      m_bJumpLine;
  unsigned                  m_uLine;
  unsigned                  m_uColumn;

  unsigned                  m_uTokenLine;      // Which line current token begins at
  unsigned                  m_uTokenColumn;    // Which column current token begins at
  TTokenType                m_eTokenType;      // Open bracket, integer, tag name, etc. See TTokenType enumeration above.
  int                       m_iTokenValue;     // For TOK_INTEGER
  double                    m_dTokenValue;     // For TOK_REAL
  std::string               m_sTokenValue;     // For TOK_STRING and TOK_TAG (in this case, the initial period is NOT included)


  //
  // Syntactical parser variables
  //

};

#endif // _KMBTEXTMSBREADER_H_
