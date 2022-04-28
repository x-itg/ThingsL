# TINGS

 [整个工程eventListList](https://github.com/zdiputs/eventListList)
# TINGS  事情步骤列表[ThingSetpList]

## 这是一个**tings**的例子，主要用于单片机裸机编程中的多任务(事情)流程处理，它使用简单静态列表管理**事情和步骤**。
 
## 特点
* 轻量，全文一个文件200行代码 ；
* 静态列表直观容易调试；
* 支持外部直接调用调度器函数改变步骤流程；
* 已在项目上使用验证[放心使用]；


## 名词解释
* 大事：事情列表的整体，在这里一个程序只能有一件大事；
* 事情：事情列表里的一行记录，可以理解为任务，一系列步骤组成的一行事情记录；
* 事情函数:这里**事情函数就是调度器函数**，外部调用这个事情函数时使用一些负数形参能改变步骤流转的顺序或使其驻留在一个步骤当中；
* 步骤：一系列步骤列表(事情)里的一条记录；
* 环节：步骤的两个细分称为环节，每个步骤预设两个环节，一个Pre环节[进入该步骤后调用一次Pre环节函数]，一个Poll环节[进入改步骤后按该步骤轮询时间间隔调用]；
* 列表：这里主要分成2种列表，一个事情列表，一个步骤列表，我们对列表里的内容进行增加或修改就可以实现某些流程上的功能而不必去修改底层调度的代码，这样做到将业务解耦代码的复用。
  

## 打开方式
-------------
1. 单片机型号STM32F401CCU6
2. [STM32CubeIDE Version: 1.9.0：./tPen/.project](./tPen/.project)   工程文件:./tPen/.project
3. [iar 9.10.2 工程文件：./tPen/EWARM/Project.eww](./tPen/EWARM/Project.eww) 
4. [电路原理图：./Pen电路.pdf](./Pen电路.pdf) 
5. [主要代码：./tPen/Core/Src/TINGSOS.c](./tPen/Core/Src/TINGSOS.c)


## 代码
-------------
~~~
/********************************************************************************
事情List:SUPPER_FRAME
  步骤List:LISTLIST_FRAME
    每个步骤里面划2个环节:StepSegFlag
这里事情的序号是i  事情里面步骤的呼号是j
               j=0    j=1    j=2     j=3    j=4
  i=0事情A     步骤1  步骤2  步骤3   步骤4   步骤4
  i=1事情B     步骤1  步骤2  步骤3   步骤4
  i=2事情C     步骤1  步骤2  步骤3
*********************************************************************************
auth：
zdiputs http://iamrobot.top
diputs  https://stgui.com
********************************************************************************/
#include "main.h"
#include "string.h"
#define NotFind 0xffff
#define SupMainPollCode (-555)   //主调用[不会操作到SupJumpOn、SupState、SupTimeCnt]
#define SupStopSwitchCode (-444) //暂停switch 跳转关闭  将不会运行switch
#define SupPauseCaseCode (-333)  //暂停case 跳转关闭  停留在case中  停止步骤的跳转 一直在步骤循环调用
#define SupStaGoon (-222)      //不指定继续（状态值为负数），或指定继续（状态值大于等于0）   跳转放行
#define SupPollNum (sppuerList[i].SupJumpOn) //驻留次数 如果为附属就不赋值
//循环次数自减 
#define SupPollNumSelfSub 	if(SupPollNum >0&&SupPollNum!=SupPauseCaseCode){SupPollNum--;}  
#define SupJumpIsEn 		(SupPollNum <= 0 && SupPollNum != SupPauseCaseCode) 					//减到0或负值才能单次跳转反向，不然就是循环次数
#define SupMState 			(sppuerList[i].SupState)                              					//当前事情的状态
#define SupMTimeCnt 		(sppuerList[i].SupTimeCnt)                          					//当前时间的毫秒计数
#define SupMTimeMax  		(sppuerList[i].plistlist[j].internalCntMax)        						//定时回调的超时数
#define SupStepSegFlag    (sppuerList[i].plistlist[j].StepSegFlag)     								//步骤中的两个环节
#define SupStepPreFunc    (*sppuerList[i].plistlist[j].funStepPre)(sppuerList[i].thingsrundata)		//预处理环节
#define SupStepPollFunc   ((*sppuerList[i].plistlist[j].funStepJue)(sppuerList[i].thingsrundata)>0)	//定时调用环节
#define SupStepNextIndex  (sppuerList[i].plistlist[j].nextIndex)									//下一个步骤
#define supperNum    2    //事情数
#define ListListANum 3 //事情里面的步骤数
#define ListListBNum 3 //事情里面的步骤数

//------------------------------步骤List 事情里面的步骤-------------------------------
typedef enum STEP
{
  STEP_PRE = 0, //环节1第一次进这个步骤的处理运行一次
  STEP_POLL,    //环节2间隔调用 环节3
} STEPS;
typedef struct listsTaskFrame
{
  const char *title;           //步骤的标题 在外部调用的时候可以查找匹配的字符串找出步骤列表中的序号
  STEPS StepSegFlag;           //一个步骤分成3个环节  、
  unsigned int internalCntMax; //定时调用的超时时间单位毫秒
  unsigned short nextIndex;    //下一状态序号 默认的下一状态JumpIndexFun会具体判断
  signed short (*funStepPre)(void *);//每个步骤的预处理函数
  signed short (*funStepJue)(void *);//每个步骤的退出选择函数 同时是定时回调  返回值大于0才可能发生状态跳转
  
} LISTLIST_FRAME;
//-----------------------事情List-------------------------------------------------------------------
typedef struct SupperFrame
{
  char *text;              //一般可以设置成2个中文1个英文0作为结束符号  这样作为事情的名称  外部查找发送指令时能用到
  signed short SupJumpOn;  //驻留计数
  signed short SupState;   //一件事情里面的第几个小case 这件事情的进程状态、0状态定义为什么都不干的状态
  unsigned int SupTimeCnt; //毫秒计数
  //第一个参数传入Poll序号，第二个参数跳转状态 负数则不赋值进结构体 ，第三个参数驻留 如果过为SupPauseCaseCode则不赋值进结构体
  //列表中的这个位置函数 即可以放主循环轮询调用 也可以外部调用 控制内部跳转
  signed short (*SupPoll)(unsigned char, signed short, signed int);
  LISTLIST_FRAME *plistlist;//步骤指针
  void * thingsrundata;//事情的运行参数的指针
} SUPPER_FRAME;
signed short supperTaskA(unsigned char i, signed short Config, signed int Stay);
signed short supperTaskB(unsigned char i, signed short Config, signed int Stay);
SUPPER_FRAME sppuerList[supperNum];           //事情列表声明
LISTLIST_FRAME sppuerListListA[ListListANum]; //事情A中的步骤列表声明
LISTLIST_FRAME sppuerListListB[ListListBNum]; //事情B中的步骤列表声明


//-----------------事情A的步骤函数-------------------------------
#if 1
signed short funcStepA1Pre(void * supperSteprun){return 1;}
signed short funcStepA2Pre(void * supperSteprun){return 1;}
signed short funcStepA3Pre(void * supperSteprun){return 1;}
signed short funcStepA1Jue(void * supperSteprun){return 1;}
signed short funcStepA2Jue(void * supperSteprun){return 1;}
signed short funcStepA3Jue(void * supperSteprun){return 1;}
#endif
//-----------------事情B的步骤环节函数-------------------------------
#if 1
signed short funcStepB1Pre(void * supperSteprun){return 1;}
signed short funcStepB2Pre(void * supperSteprun){return 1;}
signed short funcStepB3Pre(void * supperSteprun){return 1;}
signed short funcStepB1Jue(void * supperSteprun){return 1;}
signed short funcStepB2Jue(void * supperSteprun){return 1;}
signed short funcStepB3Jue(void * supperSteprun){return 1;}
#endif

typedef struct aTings
{
  unsigned char para1;
  unsigned char para2;
} TINGS_A;

typedef struct bTings
{
  unsigned char para1;
  unsigned char para2;
} TINGS_B;

TINGS_A tingsA_runData;//事情A的运行参数
TINGS_B tingsB_runData;//事情B的运行参数
SUPPER_FRAME sppuerList[supperNum] =
{
  {"事情A",1, 1, 0, supperTaskA,&sppuerListListA[0],&tingsA_runData},
  {"事情B",1, 1, 0, supperTaskB,&sppuerListListB[0],&tingsB_runData}
};
// A事情中 的3行记录分别是三个步骤
LISTLIST_FRAME sppuerListListA[ListListANum] = {
  {"A步骤1", STEP_PRE, 100, 1,funcStepA1Pre,funcStepA1Jue}, // A1步骤记录体
  {"A步骤2", STEP_PRE, 100, 2,funcStepA2Pre,funcStepA2Jue}, // A2步骤记录体
  {"A步骤3", STEP_PRE, 100, 0,funcStepA3Pre,funcStepA3Jue}  // A2步骤记录体
};

// A事情中 的3行记录分别是三个步骤
LISTLIST_FRAME sppuerListListB[ListListBNum] = {
  {"B步骤1", STEP_PRE, 100, 1,funcStepB1Pre,funcStepB1Jue}, // B1步骤记录体
  {"B步骤2", STEP_PRE, 100, 2,funcStepB2Pre,funcStepB2Jue}, // B2步骤记录体
  {"B步骤3", STEP_PRE, 100, 0,funcStepB3Pre,funcStepB3Jue}  // B3步骤记录体

};

signed short SupperGeneralSch(unsigned char i, signed short Config, signed int Stay);//通用调度器

//事情任务函数:只是为了方便看把调度器函数包了一个函数名而已
//也可以直接在事情列表中只放一个调度器函数
signed short supperTaskA(unsigned char i, signed short Config, signed int Stay){SupperGeneralSch(i,Config,Stay);return Config;}
signed short supperTaskB(unsigned char i, signed short Config, signed int Stay){SupperGeneralSch(i,Config,Stay);return Config;}
SUPPER_FRAME sppuerList[supperNum];

//每个事件时间计值的++
//放到毫秒中断计数中
void SupTimeCntFun(void)
{
  unsigned char i = 0;
  for (i = 0; i < supperNum; i++)
  {
    sppuerList[i].SupTimeCnt++;
  }
}
//流程控制循环体
//放到主循环当中
void SuperRunSequence(void)
{
  unsigned char i = 0;
  for (i = 0; i < supperNum; i++)
  {
    // i传入在List序号，
    //第二个参数传入-4 State不受影响  不会操作到SupJumpOn跳转开关
    (*(sppuerList[i].SupPoll))(i, SupMainPollCode, SupMainPollCode);
  }
}


//通用调度器
signed short SupperGeneralSch(unsigned char i, signed short Config, signed int Stay)
{
  //类似二维数组  步骤列表的序号用j来表示吧、事情列表的序号都用i表示哦
  //i 和 j这两个变量名称跟宏定义很多东西相关不要改掉
  signed short j; 

  if (Stay != SupMainPollCode)//驻留
  {
    SupPollNum = Stay;
  }
  if (Config >= 0)//指定状态的条件
  {
    SupMState = Config;//指定状态
  }
  if (SupPollNum == SupStopSwitchCode)//当前事情序号i停止运转
  {
    return SupStopSwitchCode; //暂停switch 不需要进行进switch case了
  }
  j = SupMState;//获取步骤号

  if(SupStepSegFlag==STEP_PRE)//预处理环节
  {
    SupStepPreFunc;
    SupStepSegFlag = STEP_POLL;
  }

  if(SupStepSegFlag==STEP_POLL)//定时回调环节
  {
    if(SupMTimeCnt>SupMTimeMax)
    {
      SupMTimeCnt=0;
      SupPollNumSelfSub;
      if(SupStepPollFunc&&SupJumpIsEn)//执行回调函数 跳转到下一个步骤 同事判断跳转放行的标志
      {
          SupMState=SupStepNextIndex;//更到下一个步骤 事情的状态值改变
      }
    }
  }
  
  return Config;
}

//外部调用
//这个函数是拿来给别的外部用的
#if 0//用法
SendThingsExtCode("事情A",1,SupPauseCaseCode);//停留在事情A的步骤1
SendThingsExtCode("事情A",1,2);               //停留在事情A的步骤1并驻留2次后继续步骤流转
SendThingsExtCode("事情A",1,2);               //停留在事情A的步骤1并驻留2次后继续步骤流转
SendThingsExtCode("事情A",SupStaGoon,SupStaGoon);//事情A 不指定事情的状态（步骤）继续步骤流转
SendThingsExtCode("事情A",1,SupStaGoon);//事情A 指定事情的状态（步骤）继续步骤流转

#endif
unsigned char SendThingsExtCode( char * pChar,signed short State,signed int Stay)
{
  unsigned short i;
  unsigned char findedflag=0;
  unsigned char str1[6];
  unsigned char str2[6];
  memcpy(str1,pChar,5);str1[5]=0;
  
  for(i=0;i<supperNum;i++)
  {
    memcpy(str2,sppuerList[i].text,5);str2[5]=0;
    if(strcmp((char const *)str1,(char const *)str2)==0)
    {
       (*sppuerList[i].SupPoll)( i,State,Stay);//匹配就调用一次
       findedflag=1;
    }
   
  }
  return findedflag;//没有找到就返回零
}

 
 

~~~
