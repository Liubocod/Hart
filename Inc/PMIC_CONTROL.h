/**
 ******************************************************************************
  * File Name          : PMIC_CONTROL.h

*/
#ifndef __PMIC_CONTROL_H
#define __PMIC_CONTROL_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include 	"i2c.h"
   
#define PMIC_LONG_TIMEOUT			1000    /* Long Timeout 1s */
#define	PMIC_ADDRESS				0xC0

void set_PMIC_to_PFM_mode(void);
   
#ifdef __cplusplus
}
#endif
#endif /*__PMIC_CONTROL_H */
