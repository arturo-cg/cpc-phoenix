//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBOUTPUTSTREAM_H_
#define _KMBOUTPUTSTREAM_H_


#include "kmbStreamTypes.h"


/**
** Base class of all classes representing a binary stream to which bytes of data can be written.
** There are derived classes for writing to a file, a memory buffer, etc.
*/
class kmbOutputStream
{
public:

  virtual                  ~kmbOutputStream           ()  { End(); }

  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  /** Returns the specific type of this kmbOutputStream-derived stream. */
  virtual kmbEStreamType    GetType                   () const = 0;

  /** Returns the count of bytes written so far. */
  unsigned                  GetWrittenBytesCount      () const  { return m_uWrittenBytesCount; }

  /** Writes a variable. The variable size must be known at compile time; i.e., sizeof(pVar) must return
  *** the correct size (built-in types, structs made of built-in types, etc.). */
  template <typename T>
  bool                      Write                     (const T& Var);

  /** Writes the given number of bytes. */
  virtual bool              Write                     (const void* pBuffer, unsigned uNumBytes) = 0;

  /** Writes the specified number of characters of a string.
  *** Note: Characters are written one by one as they appear in the string (i.e. no charset or end-of-line character conversion are performed)
  ***       and they are assumed to be encoded in ASCII. */
  bool                      WriteString               (const char* pszString, unsigned nCharCount);

  /** Writes a std::string. No special end-of-string character is written. See additional note in WriteString(const char*, unsigned) method. */
  bool                      WriteString               (const string& sString);


protected:

  // Constructor and Init() are protected to avoid instantiating this class
                            kmbOutputStream           ()  { m_bOk = false; }
  bool                      Init                      ();


  bool                      m_bOk;
  unsigned                  m_uWrittenBytesCount;


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();

};

#include "kmbOutputStream.inl"


#endif // _KMBOUTPUTSTREAM_H_
