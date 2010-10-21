//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef _KMBSTREAMTYPES_H_
#define _KMBSTREAMTYPES_H_


/**
** Stream types.
*/
enum kmbEStreamType
{
  STREAMTYPE_FILE = 0,    // The stream type returned by kmbFileInputStream and kmbFileOutputStream objects.
  STREAMTYPE_MEMORY,      // The stream type returned by kmbMemoryInputStream and kmbMemoryOutputStream objects.

  STREAMTYPE_LAST,
  STREAMTYPE_INVALID = 0x7FFFFFFF
};


#endif // _KMBSTREAMTYPES_H_
