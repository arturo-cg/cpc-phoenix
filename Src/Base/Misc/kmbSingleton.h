//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include "Debug/kmbAsserts.h"


/**
** Base class that makes derived classes behave as Singletons. That is, only one instance of the class can exist.
*/
template <typename T>
class kmbSingleton
{
public:

  /// Destructor
  virtual ~kmbSingleton()
  {
    m_pSingleton = NULL;
  }

  /// Returns true if the Singleton instance is already created, or false otherwise.
  static inline bool IsSingletonCreated()
  {
    return (m_pSingleton != NULL);
  }

  /// Returns a pointer to the Singleton instance
  static inline T* Singleton()
  {
    return m_pSingleton;
  }


protected:

  /// Constructor
  kmbSingleton()
  {
    m_pSingleton = static_cast<T*>( this );
  }


private:

  static T* m_pSingleton;             /// The Singleton instance

};

#endif // _SINGLETON_H_
