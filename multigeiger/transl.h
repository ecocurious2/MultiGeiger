#ifndef transl_h
#define transl_h


#if defined(TRANSL_DE)
#include "translations_de.h"
#elif defined(TRANSL_EN)
#include "translations_en.h"
#else
#warning No language defined
#include "translations_en.h"
#endif

#endif
