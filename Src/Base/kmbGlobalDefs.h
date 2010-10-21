//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

#ifndef _KMBGLOBALDEFS_H_
#define _KMBGLOBALDEFS_H_


// Define global options based on the build type.
#ifdef KM_DEBUG_BUILD
  #include "kmbGlobalOptions_Debug.h"
#else
  #ifdef KM_RELEASE_BUILD
    #include "kmbGlobalOptions_Release.h"
  #else
    #error Build type not specified!
  #endif // KM_RELEASE_BUILD
#endif // KM_DEBUG_BUILD


// New and delete utilities
#define KMBDISPOSE(x)    { delete (x); (x) = NULL; }


//
// Macro para saber el número de elementos que hay en un array.
// Ejemplo:
//   struct Pepe aPepes[] = { {...}, {...}, {...} };
//
//   sizeof(aPepes) da los bytes totales que ocupa el array aPepes
//   ARRAY_COUNT(aPepes) da 3 (el número de elementos Pepe que hay en el array aPepes)
//
#define ARRAY_COUNT(array_var)  (sizeof(array_var) / sizeof(array_var[0]))


#endif // _KMBGLOBALDEFS_H_
