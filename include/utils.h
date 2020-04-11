//! \file utils.h
//! \brief Utility functions, structs for Tiva c.
//! \author Nguyen Trong Phuong (aka trongphuongpro)
//! \date 2020 April 8

#ifndef __UTILS__
#define __UTILS__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//! \brief Data type that contains GPIO port and pin.
//!
typedef struct PortPin {
    uint32_t base; //!< Memory base of GPIO port
    uint8_t pin; //!< GPIO pin
} PortPin_t;


//! \brief Initialize SysTick.
//! \return nothing.
//!
void delay_init();


//! \brief Delay in millisecond.
//! \param ms millisecond.
//! \return nothing.
//!
void delay_ms(uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif

/********************* End of File *******************************************/