//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBSMARTPTR_H_
#define _KMBSMARTPTR_H_


#include "kmbRefCountedObject.h"


/**
** Template class for 'smart' pointers to reference-counted objects.
** \p
** It encapsulates the process of adding/removing references to reference-counted objects
** when pointers to them are copied around, passed as function parameter, returned from a
** function, etc.
** \p
** There must be a specialized kmbSmartPtr-derived class for each CRefCounted-derived class
** because those classes may have different requirements for adding/removing references to their
** objects.
*/
template <class TARGET_CLASS>
class kmbSmartPtr
{
public:

  /// Default constructor. Initializes the pointer to NULL.
  kmbSmartPtr()
  {
    m_pObject = NULL;
  }

  /// Copy constructor. Initializes the pointer so it points to the given object.
  kmbSmartPtr(TARGET_CLASS* pObject)
  {
    // Store the pointer to the object
    m_pObject = pObject;

    // Add a reference to the new object (taking into account that it could be NULL)
    if (pObject != NULL)
    {
      pObject->_AddReference();
    }
  }

  /// Copy constructor. Initializes the pointer so it points to the same object that the given kmbSmartPtr points to.
  kmbSmartPtr(const kmbSmartPtr<TARGET_CLASS>& anotherPtr)
  {
    // Store the pointer to the object
    m_pObject = anotherPtr.m_pObject;

    // Add a reference to the new object (taking into account that it could be NULL)
    if (anotherPtr.m_pObject != NULL)
    {
      anotherPtr.m_pObject->_AddReference();
    }
  }

  /// Destructor.
  virtual ~kmbSmartPtr()
  {
    if (m_pObject != NULL)
    {
      m_pObject->_RemoveReference();
    }
  }

  /// Returns a reference to the encapsulated object.
  TARGET_CLASS& operator*() const    { return *m_pObject; }
  /// Returns a pointer to the encapsulated object.
  TARGET_CLASS* operator->() const   { return m_pObject; }
  /// Returns a pointer to the encapsulated object.
  TARGET_CLASS*       Get()          { return m_pObject; }
  const TARGET_CLASS* Get() const    { return m_pObject; }
  /// Cast operator to pointer to the encapsulated object type.
  operator       TARGET_CLASS* ()        { return m_pObject; }
  operator const TARGET_CLASS* () const  { return m_pObject; }

  /*
  ** Assignment operator that accepts a pointer to a object (it can be NULL).
  */
  kmbSmartPtr<TARGET_CLASS>& operator=(TARGET_CLASS* pObject)
  {
    if (m_pObject != pObject)
    {
      TARGET_CLASS* pOld = m_pObject;

      // Store the pointer to the object
      m_pObject = pObject;

      // Add a reference to the new object (taking into account that it could be NULL)
      if (pObject != NULL)
      {
        pObject->_AddReference();
      }

      // Remove the reference to the previous object
      if (pOld != NULL)
      {
        pOld->_RemoveReference();
      }
    }

    return* this;
  }

  /// Asssignment operator that accepts another kmbSmartPtr.
  kmbSmartPtr<TARGET_CLASS>& operator=(const kmbSmartPtr<TARGET_CLASS>& anotherPtr)
  {
    return operator=( anotherPtr.m_pObject );
  }

  /// Equality operator. The parameter can be NULL.
  bool operator==(const TARGET_CLASS* pObject) const                                      { return (m_pObject == pObject); }
  /// Equality operator.
  bool operator==(const kmbSmartPtr<TARGET_CLASS>& anotherPtr) const                      { return (m_pObject == anotherPtr.m_pObject); }

  /// Inequality operator. The parameter can be NULL.
  bool operator!=(const TARGET_CLASS* pObject) const                                      { return (m_pObject != pObject); }
  /// Inequality operator.
  bool operator!=(const kmbSmartPtr<TARGET_CLASS>& anotherPtr) const                      { return (m_pObject != anotherPtr.m_pObject); }

  /// Routes the operator [] that receives a STL string to the encapsulated object.
  /// TODO - This solution is too specific, a more general solution is needed.
  kmbSmartPtr<TARGET_CLASS> operator[](const string& sStr)
  {
    return m_pObject->operator[]( sStr );
  }

  const kmbSmartPtr<TARGET_CLASS> operator[](const string& sStr) const
  {
    return m_pObject->operator[]( sStr );
  }

  /// Routes the operator () that receives an unsigned to the encapsulated object.
  /// TODO - This solution is too specific, a more general solution is needed.
  kmbSmartPtr<TARGET_CLASS> operator()(unsigned i)
  {
    return m_pObject->operator()( i );
  }

  const kmbSmartPtr<TARGET_CLASS> operator()(unsigned i) const
  {
    return m_pObject->operator()( i );
  }


private:

  TARGET_CLASS* m_pObject;

};


#endif // _KMBSMARTPTR_H_
