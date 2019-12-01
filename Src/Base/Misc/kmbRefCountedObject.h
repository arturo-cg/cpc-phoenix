//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBREFCOUNTEDOBJECT_H_
#define _KMBREFCOUNTEDOBJECT_H_


/**
** Provides a reference counter to objects of any class that inherits from this class.
** Use it in conjunction with kmbSmartPtr to provide automatic destruction of objects
** that are no longer referenced by anyone.
** The template parameter DELETE_FUNC is a functor that, when invoked, deletes the
** kmbRefCountedObject passed as parameter. It is invoked by the object itself when
** its reference count goes down to 0. This allows the object to customize the way
** it is deleted, i.e. it may need to unregister from its manager or something similar.
** A default implementation is provided that simply applies KMBDELETE on the object.
*/
template <typename DELETE_FUNC>
class kmbRefCountedObject
{
public:

  virtual ~kmbRefCountedObject()
  {
    KMASSERTM( m_uReferenceCount == 0, ("Reference-counted object being deleted but there still exist references to it!!") );
  }

  /** Returns the current reference count. */
  unsigned GetReferenceCount() const
  {
    return m_uReferenceCount;
  }

  /** Adds a reference to this object. There's no need to call it directly when referenced through a kmbSmartPtr.
  ** @return The number of references to this object after the call.
  */
  unsigned _AddReference()
  {
    return ++m_uReferenceCount;
  }

  /** Removes a reference to this object. There's no need to call it directly when referenced through a kmbSmartPtr.
  ** @return The number of references to this object after the call.
  */
  unsigned _RemoveReference()
  {
    KMASSERT(m_uReferenceCount > 0);
    unsigned uRet = --m_uReferenceCount;
    if (uRet == 0)
    {
      // Delete the object
      DELETE_FUNC deleteFunc;
      deleteFunc( this );
    }
    return uRet;     // This is safe because uRet is on the stack
  }


protected:

  // Protected constructor to avoid instantiating this class.
  kmbRefCountedObject() : m_uReferenceCount( 0 )
  {
    //...
  }


private:

  unsigned m_uReferenceCount;

};


/** Default implementation of the DELETE_FUNC functor used by the kmbRefCountedObject class that
**  simply applies KMBDELETE on the object passed as parameter. */
class _kmbDeleteRefCountedObject
{
public:
  template <typename DELETE_FUNC> void operator() (kmbRefCountedObject<DELETE_FUNC>* pObject)
  {
    delete pObject;
  }
};


#endif // _KMBREFCOUNTEDOBJECT_H_
