# SuperDoitList

1. SuperRunSequence()放到主循环里轮询
2. SupTimeCntFun()放到定时器中断中定时调用
3. 自定义的事件函数如SupCollectPoll被放到事件列表中，同时也可以在其他地方调用控制事件状态的跳转。
4. 其他在代码中注释

```
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
```
