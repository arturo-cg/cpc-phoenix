//-------------------------------------------------------------------------------------------
// File:        Msb.h
//
// Description: 
//-------------------------------------------------------------------------------------------

#ifndef _MSB_H_
#define _MSB_H_


#include "Misc/kmbRefCountedObject.h"
#include "Misc/kmbSmartPtr.h"
#include <unordered_map>


class kmbMsb;
class CIntegerMsb;
class CRealMsb;
class CStringMsb;
class CTaggedMsb;
class CEnumeratedMsb;
class kmbFile;


// Smart pointer to kmbMsb
typedef kmbSmartPtr<kmbMsb> kmbMsbPtr;


//####################################################################################
//###############                 kmbMsb (base class)                   ################
//####################################################################################
class kmbMsb : public kmbRefCountedObject<_kmbDeleteRefCountedObject>
{
public:

  enum EType
  {
    TYPE_INTEGER = 0,
    TYPE_NULL,
    TYPE_REAL,
    TYPE_STRING,
    TYPE_TAGGED,
    TYPE_ENUMERATED,

    TYPE_LAST,
    TYPE_INVALID = 0x7FFFFFFF
  };

  virtual                  ~kmbMsb                    ()  { End(); }

  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  //
  // To get the type and value of this Msb
  //

  virtual EType             GetType                   () const = 0;
  bool                      IsNull                    () const  { return (GetType() == TYPE_NULL); }

  virtual bool              GetBool                   (bool bDefaultValue = false) const                      { return bDefaultValue; }
  virtual int               GetInt                    (int iDefaultValue = 0) const                           { return iDefaultValue; }
  virtual unsigned          GetUnsigned               (unsigned uDefaultValue = 0) const                      { return uDefaultValue; }
  virtual float             GetFloat                  (float fDefaultValue = 0.f) const                       { return fDefaultValue; }
  virtual double            GetDouble                 (double dDefaultValue = 0.0) const                      { return dDefaultValue; }
  virtual std::string       GetString                 (const std::string& sDefaultValue = std::string("")) const        { return sDefaultValue; }

  //
  // Child management
  //

  virtual unsigned          GetNumChildren            () const                                           { return 0; }

          bool              HasChild                  (unsigned uIndex) const  { return (uIndex < GetNumChildren()); }
  virtual kmbMsbPtr         GetChild                  (unsigned uIndex) const;
  const   kmbMsbPtr         operator()                (unsigned uIndex) const  { return GetChild(uIndex); }
  virtual void              AddChild                  (kmbMsbPtr &ptrChildMsb)  { KMASSERTM( false, ("This MSB cannot have children.") ); }
  virtual void              RemoveChild               (unsigned uIndex)         { KMASSERTM( false, ("This MSB doesn't have children.") ); }

          bool              HasChild                  (const std::string& sTag) const  { return ( !GetChild(sTag)->IsNull() ); }
  virtual kmbMsbPtr         GetChild                  (const std::string& sTag) const;
  const   kmbMsbPtr         operator[]                (const std::string& sTag) const  { return GetChild(sTag); }
  virtual void              AddChild                  (const std::string& sTag, kmbMsbPtr &ptrChildMsb)  { KMASSERTM( false, ("This MSB cannot have children.") ); }
  virtual void              RemoveChild               (const std::string& sTag)                          { KMASSERTM( false, ("This MSB doesn't have children.") ); }


protected:

  // Constructor and Init() are protected to avoid instantiating this class
                            kmbMsb                    ()  { m_bOk = false; }
  bool                      Init                      ();

  void                      ResetVars                 ();
  void                      FreeVars                  ();


  bool                      m_bOk;


private:

};


//####################################################################################
//###############                      CNullMsb                       ################
//####################################################################################

class CNullMsb : public kmbMsb
{
public:

                            CNullMsb                  ()  { }
  virtual                  ~CNullMsb                  ()  { End(); }

  bool                      Init                      ();
  virtual void              End                       ();

  virtual EType             GetType                   () const  { return TYPE_NULL; }


private:

  typedef                   kmbMsb                      inherited;

};


//####################################################################################
//###############                      CIntegerMsb                    ################
//####################################################################################

class CIntegerMsb : public kmbMsb
{
public:

                            CIntegerMsb               ()  { }
  virtual                  ~CIntegerMsb               ()  { End(); }

  bool                      Init                      (int iValue);
  virtual void              End                       ();

  //
  // To get the type and value of this Msb
  //

  virtual EType             GetType                   () const  { return TYPE_INTEGER; }

  virtual bool              GetBool                   (bool bDefaultValue = false) const  { return (m_iValue != 0) ? true : false; }
  virtual int               GetInt                    (int iDefaultValue = 0) const       { return m_iValue; }
  virtual unsigned          GetUnsigned               (unsigned uDefaultValue = 0) const  { return (unsigned) m_iValue; }
  virtual float             GetFloat                  (float fDefaultValue = 0.f) const   { return (float) m_iValue; }
  virtual double            GetDouble                 (double dDefaultValue = 0.0) const  { return (double) m_iValue; }


protected:

  void                      ResetVars                 ();
  void                      FreeVars                  ();


private:

  typedef                   kmbMsb                      inherited;


  int                       m_iValue;

};


//####################################################################################
//###############                      CRealMsb                    ################
//####################################################################################

class CRealMsb : public kmbMsb
{
public:

                            CRealMsb                  ()  { }
  virtual                  ~CRealMsb                  ()  { End(); }

  bool                      Init                      (double dValue);
  virtual void              End                       ();

  //
  // To get the type and value of this Msb
  //

  virtual EType             GetType                   () const  { return TYPE_REAL; }

  virtual float             GetFloat                  (float fDefaultValue = 0.f) const   { return (float) m_dValue; }
  virtual double            GetDouble                 (double dDefaultValue = 0.0) const  { return         m_dValue; }


protected:

  void                      ResetVars                 ();
  void                      FreeVars                  ();


private:

  typedef                   kmbMsb                      inherited;


  double                    m_dValue;

};


//####################################################################################
//###############                      CStringMsb                    ################
//####################################################################################

class CStringMsb : public kmbMsb
{
public:

                            CStringMsb                ()  { }
  virtual                  ~CStringMsb                ()  { End(); }

  bool                      Init                      (const std::string& sValue);
  virtual void              End                       ();

  //
  // To get the type and value of this Msb
  //

  virtual EType             GetType                   () const  { return TYPE_STRING; }

  virtual std::string       GetString                 (const std::string& sDefaultValue) const  { return m_sValue; }


protected:

  void                      ResetVars                 ();
  void                      FreeVars                  ();


private:

  typedef                   kmbMsb                      inherited;


  std::string               m_sValue;

};


//####################################################################################
//###############                    CTaggedMsb                   ################
//####################################################################################

class CTaggedMsb : public kmbMsb
{
public:

  typedef                   std::unordered_map<std::string, kmbMsbPtr> TMsbMap;


                            CTaggedMsb                ()  { }
  virtual                  ~CTaggedMsb                ()  { End(); }

  bool                      Init                      ();
  virtual void              End                       ();

  virtual EType             GetType                   () const  { return TYPE_TAGGED; }


  //
  // Child management
  //

  virtual unsigned          GetNumChildren            () const;
  virtual kmbMsbPtr         GetChild                  (const std::string& sTag) const;
  void                      GetChildrenIterators      (CTaggedMsb::TMsbMap::const_iterator* pIterBegin, CTaggedMsb::TMsbMap::const_iterator* pIterEnd) const;
  virtual void              AddChild                  (const std::string& sTag, kmbMsbPtr& ptrChildMsb);
  virtual void              RemoveChild               (const std::string& sTag);


protected:

  void                      ResetVars                 ();
  void                      FreeVars                  ();


private:

  typedef                   kmbMsb                    inherited;


  TMsbMap                   m_lChildren;

};


//####################################################################################
//###############                    CEnumeratedMsb                   ################
//####################################################################################

class CEnumeratedMsb : public kmbMsb
{
public:

                            CEnumeratedMsb            ()  { }
  virtual                  ~CEnumeratedMsb            ()  { End(); }

  bool                      Init                      ();
  virtual void              End                       ();

  virtual EType             GetType                   () const  { return TYPE_ENUMERATED; }


  //
  // Child management
  //

  virtual unsigned          GetNumChildren            () const;
  virtual kmbMsbPtr         GetChild                  (unsigned uIndex) const;
  virtual void              AddChild                  (kmbMsbPtr& ptrChildMsb);
  virtual void              RemoveChild               (unsigned uIndex);


protected:

  void                      ResetVars                 ();
  void                      FreeVars                  ();


private:

  typedef                   kmbMsb                      inherited;
  typedef                   std::vector<kmbMsbPtr>           TMsbList;


  TMsbList                  m_lChildren;

};


#endif // _MSB_H_
