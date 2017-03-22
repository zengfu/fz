#include "fzshell.h"
#include "string.h"







extern UART_HandleTypeDef huart1;
extern osMutexId ShellTxMutHandle;
extern osSemaphoreId ShellReadSemHandle;
extern osMessageQId UartRxHandle;






//global
fzshellTypeDef fzshell;

/**
  * @brief  This function init shell int dma.
  * @param  None
  * @retval 0:success
  */
fz_status fz_shell_init()
{
  memset(&fzshell,0,sizeof(fzshell));
  fzshell.tx_mut=ShellTxMutHandle;
  fzshell.rx_que=UartRxHandle;
  fzshell.uart=&huart1;
  
  //enable idle line int
  __HAL_UART_ENABLE_IT(fzshell.uart,UART_IT_RXNE);
  return 0;
}

void fz_rx_isr()
{
    
    {
      uint32_t data=READ_REG(fzshell.uart->Instance->DR);
      osMessagePut(fzshell.rx_que,data,0);//no block in the inter
    }
}

int fputc(int ch, FILE *f)
{
  uint8_t a;
  a=(uint8_t)ch;
  HAL_UART_Transmit(fzshell.uart, &a, 1, 100);
  return ch;
}








