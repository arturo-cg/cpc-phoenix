//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbTextMsbReader.h"
#include "kmbMsbManager.h"
#include "Stream/kmbInputStream.h"
#include "Stream/kmbFileInputStream.h"
#include "File/kmbFile.h"



//----------------------------------------------------------------------------
/**
** Init
*/
bool kmbTextMsbReader::Init(kmbInputStream* pStream)
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if (bRet)
  {
    KMASSERTM( (pStream != NULL) && pStream->IsOk(), ("The stream is not valid.") );
    bRet = ( (pStream != NULL) && pStream->IsOk() );
  }

  // Initialize members
  if (bRet)
  {
    m_pStream = pStream;
  }

  // Initialize parser
  if (bRet)
  {
    ReadChar();       // Read first character
    ReadToken();      // Read first token
  }

  // Parse the file
  if (bRet)
  {
    bRet = sMsb( &m_ptrRootMsb );

    if(bRet && (m_ptrRootMsb == NULL))
    {
      bRet = false;

      // Print the file name if it is a file stream
      string sStreamDesc;
      if (pStream->GetType() == STREAMTYPE_FILE)
      {
        sStreamDesc = ((kmbFileInputStream*)pStream)->GetSourceFile()->GetFileName();
      }
      else
      {
        sStreamDesc = "<stream>";
      }
      KMASSERTM( bRet, ("Error parsing MSB '%s' at line %d, column %d.", sStreamDesc.c_str(), m_uTokenLine, m_uTokenColumn) );
    }
  }

//***************************** PRUEBAS *********************************************
//***************************** PRUEBAS *********************************************
/*
  if(bRet)
  {
    char szToken[200];
    char szTemp[200];

    do
    {
      ReadToken();

      switch(m_eTokenType)
      {
      case TOK_BRACKET_OPEN:
        sprintf(szToken, "<TOK_BRACKET_OPEN>");
        break;

      case TOK_BRACKET_CLOSE:
        sprintf(szToken, "<TOK_BRACKET_CLOSE>");
        break;

      case TOK_PARENTHESIS_OPEN:
        sprintf(szToken, "<TOK_PARENTHESIS_OPEN>");
        break;

      case TOK_PARENTHESIS_CLOSE:
        sprintf(szToken, "<TOK_PARENTHESIS_CLOSE>");
        break;

      case TOK_EQUAL:
        sprintf(szToken, "<TOK_EQUAL>");
        break;

      case TOK_INTEGER:
        sprintf(szToken, "<TOK_INTEGER, %d>", m_iTokenValue);
        break;

      case TOK_REAL:
        sprintf(szToken, "<TOK_REAL, %f>", m_dTokenValue);
        break;

      case TOK_STRING:
        sprintf(szToken, "<TOK_STRING, \"%s\">", m_sTokenValue.c_str());
        break;

      case TOK_TAG:
        sprintf(szToken, "<TOK_TAG, \"%s\">", m_sTokenValue.c_str());
        break;

      case TOK_EOF:
        sprintf(szToken, "<TOK_EOF>");
        break;

      case TOK_ERROR:
        sprintf(szToken, "<TOK_ERROR>");
        break;
      }

      sprintf(szTemp, "%s at line=%d column=%d\n", szToken, m_uTokenLine, m_uTokenColumn);
      OutputDebugString(szTemp);
    }
    while((m_eTokenType != TOK_EOF) && (m_eTokenType != TOK_ERROR));
  }
*/
//***************************** PRUEBAS *********************************************
//***************************** PRUEBAS *********************************************


  if(bRet)
    m_bOk = true;
  else
    FreeVars();

  return bRet;
}

//----------------------------------------------------------------------------
/**
** End
*/
/*virtual*/ void kmbTextMsbReader::End()
{
  if( IsOk() )
  {
    FreeVars();
    m_bOk = false;
  }
}

//----------------------------------------------------------------------------
/**
** ResetVars
*/
void kmbTextMsbReader::ResetVars()
{
  m_ptrRootMsb   = NULL;
  m_pStream      = NULL;

  m_cCurrentChar = '\0';

  m_bJumpLine    = false;
  m_uLine        = 1;
  m_uColumn      = 0;

  m_uTokenLine   = 1;
  m_uTokenColumn = 0;
  m_eTokenType   = TOK_ERROR;
  m_iTokenValue  = 0;
  m_dTokenValue  = 0;
  m_sTokenValue  = "";
}

//----------------------------------------------------------------------------
/**
** FreeVars
*/
void kmbTextMsbReader::FreeVars()
{
  m_ptrRootMsb = NULL;
}



//###########################################################################################
//#########################         LEXICAL PARSER        ###################################
//###########################################################################################


//----------------------------------------------------------------------------
/**
** Reads a new character from the file and moves to the next one.
*/
void kmbTextMsbReader::ReadChar()
{
  // Read next character
  m_pStream->ReadChar( &m_cCurrentChar );

  // Transparently handle newlines composed of CR+LF (\r\n) characters.
  // This allows to read text generated on Unix systems (which use LF as new line character) as well as on Windows (which use the CR+LF sequence).
  if (m_cCurrentChar == '\r')
  {
    m_pStream->ReadChar( &m_cCurrentChar );
    KMASSERT( m_cCurrentChar == '\n' );
  }

  // Update current column and line
  if (m_bJumpLine)
  {
    m_uLine++;
    m_uColumn   = 1;
    m_bJumpLine = false;
  }
  else
  {
    m_uColumn++;
  }

  if (m_cCurrentChar == '\n')
  {
    m_bJumpLine = true;
  }
}

//----------------------------------------------------------------------------
/**
** Reads a new token from the file.
*/
void kmbTextMsbReader::ReadToken()
{
  // Ignore blank spaces, tabs and new line characters
  while ( !m_pStream->IsAtEnd() &&
          ((m_cCurrentChar == ' ') || (m_cCurrentChar == '\t') || (m_cCurrentChar == '\n')) )
  {
    ReadChar();
  }

  m_uTokenLine   = m_uLine;
  m_uTokenColumn = m_uColumn;

  // Parse the file until a new token has been read
  if (m_pStream->IsAtEnd() || (m_cCurrentChar == 0))    // If end of stream reached or end of string null character reached...
  {
    m_eTokenType = TOK_EOF;
  }
  else
  if(m_cCurrentChar == '[')  // Open square bracket (begins tagged section)
  {
    m_eTokenType = TOK_BRACKET_OPEN;
    ReadChar();
  }
  else
  if(m_cCurrentChar == ']')  // Close square bracket (ends tagged section)
  {
    m_eTokenType = TOK_BRACKET_CLOSE;
    ReadChar();
  }
  else
  if(m_cCurrentChar == '(')  // Open parenthesis (begins enumerated section)
  {
    m_eTokenType = TOK_PARENTHESIS_OPEN;
    ReadChar();
  }
  else
  if(m_cCurrentChar == ')')  // Close parenthesis (ends enumerated section)
  {
    m_eTokenType = TOK_PARENTHESIS_CLOSE;
    ReadChar();
  }
  else
  if(m_cCurrentChar == '=')  // Equal (to a tag)
  {
    m_eTokenType = TOK_EQUAL;
    ReadChar();
  }
  else
  if(m_cCurrentChar == '.')  // Period (every tag must be preceded by a period)
  {
    m_sTokenValue = "";    // The leading period is NOT included
    ReadChar();
    ParseTag();
  }
  else
  if(m_cCurrentChar == '"')  // Begins a string
  {
    m_sTokenValue = "";
    ReadChar();
    ParseString();
  }
  else
  if(isdigit(m_cCurrentChar))  // Positive number (either integer or real)
  {
    m_iTokenValue = m_cCurrentChar - '0';
    ReadChar();
    ParseNumber(false);
  }
  else
  if(m_cCurrentChar == '-')  // Negative number (either integer or real)
  {
    m_iTokenValue = 0;
    ReadChar();
    ParseNumber(true);
  }
  else
  if(m_cCurrentChar == '/')  // Begins a comment (either line // or block /*...*/ comment)
  {
    ReadChar();
    ParseComment();
    ReadToken();    // Look again for the next token
  }
/*
  else
  if(m_cCurrentChar == 'XXXXXX')
  {
  }
*/
  else  // ERROR
  {
    m_eTokenType = TOK_ERROR;
    KMASSERTM( false, ("Syntax error in a MSB at line %d, column %d.", m_uTokenLine, m_uTokenColumn) );
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
void kmbTextMsbReader::ParseTag()
{
  while(isalpha(m_cCurrentChar) || isdigit(m_cCurrentChar) || (m_cCurrentChar == '_'))
  {
    m_sTokenValue += m_cCurrentChar;
    ReadChar();
  }

  if(m_sTokenValue.length() > 0)
  {
    m_eTokenType = TOK_TAG;
  }
  else
  {
    m_eTokenType = TOK_ERROR;   // ERROR: Invalid tag
  }
}

//----------------------------------------------------------------------------
/**
** Parses a string.
*/
void kmbTextMsbReader::ParseString()
{
  bool bStop = false;

  while(!bStop)
  {
    if(m_cCurrentChar == '"')
    {
      m_eTokenType = TOK_STRING;
      bStop        = true;

      ReadChar();
    }
    else
    if(m_pStream->IsAtEnd() || (m_cCurrentChar == '\n'))
    {
      m_eTokenType = TOK_ERROR;  // ERROR: New line character not allowed in a string
      bStop        = true;
    }
    else
    if(m_cCurrentChar == '\\')
    {
      ReadChar();

      if((m_cCurrentChar == '\\') || (m_cCurrentChar == '"'))
      {
        m_sTokenValue += m_cCurrentChar;
        ReadChar();
      }
      else
      {
        m_sTokenValue += '\\';    // Insert the leading '\' too. This allow strings like "C:\My Files\File.txt", but note that the '\' CANNOT be the last character.
        m_sTokenValue += m_cCurrentChar;
        ReadChar();
      }
    }
    else
    {
      m_sTokenValue += m_cCurrentChar;
      ReadChar();
    }
  }
}

//----------------------------------------------------------------------------
/**
** Parses a number.
*/
void kmbTextMsbReader::ParseNumber(bool bIsNegative)
{
  unsigned uNumDecimals = 0;

  // This check disallows a simple "-" or "-.1234" to be valid numbers
  if(bIsNegative && !isdigit(m_cCurrentChar))
  {
    m_eTokenType = TOK_ERROR;  // ERROR: Not a valid negative number
  }
  else
  {
    // Parse the integer number or the integer part of a real number
    while(isdigit(m_cCurrentChar))
    {
      m_iTokenValue = (m_iTokenValue * 10) + (m_cCurrentChar - '0');
      ReadChar();
    }

    // Is it a real number?
    if(m_cCurrentChar == '.')
    {
      char szBuffer[20];

      _snprintf(szBuffer, sizeof(szBuffer), "%d.", m_iTokenValue);
      szBuffer[sizeof(szBuffer) - 1] = '\0';

      m_sTokenValue = szBuffer;     // We temporally store the real number as a string

      ReadChar();

      // Parse the decimals of the real number (note that "123." is a valid real number, with its decimal part being 0)
      while(isdigit(m_cCurrentChar))
      {
        uNumDecimals++;
        m_sTokenValue += m_cCurrentChar;
        ReadChar();
      }

      // Convert the temporal string to a double
      m_dTokenValue = atof(m_sTokenValue.c_str());

      m_eTokenType = TOK_REAL;

      if(bIsNegative)
      {
        m_dTokenValue = -m_dTokenValue;
      }
    }
    else
    {
      // It is an integer number
      m_eTokenType = TOK_INTEGER;

      if(bIsNegative)
      {
        m_iTokenValue = -m_iTokenValue;
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
** Parses a comment.
*/
void kmbTextMsbReader::ParseComment()
{
  if(m_cCurrentChar == '/')
  {
    // This is a line comment
    ReadChar();

    while(m_cCurrentChar != '\n')
    {
      ReadChar();
    }

    ReadChar();
  }
  else
  if(m_cCurrentChar == '*')
  {
    // This is a block comment
    bool bStop = false;

    ReadChar();

    while(!bStop)
    {
      while(m_cCurrentChar != '*')
      {
        ReadChar();
      }

      ReadChar();

      if(m_cCurrentChar == '/')
      {
        bStop = true;
        ReadChar();
      }
    }
  }
  else
  {
    m_eTokenType = TOK_ERROR;  // ERROR
  }
}



//###########################################################################################
//#########################         SYNTACTICAL PARSER        ###############################
//###########################################################################################

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbReader::sMsb(kmbMsbPtr *pptrMsb)
{
  bool bRet = false;

  *pptrMsb = NULL;

  switch(m_eTokenType)
  {
  case TOK_INTEGER:
    *pptrMsb = kmbMsbManager::Singleton()->CreateIntegerMsb( m_iTokenValue );
    ReadToken();
    bRet = true;
    break;

  case TOK_REAL:
    *pptrMsb = kmbMsbManager::Singleton()->CreateRealMsb( m_dTokenValue );
    ReadToken();
    bRet = true;
    break;

  case TOK_STRING:
    *pptrMsb = kmbMsbManager::Singleton()->CreateStringMsb( m_sTokenValue );
    ReadToken();
    bRet = true;
    break;

  case TOK_BRACKET_OPEN:
    *pptrMsb = kmbMsbManager::Singleton()->CreateTaggedMsb();
    ReadToken();
    bRet = sTaggedSect( *pptrMsb );
    break;

  case TOK_PARENTHESIS_OPEN:
    *pptrMsb = kmbMsbManager::Singleton()->CreateEnumeratedMsb();
    ReadToken();
    bRet = sEnumSect( *pptrMsb );
    break;
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbReader::sTaggedSect(const kmbMsbPtr &ptrTaggedMsb)
{
  bool    bRet        = false;
  kmbMsbPtr ptrChildMsb = NULL;

  if(m_eTokenType == TOK_BRACKET_CLOSE)
  {
    ReadToken();
    bRet = true;
  }
  else
  {
    if(m_eTokenType == TOK_TAG)
    {
      std::string sTag;

      sTag = m_sTokenValue;
      ReadToken();

      if(m_eTokenType == TOK_EQUAL)   // The equal symbol is optional
      {
        ReadToken();
      }

      if( sMsb(&ptrChildMsb) )
      {
        // Insert the new Msb into the tagged Msb
        ptrTaggedMsb->AddChild(sTag, ptrChildMsb);

        if(sTaggedSect(ptrTaggedMsb))
        {
          bRet = true;
        }
      }
    }
  }

  return bRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
bool kmbTextMsbReader::sEnumSect(const kmbMsbPtr &ptrEnumMsb)
{
  bool    bRet        = false;
  kmbMsbPtr ptrChildMsb = NULL;

  if(m_eTokenType == TOK_PARENTHESIS_CLOSE)
  {
    ReadToken();
    bRet = true;
  }
  else
  {
    if( sMsb(&ptrChildMsb) )
    {
      // Insert the new Msb into the enumerated Msb
      ptrEnumMsb->AddChild( ptrChildMsb );

      if( sEnumSect(ptrEnumMsb) )
      {
        bRet = true;
      }
    }
  }

  return bRet;
}
