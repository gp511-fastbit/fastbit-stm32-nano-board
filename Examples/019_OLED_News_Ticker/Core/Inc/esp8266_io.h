/*
 * esp8266_io.h
 *
 *  Created on: Dec 27, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#ifndef INC_ESP8266_IO_H_
#define INC_ESP8266_IO_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#define RING_BUFFER_SIZE        (1024 * 12)

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t  data[RING_BUFFER_SIZE];
  uint16_t tail;
  uint16_t head;
} ring_buffer_t;
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DEFAULT_TIME_OUT                 3000 /* in ms */

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int8_t esp8266_io_init(void);
void esp8266_io_deinit(void);
void io_buff_reset(void);
int8_t uart_dma_restart(void);


int8_t esp8266_io_send(uint8_t* Buffer, uint32_t Length);
int32_t esp8266_io_recv(uint8_t* Buffer, uint32_t Length);


#endif /* INC_ESP8266_IO_H_ */
