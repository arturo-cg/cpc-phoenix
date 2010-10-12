//-------------------------------------------------------------------------------------------
// File:        MemoryBlock.cpp
//
// Description: 
//-------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "cpcMemoryBlock.h"


namespace CPC {



  //----------------------------------------------------------------------------
  /**
  ** 
  */
  CMemoryBlock::CMemoryBlock()
  {
    ResetVars();

  }

  //----------------------------------------------------------------------------
  /**
  ** ResetVars
  */
  void CMemoryBlock::ResetVars()
  {

  }

  //----------------------------------------------------------------------------
  /**
  ** FreeVars
  */
  void CMemoryBlock::FreeVars()
  {

  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  cpcByte CMemoryBlock::ReadByte(cpcWord nAddress) const
  {
    return m_anBytes[nAddress & LAST_BYTE];
  }

  //----------------------------------------------------------------------------
  /**
  ** 
  */
  void CMemoryBlock::WriteByte(cpcWord nAddress, cpcByte nValue)
  {
    m_anBytes[nAddress & LAST_BYTE] = nValue;
  }

} //namespace CPC
