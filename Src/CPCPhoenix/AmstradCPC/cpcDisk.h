//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _CPCDISK_H_
#define _CPCDISK_H_


class kmbInputStream;


namespace CPC {


  /**
  ** Base class for disks. A CDisk contains raw data of a CPC disk and can be used by the emulator to read data from (no writes yet).
  ** Derived classes load data from disk images in specific formats like DSK.
  */
  class CDisk
  {
  public:

    enum EFormat
    {
      FORMAT_STANDARD_DSK = 0,
      FORMAT_EXTENDED_DSK,
      /*** Add more here ***/

      FORMAT_LAST,
      FORMAT_INVALID = 0x7FFFFFFF
    };


    virtual                ~CDisk                     ()  { FreeVars(); }

    /** Returns the format of this disk image. */
    virtual EFormat         GetFormat                 () const = 0;

    /** Returns how many sides the disk has. */
    virtual unsigned        GetSideCount              () const = 0;
    /** Returns how many tracks the disk has. */
    virtual unsigned        GetTrackCount             () const = 0;


  protected:

    // Protected constructor
                            CDisk                     ();


  private:

    void                    ResetVars                 ();
    void                    FreeVars                  ();


  };


} //namespace CPC

#endif // _CPCDISK_H_
