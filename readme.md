#SuperDoitList

```
#include "main.h"
#define PollNum 6   //要做的事情数量

#define SupJumpEn       (spuerList[i].SupJumpOn>0)//每个跳转受外部干预的跳转条件，如果需要受外部干预暂停的case且&&上这个条件
#define SupMState       (spuerList[i].SupState)  //当前事情的状态
#define SupMTimeCnt      (spuerList[i].SupTimeCnt)//当前时间的毫秒计数

#define SupMainPollCode          (-4)//主调用[不会操作到SupJumpOn、SupState、SupTimeCnt]
#define SupStopSwitchCode       (-3)//暂停 跳转关闭  将不会运行switch
#define SupPauseCaseCode        (-2)//暂停 跳转关闭  停留在case中 
#define SupNoStaGoon            (-1)//不指定继续  跳转放行


//这里主要做 事件列表  
//列表进程（poll轮训）互相有序干预的一个框架
//有以下2个动作
//叫暂停     SupJumpOn赋0  目的是：一直停留在那个 小case环节 SupState保存不变 
//叫继续     一种指定状态跳转继续 另外一种不指定继续跳转放行

typedef struct SuperFrame{
  signed char    SupJumpOn;   //大于零 跳转和毫秒计数的必要条件，小于等于零用来暂停用，定时器计数也清零
  signed short   SupState;    //一件事情里面的第几个小case 这件事情的进程状态、0状态定义为什么都不干的状态
  unsigned int   SupTimeCnt;  //毫秒计数 SupState改变瞬间清零
  char * text;            //这件事的概述
  signed short   (* SupPoll)(unsigned char,signed short);//第一个参数传入Poll序号，
}SUPER_FRAME;
SUPER_FRAME spuerList[];//事先声明一下有这个列表  在下面定义

//Switch前的判断
signed short SupPreSwitchCase(unsigned char i,signed short Config)
{
  if(Config==SupMainPollCode)//主调用[不会操作到SupJumpOn、SupState、SupTimeCnt]
  {
    return Config;
  }
  if(Config==SupStopSwitchCode)//暂停 跳转关闭  将不会运行switch
  {
    spuerList[i].SupJumpOn=0;
    return Config;
  }

  if(Config==SupPauseCaseCode)//暂停 跳转关闭  停留在case中 
  {
    spuerList[i].SupJumpOn=0;
    return Config;
  }
  if(Config==SupNoStaGoon)//不指定继续  跳转放行
  {
    spuerList[i].SupJumpOn=1;//所有跳转可以
    return Config;
  }
  if(Config>=0)//指定继续、跳转到0一般为什么都不做的空闲状态、
  {
    spuerList[i].SupState=Config;
    spuerList[i].SupJumpOn=1;
    return Config;
  }
  return Config;
}

//i传入在List序号
//这个函数既是在列表里定义而被循环调用，
//也可以放到其他地方控制干预poll的进程-2暂停 -1不指定继续 >=0指定继续
signed short SupCollectPoll(unsigned char i,signed short ConfigState)
{
  if(SupPreSwitchCase(i,ConfigState)==SupStopSwitchCode)
  {
    return SupStopSwitchCode;//-2则为暂停 不需要进行进switch case了
  }

  switch(SupMState)//这里根据事情的填充不一样的条件
  {
    case 0://空闲状态啥都不必做
      break;
    case 1:
      if(SupJumpEn)SupMState=2;
      break;
    case 2:
      if(SupJumpEn)SupMState=1;
      break;

  }

  return SupMState;
}

//读取温度的事件
signed short SupReadTemp(unsigned char i,signed short ConfigState)
{
  if(SupPreSwitchCase(i,ConfigState)==SupStopSwitchCode)
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
        ADCPause;
        runParaData.dsT=DS18B20_GetTemperature();//温度
        ADCContinue;
      }
      break;

  }
  return ConfigState;


}
//事件记录列表  
//一行为一条事件记录 
SUPER_FRAME spuerList[] = 
{
  {1,1,0,"采样",SupCollectPoll},     
  {1,1,0,"混样",SupCollectPoll},
  {1,1,0,"留样",SupCollectPoll},
  {1,1,0,"加药",SupCollectPoll},
  {1,1,0,"供样",SupCollectPoll},
  {1,1,0,"温度",SupReadTemp},

};
//每个事件时间计值的++
//放到毫秒中断计数中
void SupTimeCntFun(void)
{
  unsigned char i=0;
  for(i=0;i<PollNum;i++)
  {
    if(SupJumpEn)
    {
      spuerList[i].SupTimeCnt++;
    }else
    {
      spuerList[i].SupTimeCnt=0;
    }
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
    (*spuerList[i].SupPoll)(i,SupMainPollCode);
  }

}
```