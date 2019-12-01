//-------------------------------------------------------------------------------------------
// File:        Msb.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "PCH_kmbBase.h"
#include "kmbMsb.h"
#include "File/kmbFile.h"
#include "Msb/kmbMsbManager.h"



//####################################################################################
//###############                 kmbMsb (base class)                   ################
//####################################################################################

//-------------------------------------------------------------------------------------------
// Usage:      Init
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

bool kmbMsb::Init()
{
  bool bRet = true;

  End();
  ResetVars();

  // Check parameters
  if(bRet)
  {
  }

  // Initialize class members
  if(bRet)
  {
  }


  if(bRet)
    m_bOk = true;
  else
    FreeVars();

  return bRet;
}


//-------------------------------------------------------------------------------------------
// Usage:      End
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ void kmbMsb::End()
{
  if( IsOk() )
  {
    FreeVars();
    m_bOk = false;
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      ResetVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void kmbMsb::ResetVars()
{

}


//-------------------------------------------------------------------------------------------
// Usage:      FreeVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void kmbMsb::FreeVars()
{

}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ kmbMsbPtr kmbMsb::GetChild(unsigned uIndex) const
{
  return kmbMsbManager::Singleton()->GetUniqueNullMsb();
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ kmbMsbPtr kmbMsb::GetChild(const std::string& sTag) const
{
  return kmbMsbManager::Singleton()->GetUniqueNullMsb();
}


//####################################################################################
//###############                      CNullMsb                       ################
//####################################################################################

//-------------------------------------------------------------------------------------------
// Usage:      Init
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

bool CNullMsb::Init()
{
  bool bRet = true;

  End();

  // Initialize base class
  if(bRet)
  {
    bRet = inherited::Init();
  }


  if(!bRet)
  {
    m_bOk = false;
  }

  return bRet;
}


//-------------------------------------------------------------------------------------------
// Usage:      End
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ void CNullMsb::End()
{
  if(IsOk())
  {
    //...
  }
}


//####################################################################################
//###############                      CIntegerMsb                    ################
//####################################################################################

//-------------------------------------------------------------------------------------------
// Usage:      Init
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

bool CIntegerMsb::Init(int iValue)
{
  bool bRet = true;

  End();
  ResetVars();

  // Initialize base class
  if(bRet)
  {
    bRet = inherited::Init();
  }

  // Initialize class members
  if(bRet)
  {
    m_iValue = iValue;
  }


  if(!bRet)
  {
    FreeVars();
    m_bOk = false;
  }

  return bRet;
}


//-------------------------------------------------------------------------------------------
// Usage:      End
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ void CIntegerMsb::End()
{
  if(IsOk())
  {
    FreeVars();
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      ResetVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CIntegerMsb::ResetVars()
{
  // Reset members of the base class
  inherited::ResetVars();

  // Reset members of this class
  m_iValue = 0;
}


//-------------------------------------------------------------------------------------------
// Usage:      FreeVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CIntegerMsb::FreeVars()
{
  // Free members of this class


  // Free members of the base class
  inherited::FreeVars();
}




//####################################################################################
//###############                      CRealMsb                    ################
//####################################################################################

//-------------------------------------------------------------------------------------------
// Usage:      Init
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

bool CRealMsb::Init(double dValue)
{
  bool bRet = true;

  End();
  ResetVars();

  // Initialize base class
  if(bRet)
  {
    bRet = inherited::Init();
  }

  // Initialize class members
  if(bRet)
  {
    m_dValue = dValue;
  }


  if(!bRet)
  {
    FreeVars();
    m_bOk = false;
  }

  return bRet;
}


//-------------------------------------------------------------------------------------------
// Usage:      End
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ void CRealMsb::End()
{
  if(IsOk())
  {
    FreeVars();
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      ResetVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CRealMsb::ResetVars()
{
  // Reset members of the base class
  inherited::ResetVars();

  // Reset members of this class
  m_dValue = 0.0;
}


//-------------------------------------------------------------------------------------------
// Usage:      FreeVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CRealMsb::FreeVars()
{
  // Free members of this class


  // Free members of the base class
  inherited::FreeVars();
}




//####################################################################################
//###############                      CStringMsb                    ################
//####################################################################################

//-------------------------------------------------------------------------------------------
// Usage:      Init
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

bool CStringMsb::Init(const std::string& sValue)
{
  bool bRet = true;

  End();
  ResetVars();

  // Initialize base class
  if(bRet)
  {
    bRet = inherited::Init();
  }

  // Initialize class members
  if(bRet)
  {
    m_sValue = sValue;
  }


  if(!bRet)
  {
    FreeVars();
    m_bOk = false;
  }

  return bRet;
}


//-------------------------------------------------------------------------------------------
// Usage:      End
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ void CStringMsb::End()
{
  if(IsOk())
  {
    FreeVars();
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      ResetVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CStringMsb::ResetVars()
{
  // Reset members of the base class
  inherited::ResetVars();

  // Reset members of this class
  m_sValue = "";
}


//-------------------------------------------------------------------------------------------
// Usage:      FreeVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CStringMsb::FreeVars()
{
  // Free members of this class


  // Free members of the base class
  inherited::FreeVars();
}




//####################################################################################
//###############                    CTaggedMsb                   ################
//####################################################################################

//-------------------------------------------------------------------------------------------
// Usage:      Init
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

bool CTaggedMsb::Init()
{
  bool bRet = true;

  End();
  ResetVars();

  // Initialize base class
  if(bRet)
  {
    bRet = inherited::Init();
  }

  // Initialize class members
  if(bRet)
  {
    m_lChildren.clear();
  }


  if(!bRet)
  {
    FreeVars();
    m_bOk = false;
  }

  return bRet;
}


//-------------------------------------------------------------------------------------------
// Usage:      End
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ void CTaggedMsb::End()
{
  if(IsOk())
  {
    FreeVars();
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      ResetVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CTaggedMsb::ResetVars()
{
  // Reset members of the base class
  inherited::ResetVars();

  // Reset members of this class
  m_lChildren.clear();
}


//-------------------------------------------------------------------------------------------
// Usage:      FreeVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CTaggedMsb::FreeVars()
{
  TMsbMap::iterator Iter;

  // Free members of this class
  m_lChildren.clear();   // This destroys every single kmbMsbPtr in it, which in turn removes the reference to their kmbMsb

  // Free members of the base class
  inherited::FreeVars();
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CTaggedMsb::AddChild(const std::string& sTag, kmbMsbPtr& ptrChildMsb)
{
  if(ptrChildMsb != NULL)
  {
    if(sTag.length() > 0)
    {
      m_lChildren[sTag] = ptrChildMsb;
    }
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------
/*virtual*/ void CTaggedMsb::RemoveChild(const std::string& sTag)
{
  TMsbMap::iterator Iter;
  Iter = m_lChildren.find( sTag );
  if (Iter != m_lChildren.end())
  {
    m_lChildren.erase( Iter );
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

unsigned CTaggedMsb::GetNumChildren() const
{
  return (unsigned) m_lChildren.size();
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

kmbMsbPtr CTaggedMsb::GetChild(const std::string& sTag) const
{
  TMsbMap::const_iterator Iter;
  Iter = m_lChildren.find( sTag );

  return ( Iter!=m_lChildren.end() ? Iter->second : kmbMsbManager::Singleton()->GetUniqueNullMsb() );
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CTaggedMsb::GetChildrenIterators(CTaggedMsb::TMsbMap::const_iterator* pIterBegin, CTaggedMsb::TMsbMap::const_iterator* pIterEnd) const
{
  if (pIterBegin != NULL)
  {
    *pIterBegin = m_lChildren.begin();
  }

  if (pIterEnd != NULL)
  {
    *pIterEnd = m_lChildren.end();
  }
}




//####################################################################################
//###############                    CEnumeratedMsb                   ################
//####################################################################################

//-------------------------------------------------------------------------------------------
// Usage:      Init
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

bool CEnumeratedMsb::Init()
{
  bool bRet = true;

  End();
  ResetVars();

  // Initialize base class
  if(bRet)
  {
    bRet = inherited::Init();
  }

  // Initialize class members
  if(bRet)
  {
    m_lChildren.clear();
  }


  if(!bRet)
  {
    FreeVars();
    m_bOk = false;
  }

  return bRet;
}


//-------------------------------------------------------------------------------------------
// Usage:      End
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

/*virtual*/ void CEnumeratedMsb::End()
{
  if(IsOk())
  {
    FreeVars();
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      ResetVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CEnumeratedMsb::ResetVars()
{
  // Reset members of the base class
  inherited::ResetVars();

  // Reset members of this class
  m_lChildren.clear();
}


//-------------------------------------------------------------------------------------------
// Usage:      FreeVars
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CEnumeratedMsb::FreeVars()
{
  std::vector<kmbMsb *>::iterator Iter;

  // Free members of this class
  m_lChildren.clear();   // This destroys every single kmbMsbPtr in it, which in turn removes the reference to their kmbMsb

  // Free members of the base class
  inherited::FreeVars();
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

void CEnumeratedMsb::AddChild(kmbMsbPtr& ptrChildMsb)
{
  if(ptrChildMsb != NULL)
  {
    m_lChildren.push_back( ptrChildMsb );
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------
/*virtual*/ void CEnumeratedMsb::RemoveChild(unsigned uIndex)
{
  if (uIndex < m_lChildren.size())
  {
    m_lChildren.erase( m_lChildren.begin() + uIndex );
  }
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

unsigned CEnumeratedMsb::GetNumChildren() const
{
  return (unsigned) m_lChildren.size();
}


//-------------------------------------------------------------------------------------------
// Usage:      
// Return:     
// Parameters: 
//-------------------------------------------------------------------------------------------

kmbMsbPtr CEnumeratedMsb::GetChild(unsigned uIndex) const
{
  kmbMsbPtr ptrRet( NULL );

  if(uIndex < GetNumChildren())
  {
    ptrRet = m_lChildren[uIndex];
  }
  else
  {
    ptrRet = kmbMsbManager::Singleton()->GetUniqueNullMsb();
  }

  return ptrRet;
}
