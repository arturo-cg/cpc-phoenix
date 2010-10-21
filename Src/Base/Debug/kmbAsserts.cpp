//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"


#ifdef ASSERTS_ENABLED


#define MAX_BUFFER_LENGTH 2000


//----------------------------------------------------------------------------
/**
** 
*/
int _kmShowAssert(const char *pszCondition, const char *pszMessage, const char *pszFilename, unsigned uLine)
{
  int iRet = -1;

  if((pszCondition != NULL) && (pszFilename != NULL))
  {
    char szBuffer[MAX_BUFFER_LENGTH];
    char szMessageBuffer[MAX_BUFFER_LENGTH];
    int  iButtonPressed;

    if(pszMessage != NULL)
    {
      _snprintf(szMessageBuffer, MAX_BUFFER_LENGTH, "\n%s\n", pszMessage);
      szMessageBuffer[MAX_BUFFER_LENGTH - 1] = '\0';
    }
    else
    {
      szMessageBuffer[0] = '\0';
    }


    _snprintf( szBuffer, MAX_BUFFER_LENGTH, "Condition: %s\n"
                                            "File: %s\n"
                                            "Line: %u\n"
                                            "%s"
                                            "\n"
                                            "[Yes] Break    [No] Ignore    [Cancel] Ignore always",
                                            pszCondition,
                                            pszFilename,
                                            uLine,
                                            szMessageBuffer );
    szBuffer[MAX_BUFFER_LENGTH - 1] = '\0';
    iButtonPressed = ::MessageBox(NULL, szBuffer, "ASSERTION", MB_YESNOCANCEL);

    switch (iButtonPressed)
    {
      case IDYES:     iRet = 0; break;
      case IDNO:      iRet = 1; break;
      case IDCANCEL:  iRet = 2; break;
    }
  }

  return iRet;
}

//----------------------------------------------------------------------------
/**
** 
*/
const char* _kmExpandAssertFmtMessage( const char* pszFmtMessage, ... )
{
  static char pszExpandedMessage[MAX_BUFFER_LENGTH];

  va_list args;
  va_start( args, pszFmtMessage );

  vsnprintf( pszExpandedMessage, sizeof(pszExpandedMessage) - 1, pszFmtMessage, args );

  return pszExpandedMessage;
}


#endif // ASSERTS_ENABLED
