//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
** 
*/
inline void kmbPrecisionTimer::Read(kmbPrecisionTimer::Value* pValue) const
{
  if (pValue != NULL)
  {
    QueryPerformanceCounter( (LARGE_INTEGER *) &pValue->m_i64Value );
  }
}

//----------------------------------------------------------------------------
/**
** 
*/
inline double kmbPrecisionTimer::ComputeElapsedSecs(const kmbPrecisionTimer::Value& from, const kmbPrecisionTimer::Value& to) const
{
  return ( double(to.m_i64Value - from.m_i64Value) / m_i64TimerFrequency );
}

//----------------------------------------------------------------------------
/**
** 
*/
inline double kmbPrecisionTimer::ComputeElapsedMsecs(const kmbPrecisionTimer::Value& from, const kmbPrecisionTimer::Value& to) const
{
  return ( ComputeElapsedSecs(from, to) * 1000.f );
}

//----------------------------------------------------------------------------
/**
** 
*/
inline double kmbPrecisionTimer::ComputeElapsedUsecs(const kmbPrecisionTimer::Value& from, const kmbPrecisionTimer::Value& to) const
{
    return ( ComputeElapsedSecs(from, to) * 1000000.f );
}
