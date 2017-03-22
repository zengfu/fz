#include "fzshelllib.h"
#include "string.h"

extern fzshellTypeDef fzshell;
static void line_remove(uint8_t* dest,uint8_t* src,uint16_t l);
static void line_add(uint8_t* dest,uint8_t data,uint16_t l);
void fzshell_thread()
{
  fzshellTypeDef* shell;
  fz_shell_init();
  osEvent evt;
  shell=&fzshell;
  printf("%s>>","fz_shell");
  while(1)
  {
    evt=osMessageGet(shell->rx_que,osWaitForever);
    uint8_t ch;
    ch=(uint8_t)evt.value.v;
  
     /*
             * handle control key
             * up key  : 0x1b 0x5b 0x41
             * down key: 0x1b 0x5b 0x42
             * right key:0x1b 0x5b 0x43
             * left key: 0x1b 0x5b 0x44
             */
    if(ch==0x1b)
    {
      shell->state=WAIT_SPEC_KEY;
      continue;
    }
    else if(shell->state==WAIT_SPEC_KEY)
    {
      if(ch==0x5b)
      {
        shell->state=WAIT_FUNC_KEY;
        continue;
      }
      shell->state=WAIT_NORMAL;
    }
    else if(shell->state==WAIT_FUNC_KEY)
    {
      shell->state = WAIT_NORMAL;
      if(ch==0x41)/*up key*/
      {
        //history cmd
        continue;
      }
      else if(ch==0x42) /*down key*/
      {
        //history cmd
        continue;
      }
      else if(ch==0x43) /*right key*/
      {
        if(shell->line_curpos<shell->line_position)
        {
          printf("%c",shell->line[shell->line_curpos]);/*overwrite the char*/
          shell->line_curpos++;
        }
        continue;
      }
      else if(ch==0x44)
      {
        if(shell->line_curpos)
        {
          printf("\b");
          shell->line_curpos--;
        }
        continue;
      }
    }
    // enter 
    if(ch=='\r'||ch=='\n')
    {
      //exec_cmd
      printf("\r\n");
      printf("%s>>","fz_shell");
      memset(shell->line,0,shell->line_position);
      shell->line_curpos = shell->line_position = 0;
      continue;
    }
    //back
    if(ch==0x7f||ch==0x08)
    {
      int i;
      if (shell->line_curpos == 0)
                    continue;
      //wahtever --
      shell->line_curpos--;
      shell->line_position--;
      if(shell->line_curpos<shell->line_position)
      {
        line_remove(&shell->line[shell->line_curpos],&shell->line[shell->line_curpos+1],shell->line_position-shell->line_curpos);
        shell->line[shell->line_position]=0;
        //re printf space replace the last one and return 
        printf("\b%s \b",&shell->line[shell->line_curpos]);
        /* move the cursor to the origin position */
        for (i = shell->line_curpos; i < shell->line_position; i++)
          printf("\b");
        
      }
      else
      {
        printf("\b \b");
        shell->line[shell->line_position] = 0;
      }
      continue;
    }
    
    //normal mode 
    if(shell->line_curpos < shell->line_position)
    {
      int i;
      line_add(&shell->line[shell->line_curpos],ch,shell->line_position-shell->line_curpos);
      printf("%s",&shell->line[shell->line_curpos]);
      for (i = shell->line_curpos; i < shell->line_position; i++)
          printf("\b");
    }
    else
    {
      shell->line[shell->line_position]=ch;
      printf("%c",ch);
    }
    //
    ch=0;
    shell->line_curpos++;
    shell->line_position++;
    if(shell->line_position>=FZ_CMD_SIZE)
    {
      /* clear command line */
      shell->line_position = 0;
      shell->line_curpos = 0;
    }
    
    
  }
}
static void line_remove(uint8_t* dest,uint8_t* src,uint16_t l)
{
  while(l--)
    *dest++=*src++;
}
/* from the end to start */
uint8_t *tmp;
static void line_add(uint8_t* dest,uint8_t data,uint16_t l)
{
 
  tmp=dest+l;
  dest+=(l-1);
  
  while(l--)
  {
    *tmp--=*dest--;          
  }
  *(dest+1)=data;
  
}

