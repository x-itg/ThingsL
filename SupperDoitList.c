#define PollNum 8   //要做的事情数量
#define NotFind  0xffff
#define SupMainPollCode         (-555)//主循环轮训的主调用[不会操作到SupJumpOn、SupState、SupTimeCnt]
#define SupStopSwitchCode       (-444)//暂停switch 跳转关闭  将不会运行switch
#define SupPauseCaseCode        (-333)//暂停case 跳转关闭  停留在case中 
#define SupNoStaGoon            (-222)//不指定继续  跳转放行 
#define SupYsStaGoon            (-111)//指定继续  跳转放行 

#define SupPollNum          (spuerList[i].SupJumpOn)//驻留次数  
#define SupPollNumSelfSub   if(SupPollNum>0&&SupPollNum!=SupPauseCaseCode){SupPollNum--;}//驻留次数自减  不暂停case的情况下才能自减
#define SupJumpIsEn        (SupPollNum<=0&&SupPollNum!=SupPauseCaseCode)//减到0或负值才能单次跳转放行，不然就是循环次数
#define SupMState         (spuerList[i].SupState)  //当前事情的状态 如果是负数 不会赋值进结构体
#define SupMTimeCnt       (spuerList[i].SupTimeCnt)//当前时间的毫秒计数 

typedef struct SuperFrame{
  signed int     SupJumpOn;   //判断一次减一 减到0才能够跳转 不然就是调用的次数
  signed short   SupState;    //一件事情里面的第几个小case 这件事情的进程状态、0状态定义为什么都不干的状态
  unsigned int   SupTimeCnt;  //毫秒计数 SupState改变瞬间清零
  const char * text;                  //这件事的概述 id 用于外部调用时的序号的查找匹配 一般两个中文字
  signed short   (* SupPoll)(unsigned char,signed short,signed int);//第一个参数传入Poll序号，
}SUPER_FRAME;

#define MAXTHINGNUM 8

SUPER_FRAME spuerList[MAXTHINGNUM];
//事件记录列表  
//一行为一条事件记录 
SUPER_FRAME spuerList[MAXTHINGNUM] = 
{
  {-1,1,0,"ABCC",SupCollectPoll},     
  {-1,1,0,"CDAD",SupCollectPoll},
  {-1,1,0,"SSHA",SupCollectPoll},
  {-1,1,0,"1234",SupCollectPoll},
  {-1,1,0,"3323",SupCollectPoll},
  {-1,1,0,"4342",SupReadTemp},
  {-1,1,0,"3242",SupReadLcdPageAndTime},
  {-1,1,0,"2323",SupxyMotorRun}
 
};
  

void SendThingsExtCode(char * pChar,signed short State,signed int Stay);
void SuperRunSequence(void);
void SupTimeCntFun(void);
signed short SupCollectPoll(unsigned char i,signed short ConfigState,signed int StayPollCnt);
signed short SupReadTemp(unsigned char i,signed short ConfigState,signed int StayPollCnt);
void SupPreSwitchCase(unsigned char i,signed short ConfigState,signed int Stay);
signed short SupReadLcdPageAndTime(unsigned char i,signed short ConfigState,signed int StayPollCnt)
signed short SupxyMotorRun(unsigned char i,signed short ConfigState,signed int StayPollCnt);
unsigned short findTingsIndex(const char * pChar);

void SupPreSwitchCase(unsigned char i,signed short ConfigState,signed int Stay);
{
  if(Stay!=SupMainPollCode)
  {
    SupPollNum=Stay;
  }
  if(ConfigState>0)
  {
    SupMState=ConfigState;
  }
}

//i传入在List序号
//这个函数既是在列表里定义而被循环调用，
//也可以放到其他地方控制干预poll的进程-2暂停 -1不指定继续 >=0指定继续
signed short SupCollectPoll(unsigned char i,signed short ConfigState,signed int StayPollCnt)
{
  SupPreSwitchCase(i,ConfigState,StayPollCnt);
  if(SupPollNum==SupStopSwitchCode)//暂停 跳转关闭  将不会运行switch
  {
    return SupStopSwitchCode;//-2则为暂停 不需要进行进switch case了
  }
   
  switch(SupMState)//spuerList[i].SupState 这里根据事情的填充不一样的条件
  {
    case 0://空闲状态啥都不必做
      break;
    case 1:
      if(SupJumpIsEn)SupMState=2;
      break;
    case 2:
      if(SupJumpIsEn)SupMState=1;
      break;
    
  }
  
  return SupMState;
}

//读取温度的事件
signed short SupReadTemp(unsigned char i,signed short ConfigState,signed int StayPollCnt)
{
  SupPreSwitchCase(i,ConfigState,StayPollCnt);
  if(SupPollNum==SupStopSwitchCode)//暂停 跳转关闭  将不会运行switch
  {
    return SupStopSwitchCode;//-2则为暂停 不需要进行进switch case了
  }
  switch(SupMState)//这里根据事情的填充不一样的条件
  {
    case 0://空闲状态啥都不必做
      break;
    case 1://只要轮训一个的话就停留在这个case
      if(SupMTimeCnt>100)
      {
        SupMTimeCnt=0;
        SupPollNumSelfSub;
        ADCPause;
        runParaData.dsT=DS18B20_GetTemperature();//温度
        ADCContinue;
      }
      break;
    
  }
  return ConfigState;
      
    
}

//页面读取发送给屏幕   
//数据发送给屏幕
//时间读取发送给屏幕
signed short SupReadLcdPageAndTime(unsigned char i,signed short ConfigState,signed int StayPollCnt)
{
  SupPreSwitchCase(i,ConfigState,StayPollCnt);
  if(SupPollNum==SupStopSwitchCode)//暂停 跳转关闭  将不会运行switch
  {
    return SupStopSwitchCode;//-2则为暂停 不需要进行进switch case了
  }
  switch(SupMState)//这里根据事情的填充不一样的条件
  {
    case 0://空闲状态啥都不必做
      break;
    case 1://读取页面
      if(SupMTimeCnt>50)
      {
        SupMTimeCnt=0;
        SupPollNumSelfSub;
        ReadCurrentPageFunc();//读取当前页面 EE B1 01 FF FC FF FF
        if(SupJumpIsEn){
          SupMState=2;
        }
      }
      break;
    case 2://发送板子数据
      if(SupMTimeCnt>50)
      {
        SupMTimeCnt=0;
        SupPollNumSelfSub;
        lcdDataSend(runParaData.recHid);//数据发送
         if(SupJumpIsEn){
          SupMState=3;
        }
      }
      break;
    
    case 3://读取时间
      if(SupMTimeCnt>100)
      {
        SupMTimeCnt=0;
        SupPollNumSelfSub;
        GetTimeSend();//读取时间EE B1 01 FF FC FF FF
        if(SupJumpIsEn){
          SupMState=1;
        }
      }
      break;
    
  }
  
  return SupMState;
}


signed short SupxyMotorRun(unsigned char i,signed short ConfigState,signed int StayPollCnt)
{
  SupPreSwitchCase(i,ConfigState,StayPollCnt);
  if(SupPollNum==SupStopSwitchCode)//暂停 跳转关闭  将不会运行switch
  {
    return SupStopSwitchCode;//-2则为暂停 不需要进行进switch case了
  }
  //x轴y轴找瓶一系列动作
  switch (SupMState) {
  case 0://空闲状态啥都不必做
    break;
  case 1: //初始化
    if (findbottleStart == 1) {
      findbottleStart = 0; //接收到开始找瓶子指令
      SupMState = 2;
    }
    break;
  case 2: 
    if (neachXzero)
    {
      mdrL1;
      MotoN[0]=100;
    }
    if (neachYzero)
    {
      mdrL4;
      MotoN[3]=100;
    }
    if (reachXzero&&reachYzero)
    {
      SupMState = 3;
    }
    break;
  case 3: //找到原点提取瓶号
    FindGotoValue(findbottleN, &MotoN[0], &MotoN[3]); //x轴
    SupMState = 4;
    break;
  case 4: //正移动
    mdrH1;
    mdrH4;
    if(MotoN[0]==0&&MotoN[3]==0)
    {
      SupMState = 5;
    }
    break;
  case 5: //到达目标瓶位置
    break;
    
  }
  
  return SupMState;
}


unsigned char str1[5];
unsigned char str2[5];
const char *pCode="页面";
unsigned short findTingsIndex(const char * pChar)
{
  unsigned short i;
  memcpy(str1,pChar,4);
  str1[4]=0;
  str2[4]=0;
  for(i=0;i<MAXTHINGNUM;i++)
  {
    memcpy(str2,spuerList[i].text,4);
    if(strcmp(str1,str2)==0)
    {
       return i;
    }
   
  }
  return NotFind;
}


//外部调用 
void SendThingsExtCode( char * pChar,signed short State,signed int Stay)
{
  unsigned short i;
  i=findTingsIndex(pChar);
  (*spuerList[i].SupPoll)( i,State,Stay);
}

//每个事件时间计值的++
//放到毫秒中断计数中
void SupTimeCntFun(void)
{
  unsigned char i=0;
  for(i=0;i<PollNum;i++)
  {
    spuerList[i].SupTimeCnt++; 
  }
}

//流程控制循环体
//放到主循环当中
void SuperRunSequence(void)
{
  unsigned char i=0;
  for(i=0;i<PollNum;i++)
  {
    //i传入在List序号，
    //第二个参数传入-4 State不受影响  不会操作到SupJumpOn跳转开关
    (*spuerList[i].SupPoll)( i,
                            SupMainPollCode,//ConfigState  负数不会被赋值到结构体
                            SupMainPollCode);//Stay   正数和SupPauseCaseCode会被赋值到结构体 其他负数不会被赋值到结构体
  }
  
}