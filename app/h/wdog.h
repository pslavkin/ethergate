#ifndef WDOG
#define WDOG
//-----------------------------------------------
extern 						void Init_Wdog		(void);
extern __attribute__ ((interrupt ("IRQ"))) 	void Wdog_Handler 	(void);
extern 						void Wdog_Clear		(void);
//-----------------------------------------------
#endif
