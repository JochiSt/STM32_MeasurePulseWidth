/*
 * manchester_DMA.c
 * output the
 */

#include "manchester_DMA.h"
#include "tim.h"

/**
 * @addtogroup ManValues Defintion of the Manchester Zero and One
 * @{
 */
const pwm_t MAN_ONE[2] = {0,PWM_MAX_CNT};
const pwm_t MAN_ZERO[2] = {PWM_MAX_CNT, 0};
/** @} */

/**
 * @addtogroup ManHeader definition of the header for the Manchester
 * @{
 */
/// Number of required sync '0's before any bit can start
/// definitions for the Manchester HEADER
#define MAN_HEADER_LEN  8
const pwm_t MAN_HEADER[MAN_HEADER_LEN] = {0, PWM_MAX_CNT, 0, PWM_MAX_CNT, 0, PWM_MAX_CNT, 0, PWM_MAX_CNT};
/** @} */

/// maximum 16 characters per transmission string
#define MAX_STRING_LENGTH 16
/// storage of the data to be send via Manchester
pwm_t MAN_DATA[MAX_STRING_LENGTH * 2*8 + 2*MAN_HEADER_LEN];

void sendManchester(char* str, uint8_t len){

  uint8_t manidx = 0;

  // first apply header
  for(uint8_t manbitidx=0; manbitidx<MAN_HEADER_LEN; manbitidx++){
    MAN_DATA[manidx] = MAN_HEADER[manbitidx];
    manidx++;
  }

  // translate bytes into bits
  for(uint8_t charidx=0; charidx<len; charidx++){
    for(uint8_t bitidx=0; bitidx<8; bitidx++){
      uint8_t man_TXbit = ( str[charidx] & (1<< bitidx) ) >> bitidx;

      // if the bit is set
      if(man_TXbit){
        for(uint8_t manbitidx=0; manbitidx<2; manbitidx++){
          MAN_DATA[manidx] = MAN_ONE[manbitidx];
          manidx++;
        }
      }else{
        for(uint8_t manbitidx=0; manbitidx<2; manbitidx++){
          MAN_DATA[manidx] = MAN_ZERO[manbitidx];
          manidx++;
        }
      }
    }
  }

  // apply a trailer
  for(uint8_t manbitidx=0; manbitidx<MAN_HEADER_LEN; manbitidx++){
    MAN_DATA[manidx] = MAN_HEADER[manbitidx];
    manidx++;
  }

  // finally scale all values to the right value
  HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_3, (uint32_t *)MAN_DATA, manidx);
}
