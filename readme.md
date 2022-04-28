# ThingsL  

## [Things Step List System][事情步骤列表系统][demo：https://github.com/zdiputs/ThingsL](https://github.com/zdiputs/ThingsL)

### TINGLS为动作流程自动化管理提供一套[简便]、[直观]、[便于调试]的C语言代码接口函数和列表。

## 特点
* 轻量，全文一个文件200行代码 ；
* 静态列表直观容易调试；
* 支持外部直接调用调度器函数改变步骤流程；
* 已在项目上使用验证[放心使用]；


## 名词解释
* 事情：事情列表里的一行记录，可以理解为任务，一系列步骤组成的一行事情记录；
* 事情函数:这里**事情函数就是调度器函数**，外部调用这个事情函数时使用一些负数形参能改变步骤流转的顺序或使其驻留在一个步骤当中；
* 步骤：一系列步骤列表(事情)里的一条记录；
* 环节：步骤的两个细分称为环节，每个步骤预设两个环节，一个Pre环节[进入该步骤后调用一次Pre环节函数]，一个Poll环节[进入改步骤后按该步骤轮询时间间隔调用]；
* 列表：这里主要分成2种列表，一个事情列表，一个步骤列表，我们对列表里的内容进行增加或修改就可以实现某些流程上的功能而不必去修改底层调度的代码，这样做到将业务解耦代码的复用。
 

## 代码
-------------
~~~
#include "main.h"
#include "string.h"
/********************************************************************************
事情List                   ThingsL_PER_FRAME
  步骤List                 LISTLIST_FRAME
    每个步骤里面划3个环节   StepSegFlag
这里事情的序号是i  事情里面步骤的呼号是j
               j=0    j=1    j=2     j=3    j=4
  i=0事情A     步骤1  步骤2  步骤3   步骤4   步骤4
  i=1事情B     步骤1  步骤2  步骤3   步骤4
  i=2事情C     步骤1  步骤2  步骤3
*********************************************************************************
  auth:火星火箭 (z)diputs 
  http:iamrobot.top
  https:stgui.com
********************************************************************************/
#define NotFind 0xffff
#define ThingsL_MainPollCode (-555)   //主调用[不会操作到ThingsL_JumpOn、ThingsL_State、ThingsL_TimeCnt]
#define ThingsL_StopSwitchCode (-444) //暂停switch 跳转关闭  将不会运行switch
#define ThingsL_PauseCaseCode (-333)  //暂停case 跳转关闭  停留在case中  停止步骤的跳转 一直在步骤循环调用
#define ThingsL_StaGoon (-222)      //不指定继续（状态值为负数），或指定继续（状态值大于等于0）   跳转放行
#define ThingsL_PollNum (ThingsL_List[i].ThingsL_JumpOn) 							//驻留次数 如果为附属就不赋值
#define ThingsL_PollNumSelfSub     if(ThingsL_PollNum>0&&ThingsL_PollNum!=ThingsL_PauseCaseCode){ThingsL_PollNum--;}//循环次数自减  不暂停case的情况下才能自减
#define ThingsL_JumpIsEn 			(ThingsL_PollNum <= 0 && ThingsL_PollNum != ThingsL_PauseCaseCode) //减到0或负值才能单次跳转反向，不然就是循环次数
#define ThingsL_MState 				(ThingsL_List[i].ThingsL_State)                              //当前事情的状态
#define ThingsL_MTimeCnt 			(ThingsL_List[i].ThingsL_TimeCnt)                          //当前时间的毫秒计数
#define ThingsL_MTimeMax  			(ThingsL_List[i].plistlist[j].internalCntMax)        //定时回调的超时数
#define ThingsL_StepSegFlag    		(ThingsL_List[i].plistlist[j].StepSegFlag)     		//步骤中的两个环节
#define ThingsL_StepPreFunc    		(*ThingsL_List[i].plistlist[j].funStepPre)(ThingsL_List[i].thingsrundata)//预处理环节
#define ThingsL_StepPollFunc   		((*ThingsL_List[i].plistlist[j].funStepJue)(ThingsL_List[i].thingsrundata)>0)//定时调用环节
#define ThingsL_StepNextIndex  		(ThingsL_List[i].plistlist[j].nextIndex)				//下一个步骤的序号也是下一事情的状态
#define ThingsL_erRunSeqFun			(*ThingsL_List[i].ThingsL_Poll)
#define ThingsL_perNum 2    //事情数
#define ListListANum 3 //事情里面的步骤数
#define ListListBNum 3 //事情里面的步骤数

//------------------------------步骤List 事情里面的步骤-------------------------------
typedef enum STEP
{
  STEP_PRE = 0, //环节1第一次进这个步骤的处理运行一次
  STEP_POLL,    //环节2间隔调用
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
typedef struct ThingsL_perFrame
{
  char *text;              //一般可以设置成2个中文1个英文0作为结束符号  这样作为事情的名称  外部查找发送指令时能用到
  signed short ThingsL_JumpOn;  //驻留计数
  signed short ThingsL_State;   //一件事情里面的第几个小case 这件事情的进程状态、0状态定义为什么都不干的状态
  unsigned int ThingsL_TimeCnt; //毫秒计数
  //第一个参数传入Poll序号，第二个参数跳转状态 负数则不赋值进结构体 ，第三个参数驻留 如果过为ThingsL_PauseCaseCode则不赋值进结构体
  //列表中的这个位置函数 即可以放主循环轮询调用 也可以外部调用 控制内部跳转
  signed short (*ThingsL_Poll)(unsigned char, signed short, signed int);
  LISTLIST_FRAME *plistlist;//步骤指针
  void * thingsrundata;//事情的运行参数的指针
} ThingsL_PER_FRAME;
signed short ThingsL_perTaskA(unsigned char i, signed short Config, signed int Stay);
signed short ThingsL_perTaskB(unsigned char i, signed short Config, signed int Stay);

ThingsL_PER_FRAME ThingsL_List[ThingsL_perNum];           //事情列表声明
LISTLIST_FRAME ThingsL_ListListA[ListListANum]; //事情A中的步骤列表声明
LISTLIST_FRAME ThingsL_ListListB[ListListBNum]; //事情B中的步骤列表声明
signed short ThingsL_perGeneralSch(unsigned char i, signed short Config, signed int Stay);//通用调度器
typedef struct aTings
{
  unsigned char para1;
  unsigned char para2;
} TINGS_A;
TINGS_A tingsA_runData;//事情A的运行参数
//-----------------事情A的步骤预处理函数--------返回值大于0才可能发生状态跳转-----内容里可以放一些步骤的功能------------------
signed short funcStepA1Pre(void * ThingsL_perSteprun){return 1;}
signed short funcStepA2Pre(void * ThingsL_perSteprun){return 1;}
signed short funcStepA3Pre(void * ThingsL_perSteprun){return 1;}
//-----------------事情A的步骤定时调用函数------返回值大于0才可能发生状态跳转------内容里可以放一些步骤的功能-------------------
signed short funcStepA1Jue(void * ThingsL_perSteprun){return 1;}
signed short funcStepA2Jue(void * ThingsL_perSteprun){return 1;}
signed short funcStepA3Jue(void * ThingsL_perSteprun){return 1;}

typedef struct bTings
{
  unsigned char para1;
  unsigned char para2;
} TINGS_B;
TINGS_B tingsB_runData;//事情B的运行参数
//-----------------事情B的步骤预处理函数--------返回值大于0才可能发生状态跳转------内容里可以放一些步骤的功能-----------------
signed short funcStepB1Pre(void * ThingsL_perSteprun){return 1;}
signed short funcStepB2Pre(void * ThingsL_perSteprun){return 1; }
signed short funcStepB3Pre(void * ThingsL_perSteprun){return 1;}
//-----------------事情A的步骤定时调用函数------返回值大于0才可能发生状态跳转-------内容里可以放一些步骤的功能---------
signed short funcStepB1Jue(void * ThingsL_perSteprun){return 1;}
signed short funcStepB2Jue(void * ThingsL_perSteprun){return 1;}
signed short funcStepB3Jue(void * ThingsL_perSteprun){return 1;}
//事情列表：是整件[大事]
ThingsL_PER_FRAME ThingsL_List[ThingsL_perNum] =
{
  {"事情A",1, 1, 0, ThingsL_perGeneralSch,&ThingsL_ListListA[0],&tingsA_runData},//一行记录与一件事情对应
  {"事情B",1, 1, 0, ThingsL_perGeneralSch,&ThingsL_ListListB[0],&tingsB_runData}//一行记录与一件事情对应
};

// A事情中 的3行记录分别是三个步骤
LISTLIST_FRAME ThingsL_ListListA[ListListANum] = {
  {"A步骤1", STEP_PRE, 100, 1,funcStepA1Pre,funcStepA1Jue}, // A1步骤记录体  一行记录与一件步骤对应
  {"A步骤2", STEP_PRE, 100, 2,funcStepA2Pre,funcStepA2Jue}, // A2步骤记录体  一行记录与一件步骤对应
  {"A步骤3", STEP_PRE, 100, 0,funcStepA3Pre,funcStepA3Jue}  // A2步骤记录体  一行记录与一件步骤对应
};

// A事情中 的3行记录分别是三个步骤
LISTLIST_FRAME ThingsL_ListListB[ListListBNum] = {
  {"B步骤1", STEP_PRE, 100, 1,funcStepB1Pre,funcStepB1Jue}, // B1步骤记录体  一行记录与一件步骤对应
  {"B步骤2", STEP_PRE, 100, 2,funcStepB2Pre,funcStepB2Jue}, // B2步骤记录体  一行记录与一件步骤对应
  {"B步骤3", STEP_PRE, 100, 0,funcStepB3Pre,funcStepB3Jue}  // B3步骤记录体  一行记录与一件步骤对应

};


//外部调用
//这个函数是拿来给别的外部用的
#if 0//用法
SendThingsExtCode("事情A",1,ThingsL_PauseCaseCode);//停留在事情A的步骤1
SendThingsExtCode("事情A",1,2);               //停留在事情A的步骤1并驻留2次后继续步骤流转
SendThingsExtCode("事情A",1,2);               //停留在事情A的步骤1并驻留2次后继续步骤流转
SendThingsExtCode("事情A",ThingsL_StaGoon,ThingsL_StaGoon);//事情A 不指定事情的状态（步骤）继续步骤流转
SendThingsExtCode("事情A",1,ThingsL_StaGoon);//事情A 指定事情的状态（步骤）继续步骤流转
#endif
unsigned char SendThingsExtCode( char * pChar,signed short State,signed int Stay)
{
  unsigned short i;
  unsigned char findedflag=0;
  unsigned char str1[6];
  unsigned char str2[6];
  memcpy(str1,pChar,5);str1[5]=0;
  
  for(i=0;i<ThingsL_perNum;i++)
  {
    memcpy(str2,ThingsL_List[i].text,5);str2[5]=0;
    if(strcmp((char const *)str1,(char const *)str2)==0)
    {
       ThingsL_erRunSeqFun( i,State,Stay);//匹配就调用一次
       findedflag=1;
    }
   
  }
  return findedflag;//没有找到就返回零
}

 
/************************************************************************************
通用调度器
可以把这个放到每个自定义的[事情任务函数]中 这样会自动执行对应的步骤列表
一般事情列表中放这个函数，让调度器自动执行对应的步骤列表
*************************************************************************************/
signed short ThingsL_perGeneralSch(unsigned char i, signed short Config, signed int Stay)
{
  //类似二维数组  步骤列表的序号用j来表示吧、事情列表的序号都用i表示哦
  //i 和 j这两个变量名称跟宏定义很多东西相关不要改掉
  signed short j; 

  if (Stay != ThingsL_MainPollCode)//驻留
  {
    ThingsL_PollNum = Stay;
  }
  if (Config >= 0)//指定状态的条件
  {
    ThingsL_MState = Config;//指定状态
  }
  if (ThingsL_PollNum == ThingsL_StopSwitchCode)//当前事情序号i停止运转
  {
    return ThingsL_StopSwitchCode; //暂停switch 不需要进行进switch case了
  }
  j = ThingsL_MState;//获取步骤号

  if(ThingsL_StepSegFlag==STEP_PRE)//预处理环节
  {
    ThingsL_StepPreFunc;
    ThingsL_StepSegFlag = STEP_POLL;
  }

  if(ThingsL_StepSegFlag==STEP_POLL)//定时回调环节
  {
    if(ThingsL_MTimeCnt>ThingsL_MTimeMax)
    {
      ThingsL_MTimeCnt=0;
      ThingsL_PollNumSelfSub;
      if(ThingsL_StepPollFunc&&ThingsL_JumpIsEn)//执行回调函数 跳转到下一个步骤 同事判断跳转放行的标志
      {
          ThingsL_MState=ThingsL_StepNextIndex;//更到下一个步骤 事情的状态值改变
      }
    }
  }
  
  return Config;
}


/*---自定义调度器-----------------------------------------------------------------------------
自定义调度器
如果不用[通用调度器]ThingsL_perGeneralSch(可以把类似这种函数放进去事情回调函数中)，
在这个函数的switch case中直接写流程,则会跳过步骤列表里的内容.
{"事情A",1, 1, 0, NoSchedulerPutitLikethis,&ThingsL_ListListA[0],&tingsA_runData},
--------------------------------------------------------------------------------------------*/
void NoSchedulerPutitLikethis(unsigned char i,signed short ConfigState,signed int Stay)
{
  if(Stay!=ThingsL_MainPollCode)
  {
    ThingsL_PollNum=Stay;
  }
  if(ConfigState>0)
  {
    ThingsL_MState=ConfigState;
  }
  
  switch(ThingsL_MState)
  {
	  case 0:ThingsL_MState=1;break;
	  case 1:ThingsL_MState=2;break;
	  case 2:ThingsL_MState=3;break;
	  case 3:ThingsL_MState=0;break;

  }
}


//每个事件时间计值的++
//放到毫秒中断计数中
void ThingsL_TimeCntFun(void)
{
  unsigned char i = 0;
  for (i = 0; i < ThingsL_perNum; i++)
  {
    ThingsL_MTimeCnt++;
  }
}
//流程控制循环体
//放到主循环当中
void ThingsL_erRunSequence(void)
{
  for (unsigned char i = 0; i < ThingsL_perNum; i++)
  {
    ThingsL_erRunSeqFun(i, ThingsL_MainPollCode, ThingsL_MainPollCode);
  }
}
~~~
