#ifndef __IO_H__
#define __IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Exported function prototypes.
//
//*****************************************************************************
void io_init     ( void     );
void io_set_led  ( bool bOn );
int io_is_led_on ( void     );

#ifdef __cplusplus
}
#endif

#endif // __IO_H__
