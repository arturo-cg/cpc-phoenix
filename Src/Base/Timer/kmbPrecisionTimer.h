//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBPRECISIONTIMER_H_
#define _KMBPRECISIONTIMER_H_


/**
** A high-precision system timer.
*/
class kmbPrecisionTimer
{
public:

  /**
  ** A value read from a kmbPrecisionTimer.
  **
  ** This class abstracts the user code from the actual low-level value type
  ** used by the platform-specific precision timer.
  */
  class Value
  {
    friend class kmbPrecisionTimer;

  public:

                              Value                     ()  { m_i64Value = 0; }


  private:

    __int64                   m_i64Value;

  };



                            kmbPrecisionTimer         ()  { m_bOk = false; }
  virtual                  ~kmbPrecisionTimer         ()  { End(); }

  bool                      Init                      ();
  virtual void              End                       ();
  bool                      IsOk                      () const  { return m_bOk; }

  /** Reads the current timer value. */
  void                      Read                      (Value* pValue) const;

  /** Computes the elapsed time in seconds between the two kmbPrecisionTimer values. */
  double                    ComputeElapsedSecs        (const Value& from, const Value& to) const;
  /** Computes the elapsed time in milliseconds between the two kmbPrecisionTimer values. */
  double                    ComputeElapsedMsecs       (const Value& from, const Value& to) const;
  /** Computes the elapsed time in microseconds between the two kmbPrecisionTimer values. */
  double                    ComputeElapsedUsecs       (const Value& from, const Value& to) const;


private:

  void                      ResetVars                 ();
  void                      FreeVars                  ();


  bool                      m_bOk;

  __int64                   m_i64TimerFrequency;

};

#include "kmbPrecisionTimer.inl"


#endif // _KMBPRECISIONTIMER_H_
