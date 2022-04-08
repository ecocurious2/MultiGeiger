#ifndef transl_h
#define transl_h
/* TR, 07.04.2022
 add new translation strings to ALL files translations_xx.h !!!
 do NOT add translation_xx.h to any of your routines, but transl.h
 Steps to add a new language :
  1. create a new file translation_xx.h where xx is the language code
  2. add this file name to transl.h
  3. add a new chapter into platformio.ini (like [env:geiger_en] (copy&paste), and only adopt the buildflag '-DTRANSL_XX' to the new language
     this defines TRANSL_XX which can be queried in transl.h during runtime in its if-elif and includes the corresponding translation_xx.h.
*/

#if defined(TRANSL_DE)
#include "translations_de.h"
#elif defined(TRANSL_EN)
#include "translations_en.h"
#elif defined(TRANSL_IT)
#include "translations_it.h"
#else
#warning No language defined
#include "translations_en.h"
#endif

#endif
