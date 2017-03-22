#ifndef _FZ_SHELL_
#define _FZ_SHELL_



//based on stm32f4
#include "stm32f4xx_hal.h"
#include "fzshelllib.h"
#include "cmsis_os.h"


#define FZ_CMD_SIZE 100 //every line max length


typedef int fz_status;

void fz_rx_isr();
fz_status fz_shell_init();
void fz_shell_write(uint8_t* data,uint16_t len);


typedef enum 
{
  WAIT_NORMAL=0U,
  WAIT_SPEC_KEY,
  WAIT_FUNC_KEY,
}input_stat;




typedef struct
{
  osMutexId tx_mut;  //tx mutex
  osMessageQId rx_que; //read que
  UART_HandleTypeDef* uart;

  input_stat state;
  uint8_t line[FZ_CMD_SIZE];
  uint8_t line_position;
  uint8_t line_curpos;
}fzshellTypeDef;






#endif