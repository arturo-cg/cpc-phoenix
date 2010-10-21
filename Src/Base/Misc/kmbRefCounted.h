//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _REFCOUNTED_H_
#define _REFCOUNTED_H_


/**
** Provides a reference counter to objects of any class that inherits from CRefCounted.
** Use it in conjunction with kmbRefCountedPtr to provide automatic destruction of objects
** that are no longer referenced by anyone.
*/
class kmbRefCounted
{
public:

  virtual                  ~kmbRefCounted             ()  { ASSERTM( m_uReferenceCount == 0, ("Reference-counted object being deleted but there still exist references to it!!") ); }

  /**
  ** Adds a reference to this object. Don't call it directly, it is used by kmbRefCountedPtr-derived classes.
  ** @return The number of references to this object after the call.
  */
  unsigned                  _AddReference             ()  { return ++m_uReferenceCount; }
  /**
  ** Removes a reference to this object. Don't call it directly, it is used by CRefCounted-Ptr-derived classes.
  ** @return The number of references to this object after the call.
  */
  unsigned                  _RemoveReference          ()  { ASSERT(m_uReferenceCount > 0); return --m_uReferenceCount; }


protected:

  // Protected constructor to avoid instantiating this class.
                            kmbRefCounted             () : m_uReferenceCount( 0 )  { }


private:

  unsigned                  m_uReferenceCount;

};


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

/**
** Default accesor class for reference-counted objects. Use it for referenced-counted objects
** that are created with the macro KMBNEW and therefore should be destroyed with the macro KMBDISPOSE.
*/
template <typename OBJ_CLASS>
class kmbDefaultRefCountedAccessor
{
public:
    virtual void AddReference(OBJ_CLASS *pObject)
    {
        pObject->_AddReference();
    }

    virtual void RemoveReference(OBJ_CLASS *pObject)
    {
        if(pObject->_RemoveReference() == 0)
        {
          KMBDISPOSE( pObject );
        }
    }
};



//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

/**
** Template class for 'smart' pointers to reference-counted objects.
** \p
** It encapsulates the process of adding/removing references to reference-counted objects
** when pointers to them are copied around, passed as function parameter, returned from a
** function, etc.
** \p
** There must be a specialized kmbRefCountedPtr-derived class for each CRefCounted-derived class
** because those classes may have different requirements for adding/removing references to their
** objects.
*/
template < typename OBJ_CLASS, typename ACCESSOR_CLASS = kmbDefaultRefCountedAccessor<OBJ_CLASS> >
class kmbRefCountedPtr
{
public:

  /// Default constructor. Initializes the pointer to NULL.
  kmbRefCountedPtr()
  {
    m_pObject = NULL;
  }

  /// Copy constructor. Initializes the pointer so it points to the given object.
  kmbRefCountedPtr(OBJ_CLASS *pObject)
  {
    // Store the pointer to the object
    m_pObject = pObject;

    // Add a reference to the new object (taking into account that it could be NULL)
    if(pObject != NULL)
    {
      m_RefAccessor.AddReference(pObject);
    }
  }

  /// Copy constructor. Initializes the pointer so it points to the same object that the given kmbRefCountedPtr points to.
  kmbRefCountedPtr(const kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> &AnotherPtr)
  {
    // Store the pointer to the object
    m_pObject = AnotherPtr.m_pObject;

    // Add a reference to the new object (taking into account that it could be NULL)
    if(AnotherPtr.m_pObject != NULL)
    {
      m_RefAccessor.AddReference(AnotherPtr.m_pObject);
    }
  }

  /// Destructor.
  virtual ~kmbRefCountedPtr()
  {
    if(m_pObject != NULL)
    {
      m_RefAccessor.RemoveReference(m_pObject);
    }
  }

  /// Returns a reference to the encapsulated object.
  OBJ_CLASS &operator*() const    { return *m_pObject; }
  /// Returns a pointer to the encapsulated object.
  OBJ_CLASS *operator->() const   { return m_pObject; }
  /// Returns a pointer to the encapsulated object.
  OBJ_CLASS       *Get()          { return m_pObject; }
  const OBJ_CLASS *Get() const    { return m_pObject; }

  /*
  ** Assignment operator that accepts a pointer to a object (it can be NULL).
  ** @remarks
  **    This operator DOES NOT add a reference to the object, it is assumed it has already been done
  **    just before assigning it to this kmbRefCountedPtr. This would be the case for the following
  **    example, which is the most typical use of this version of the operator:
  **
  **        MyMaterialPtr = kmgMaterialManager::Singleton()->GrabMaterial("MyMaterial");
  **
  **    Here, the method kmgMaterialManager::GrabMaterial() already adds a reference to the material.
  */
  kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> &operator=(OBJ_CLASS *pObject)
  {
    if(m_pObject != pObject)
    {
      OBJ_CLASS *pOld = m_pObject;

      // Store the pointer to the object
      m_pObject = pObject;

      // Add a reference to the new object (taking into account that it could be NULL)
      if(pObject != NULL)
      {
        m_RefAccessor.AddReference(pObject);
      }

      // Remove the reference to the previous object
      if(pOld != NULL)
      {
        m_RefAccessor.RemoveReference(pOld);
      }
    }

    return *this;
  }

  /// Asssignment operator that accepts another kmbRefCountedPtr.
  kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> &operator=(const kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> &AnotherPtr)
  {
    return operator=(AnotherPtr.m_pObject);
  }

  /// Equality operator. The parameter can be NULL.
  bool operator==(const OBJ_CLASS *pObject) const                                      { return (m_pObject == pObject); }
  /// Equality operator.
  bool operator==(const kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> &AnotherPtr) const  { return (m_pObject == AnotherPtr.m_pObject); }

  /// Inequality operator. The parameter can be NULL.
  bool operator!=(const OBJ_CLASS *pObject) const                                      { return (m_pObject != pObject); }
  /// Inequality operator.
  bool operator!=(const kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> &AnotherPtr) const  { return (m_pObject != AnotherPtr.m_pObject); }

/*
  /// Casts to a pointer to the class of the object. This makes possible to use a kmbRefCountedPtr anywhere a pointer to the object is expected.
  operator OBJ_CLASS*() const   { return m_pObject; }
  operator OBJ_CLASS*()         { return m_pObject; }
*/

  /// Routes the operator [] that receives a STL string to the encapsulated object.
  /// TODO - This solution is too specific, a more general solution is needed.
  kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> operator[](const string& sStr)
  {
    return m_pObject->operator[]( sStr );
  }

  const kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> operator[](const string& sStr) const
  {
    return m_pObject->operator[]( sStr );
  }

  /// Routes the operator () that receives an unsigned to the encapsulated object.
  /// TODO - This solution is too specific, a more general solution is needed.
  kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> operator()(unsigned i)
  {
    return m_pObject->operator()( i );
  }

  const kmbRefCountedPtr<OBJ_CLASS,ACCESSOR_CLASS> operator()(unsigned i) const
  {
    return m_pObject->operator()( i );
  }


private:

  OBJ_CLASS      *m_pObject;
  ACCESSOR_CLASS  m_RefAccessor;

};


#endif // _REFCOUNTED_H_
