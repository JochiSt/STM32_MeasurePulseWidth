/*
 * manchester_DMA.h
 *
 *  Created on: Jun 7, 2023
 *      Author: steinmann
 */

#ifndef INC_MANCHESTER_DMA_H_
#define INC_MANCHESTER_DMA_H_

#include <stdint.h>

/// TIM DMA settings
#define MAN_TIM_DMA &htim3, TIM_CHANNEL_3

void sendManchester(char* str, uint8_t len);

#endif /* INC_MANCHESTER_DMA_H_ */
