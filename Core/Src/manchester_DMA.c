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

#define MAN_IDLE_LENGTH 16
const pwm_t MAN_IDLE[MAN_IDLE_LENGTH] = { PWM_MAX_CNT, 0,\
                                          PWM_MAX_CNT, 0,\
                                          PWM_MAX_CNT, 0,\
                                          PWM_MAX_CNT, 0,\
                                          PWM_MAX_CNT, 0,\
                                          PWM_MAX_CNT, 0,\
                                          PWM_MAX_CNT, 0,\
                                          PWM_MAX_CNT, 0\
                                          };
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

enum man_state_t {
    STATE_MAN_IDLE,
    STATE_MAN_SEND,
    STATE_MAN_ZERO,
} man_state;

struct{
    uint8_t datasent  : 1;
    uint8_t dataready : 1;
} man_status;

/// maximum 16 characters per transmission string
#define MAX_STRING_LENGTH 16
/// storage of the data to be send via Manchester
pwm_t MAN_DATA[MAX_STRING_LENGTH * 2*8 + 2*MAN_HEADER_LEN];
uint8_t manidx;

void initManchester(){
  man_status.dataready = 0;
  man_status.datasent = 1;

  man_state = STATE_MAN_IDLE;
}

void sendManchester(char* str, uint8_t len){

  // wait until the data is send out
  while( ! man_status.datasent ){
    HAL_Delay(100);
  };

  // reset the length indication value
  manidx = 0;

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

  // finally indicate, that the data is ready to be send out
  man_status.dataready = 1;
  // there is no data sent out
  man_status.datasent = 0;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  // stop the data transfer
  HAL_TIM_PWM_Stop_DMA(MAN_TIM_DMA);

  // if the previous transfer was a data transfer, indicate, that the data has
  // been send out
  if( man_state == STATE_MAN_SEND){
    man_status.datasent = 1;
  }

  // is there new data ready to be send?
  // otherwise send out the idle pattern
  if( man_status.dataready ){
    man_status.dataready = 0;
    HAL_TIM_PWM_Start_DMA(MAN_TIM_DMA, (uint32_t *)MAN_DATA, manidx);
    man_state = STATE_MAN_SEND;
  }else{
    HAL_TIM_PWM_Start_DMA(MAN_TIM_DMA, (uint32_t *)MAN_IDLE, MAN_IDLE_LENGTH);
    man_state = STATE_MAN_ZERO;
  }

}
