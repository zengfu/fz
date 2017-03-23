#include "fzshelllib.h"
#include "string.h"
#include "stdlib.h"

#define ARGV_MAX_SIZE 10



extern fzshellTypeDef fzshell;
static void line_remove(uint8_t* dest,uint8_t* src,uint16_t l);
static void line_add(uint8_t* dest,uint8_t data,uint16_t l);
static uint8_t splite(uint8_t *cmd,uint16_t length,uint8_t* argv[ARGV_MAX_SIZE]);

int caculate(uint8_t* data);

void fz_exec(uint8_t* cmd,uint16_t length)
{
  //reduce the the first spapce
  uint8_t* argv[ARGV_MAX_SIZE];
  uint8_t argc;
  argc=splite(cmd,length,argv);
  //test
  if(argc==2)
  {
    if(!strcmp("math",argv[0]))
    {
      int result;
      result=caculate(argv[1]);
      printf("result:%d\r\n",result);
    }
  }
  
}

uint8_t lable[10];
int num[10];

int _caculate(int* num,uint8_t len,uint8_t* label)
{
  int16_t result;
  for(int i=len-2;i>=0;i--)
  {
    int tmp=0;
    if(label[i]=='*'||label[i]=='/')
    {
      if(label[i]=='*')
      {
        tmp=num[i]*num[i+1];
      }
      else if(lable[i]=='/')
      {
        tmp=num[i]/num[i+1];
      }
      label[i]='+';
      num[i]=tmp;
      num[i+1]=0;  
    }
  }
  result=num[0];
  for(int j=0;j<len-1;j++)
  {
    if(lable[j]=='+')
      result+=num[j+1];
    else if(lable[j]=='-')
      result-=num[j+1];
  }
  
  return result;
  
}
int caculate(uint8_t* data)
{
  
  uint16_t len=strlen(data);
  uint16_t cnt=0;
  uint16_t index=0;
  for(int i=0;i<len;i++)
  {
    if(data[i]>0x39||data[i]<0x30)
    {
      lable[cnt]=data[i];
      data[i]='\0';
      num[cnt]=atoi(data+index);
      cnt++;
      index=i+1; 
    }
  }
  num[cnt++]=atoi(data+index);
  //caculate
  return _caculate(num,cnt,lable);
}




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
      printf("\r\n");
      fz_exec(shell->line,shell->line_position);
      //exec_cmd
      
      printf("%s>>","fz_shell");
      memset(shell->line,0,shell->line_position);
      shell->line_curpos = shell->line_position = 0;
      continue;
    }
    //tab
    if(ch=='\t')
    {
      //auto complete
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
static uint8_t splite(uint8_t *cmd,uint16_t length,uint8_t* argv[ARGV_MAX_SIZE])
{
  uint8_t argc=0;
  /*
  * argc >=1
  * ls -s -a
  * argc=3 argv[0]=ls argv[1]=-s argv[2]=-a
  */
  
  uint8_t* index=cmd;
  uint16_t position=0;
  
  while(position<length)
  {
    //replace the space to '\0'
    while((*index == ' ' || *index == '\t') && position < length)
    {
      *index='\0';
      index++;
      position++;
    }
    if(position<length)
    {
      argv[argc]=index;
      argc++;
    }
    while((*index != ' ') && position < length)
    {
      index++;
      position++;
    }
   
  }
  return argc;
}

