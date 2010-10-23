//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#ifndef _ASSERTS_H_
#define _ASSERTS_H_

#ifdef ASSERTS_ENABLED


/** Simple assert macro.
***
*** Example:
***   KMASSERT( m_bIsOk );
*/
#define KMASSERT(condition) \
  { \
    static bool bIgnoreAlways = false; \
    if ( !(condition) && !bIgnoreAlways ) \
    { \
      switch ( ::_kmShowAssert(#condition, NULL, __FILE__, __LINE__) ) \
      { \
        case 0:  __debugbreak(); break;        /* Break the execution. */ \
        case 1:  break;                        /* Ignore this assert (only this time). */ \
        case 2:  bIgnoreAlways = true; break;  /* Always ignore this assert (this assert will never show up again). */ \
      } \
    } \
  }


/** Assert macro with printf-like message.
***
*** Examples:
***   KMASSERTM( m_pFile->CanWrite(), ("The file must be open with write permissions.") );
***   KMASSERTM( m_pFile->CanWrite(), ("The file %s must be open with write permissions.", m_pFile->GetFileName()) );
*/

#define KMASSERTM(condition, fmt_message) \
  { \
    static bool bIgnoreAlways = false; \
    if ( !(condition) && !bIgnoreAlways ) \
    { \
      switch ( ::_kmShowAssert(#condition, _kmExpandAssertFmtMessage fmt_message, __FILE__, __LINE__) ) \
      { \
        case 0:  __debugbreak(); break;        /* Break the execution. */ \
        case 1:  break;                        /* Ignore this assert (only this time). */ \
        case 2:  bIgnoreAlways = true; break;  /* Always ignore this assert (this assert will never show up again). */ \
      } \
    } \
  }


// Not to be used directly
int _kmShowAssert(const char *pszCondition, const char *pszMessage, const char *pszFilename, unsigned uLine);
const char* _kmExpandAssertFmtMessage( const char* pszFmtMessage, ... );

#else

#define KMASSERT(condition)
#define KMASSERTM(condition, fmt_message)

#endif // ASSERTS_ENABLED


#endif // _ASSERTS_H_
