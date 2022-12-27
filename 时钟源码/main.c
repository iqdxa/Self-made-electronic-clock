#include "STC15F2K.h"
#include "intrins.h"
#include <math.h>

#define uchar unsigned char 
#define uint unsigned int

sfr ADC_LOW2    =   0xBE;           //ADC低2位结果

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟

#define FOSC 22118400L
#define T1MS (65536-FOSC/1000)      //1T模式
//#define T1MS (65536-FOSC/12/1000) //12T模式		 1
#define BAUD 115200             //串口波特率
#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7

sbit DS4=P3^4;		 //正极
sbit DS3=P3^5;
sbit DS2=P3^6;
sbit DS1=P3^7;

sbit set=P3^0;
sbit jia=P3^1;

sbit zhuan=P3^3;
sbit bell=P1^7;

bit Time_Point_Blink=0;		  			//时间显示中间两点闪烁标志位
bit Flag_Lum_AD=0;								//光敏AD采样允许位

uchar dat1[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xf8,0X80,0X90,0xff,0xc6};			 
uchar dat2[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0X00,0X10,0xff};//有小数点

uchar dat3[]={0x40,0x4F,0x24,0x06,0x0B,0x12,0x10,0x47,0X00,0X02,0xff};//有小数点
uchar dat4[]={0xC0,0xCF,0xA4,0x86,0x8B,0x92,0x90,0xC7,0X80,0X82,0xff};// 
   
uchar table[4]={0};
unsigned int temp[3];
uchar Temp_time1,Temp_time2;
uchar Differ_Time=0;		  			//不同数字所需要延时时间
uint Lum_Val_Buf[10]=0;
uchar Lum_Count=0;							//亮度采样10次，求平均值
uint lum_temp=0;									//10次采样值之和

extern void init_ds1302_io();
extern void init_ds1302();
extern void read_time();
extern void write_time();
extern void read_nao();
extern void write_setting();
extern void read_setting();

extern int fen;
extern int shi;
extern int miao;

extern int nfen;
extern int nshi;

extern uchar nk;
extern uchar gk;
extern bit nx;
extern bit sw;

void Delayms(uint t)
{
 	uchar i;
	while(t--)
	   for(i=0;i<123;i++);
}

void Delay_select(uchar i)
{
	switch(i)
	{
		case 0:Differ_Time=5;break;
		case 1:Differ_Time=2;break;
		case 2:Differ_Time=5;break;
		case 3:Differ_Time=5;break;
		case 4:Differ_Time=4;break;
		case 5:Differ_Time=5;break;
		case 6:Differ_Time=5;break;
		case 7:Differ_Time=3;break;
		case 8:Differ_Time=5;break;
		case 9:Differ_Time=5;break;
		default:break;
	}
}
 
char flag=1;
void wd_display()
{
	P2=0XFF;
	DS1=1;
	DS2=0;
	DS3=0;
	DS4=0;
	P2=dat1[table[0]];
	Delay_select(table[0]);
	Delayms(Differ_Time);

	P2=0XFF;
	DS1=0;
	DS2=1;
	DS3=0;
	DS4=0;
	P2=dat2[table[1]];
	Delay_select(table[1]);
	Delayms(Differ_Time+1);

	P2=0XFF;
	DS1=0;
	DS2=0;
	DS3=1;
	DS4=0;
	P2=dat4[table[2]];
	Delay_select(table[2]);
	Delayms(Differ_Time);

	P2=0XFF;
	DS1=0;
	DS2=0;
	DS3=0;
	DS4=1;
	P2=dat1[table[3]];
	Delayms(4);

	DS1=0;
	DS2=0;
	DS3=0;
	DS4=0;
}
int ld;
void display()
{
	P2=0XFF;
	DS1=1;
	DS2=0;
	DS3=0;
	DS4=0;
	P2=dat1[table[0]];
	Delay_select(table[0]);
	Delayms(Differ_Time);

	if(flag>0)							 //时间显示中间两点亮
	{
		P2=0XFF;
		DS1=0;
		DS2=1;
		DS3=0;
		DS4=0;
		P2=dat2[table[1]];
		Delay_select(table[1]);
		Delayms(Differ_Time+1);

		P2=0XFF;
		DS1=0;
		DS2=0;
		DS3=1;
		DS4=0;
		P2=dat3[table[2]];
		Delay_select(table[2]);
		Delayms(Differ_Time+1);

	}
	else								  //时间不显示中间两点
	{
		P2=0XFF;
		DS1=0;
		DS2=1;
		DS3=0;
		DS4=0;
		P2=dat1[table[1]];
		Delay_select(table[1]);
		Delayms(Differ_Time);

		P2=0XFF;
		DS1=0;
		DS2=0;
		DS3=1;
		DS4=0;
		P2=dat4[table[2]];
		Delay_select(table[2]);
		Delayms(Differ_Time);

	}

	P2=0XFF;
	DS1=0;
	DS2=0;
	DS3=0;
	DS4=1;
	P2=dat1[table[3]];
	Delay_select(table[3]);
	Delayms(Differ_Time);
	
	DS1=0;
	DS2=0;
	DS3=0;
	DS4=0;
}
void set_display()
{
	DS1=1;
	DS2=0;
	DS3=0;
	DS4=0;
	P2=dat1[table[0]];
	Delayms(5);

	DS1=0;
	DS2=1;
	DS3=0;
	DS4=0;
	P2=dat2[table[1]];

	Delayms(5);
	DS1=0;
	DS2=0;
	DS3=1;
	DS4=0;
	P2=dat3[table[2]];
	Delayms(5);

	DS1=0;
	DS2=0;
	DS3=0;
	DS4=1;
	P2=dat1[table[3]];
	Delayms(5);

	DS1=0;
	DS2=0;
	DS3=0;
	DS4=0;
}
void init()
{
    TMOD= 0x01;				   
    TL0 = (65536-50000)/256;        //设置定时初值
    TH0 = (65536-50000)%256;        //设置定时初值
    ET0 = 1;
    TR0 = 1;
    EA = 1;
}

//void init()
//{
////    AUXR |= 0x80;                   //定时器0为1T模式
//////  AUXR &= 0x7f;                   //定时器0为12T模式
////
////    TMOD = 0x00;                    //设置定时器为模式0(16位自动重装载)
////    TL0 = T1MS;                     //初始化计时值
////    TH0 = T1MS >> 8;
////    TR0 = 1;                        //定时器0开始计时
////    ET0 = 1;                        //使能定时器0中断
////    EA = 1;
//
//	AUXR |= 0x80;		//定时器时钟1T模式
//	TMOD &= 0xF0;		//设置定时器模式
//	TL0 = 0x5C;		//设置定时初值
//	TH0 = 0xF7;		//设置定时初值
//	TF0 = 0;		//清除TF0标志
//	TR0 = 1;		//定时器0开始计时
//	ET0 = 1;                        //使能定时器0中断
//    EA = 1;
//}

bit ps=0;
char menu=0;
//bit nf=0;//闹钟开标志：受水银影响
//bit gk=0;//光控开标志
//bit nk=0;//闹钟开标志：受设置影响
//bit nx=1;//闹钟响标志：受时间影响
void key()
{
		if(set==0)
		{
			Delayms(5);
			if(set==0)
			{
				bell=0;
				menu++;
				if(menu==3){read_nao();}
				if(menu==8){menu=0;ET1 = 1;write_time();}
				while(set==0);	
				bell=1;
			}	
		}

		if(menu==0)
		{
			if(jia==0)
			{
				if(shi==nshi && fen==nfen)
				{
					nx=1;
				}
			}
		}	

		if(menu==1)
		{
			if(jia==0)
			{
				bell=0;
				if(fen >= 0x59)
					fen = 0;

				else fen=fen+0x01;
				if((fen & 0x0f) >= 0x0a)
					fen = (fen & 0xf0) + 0x10;
				while(jia==0);
				miao=0;//秒钟归零
				bell=1;
			}			
		}
		if(menu==2)
		{
			if(jia==0)
			{
				bell=0;
				shi+=0x01;
				if((shi & 0x0f) >= 0x0a)
					shi = (shi & 0xf0) + 0x10;

				if(shi >= 0x24)
					shi = 0;
				while(jia==0);
				miao=0;//秒钟归零
				bell=1;
			}		
		}
		if(menu==3)	//闹钟
		{
			if(jia==0)
			{
				bell=0;
				if(nfen >= 0x59)
					nfen = 0;
				else nfen=nfen+0x01;
				if((nfen & 0x0f) >= 0x0a)
					nfen = (nfen & 0xf0) + 0x10;
				while(jia==0);
				bell=1;	
			}
		}
		if(menu==4)
		{
			if(jia==0)
			{
				bell=0;
				nshi+=0x01;
				if((nshi & 0x0f) >= 0x0a)
					nshi = (nshi & 0xf0) + 0x10;

				if(nshi >= 0x24)
					nshi = 0;
				while(jia==0);
				bell=1;
			}	
		}
		if(menu==5)
		{
			if(jia==0)
			{
				while(jia==0);
				gk=!gk;
			}	
		}
		if(menu==6)
		{
			if(jia==0)
			{
				while(jia==0);
				nk=!nk;
			}
			if(nk==1)
			{
				nx=0;
			}
			else if(nk==0)
			{
				nx=1;	//不响
			}	
		}
		if(menu==7)
		{
			if(jia==0)
			{
				while(jia==0);
				sw=!sw;			//
			}
		}

	  write_setting();	 //断电保存
//	}	
}
void InitADC()
{
  	P1ASF = 0x7f;                	//Open channels ADC function 0100 0000 p1.6使用AD功能
	ADC_RES  = 0;                    		//Clear previous result
	ADC_LOW2 = 0;
  	ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
}


void GetADCResult(unsigned char ch,unsigned int *value)
{
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
    _nop_();                        //Must wait before inquiry
    _nop_();
    _nop_();
    _nop_();
    _nop_();                        //Must wait before inquiry
    _nop_();
    while(!(ADC_CONTR & ADC_FLAG));//Wait complete flag
    ADC_CONTR &= ~ADC_FLAG;         //Close ADC

	*value = 0;
	*value = ADC_RES;
	*value = ((*value)*4 + ADC_LOW2);		//Return ADC result.×￠êíμ?′????ò·μ??8??ADC?á1?
}

void ADC_convert(void)
{
	GetADCResult(3,&temp[0]);		  //热敏AD采样
	temp[0] = (unsigned int) ( ( 3950.0 / ( 11.33657 + log( 6.04 * (float)temp[0] / ( 1024.0 - (float)temp[0] ) ) ) - 278.15) * 100 );//273.15
}

bit flag1=1;
bit w_flag=0;
void main()
{	
	uchar i=0;

//
	P2M0=0xFF;
	P2M1=0x00; 
//	
	P3M0 = 0xF0;		  
	P3M1 = 0x00;

  init();
	InitADC();
	init_ds1302_io();
	init_ds1302();
	read_nao();
	read_setting();
	if(gk>2)
	{gk=0;nk=0;nx=1;sw=0;}
	while(1)
	{

		key();

			if(menu==0)
			{
				if((flag1==0)&&(sw==0))   //flag1=0，显示温度
				{
					if(w_flag==0)		   //定时刷新温度值
					{
					 	ADC_convert();
						w_flag=1;
					}
					
					table[0]=temp[0]/1000;
					table[1]=temp[0]%1000/100;
					table[2]=temp[0]%1000%100/10;
					table[3]=11;
					wd_display();
					Delayms(ld);				
				}
	
				if((flag1==1)||(sw==1))
				{	
					if(Flag_Lum_AD==1)
					{
						lum_temp=0;
						Flag_Lum_AD=0;

						GetADCResult(2,&Lum_Val_Buf[Lum_Count++]);		 //光敏AD采样	

						if(Lum_Count>=10)					   			//采样10次后，算出平均值
						{
							Lum_Count=0;
							for(i=0;i<10;i++)
							{
								lum_temp=lum_temp+Lum_Val_Buf[i];
							}
							temp[2]=lum_temp/10;						
						}
					}

					if(gk==1)//开光控
					{

						switch(temp[2]/150)
						{
							case 4 :ld=120;break;
							case 3 :ld=80;break;
							case 2 :ld=40;break;
							case 1 :ld=10;break;
							case 0 :ld=2;break;
							default :ld=180;break;
						}
	
					}
					else ld=2;

					read_time();

					table[0]=shi/16;
					table[1]=shi%16;
					table[2]=fen/16;
					table[3]=fen%16;

					display();
					Delayms(ld);			
				}			
			}
			else
			{
				if(menu==1)
				{	
					if(flag>0)
					{
						table[2]=fen/16;
						table[3]=fen%16;
					}
					else
					{
						table[2]=10;
						table[3]=10;				
					}
					table[0]=shi/16;
					table[1]=shi%16;			
				}
				if(menu==2)
				{
					table[2]=fen/16;
					table[3]=fen%16;
					if(flag>0)
					{
						table[0]=shi/16;
						table[1]=shi%16;				
					}
					else
					{
						table[0]=10;
						table[1]=10;	
					}
				}
				if(menu==3)
				{	
					if(flag>0)
					{
						table[2]=nfen/16;
						table[3]=nfen%16;
					}
					else
					{
						table[2]=10;
						table[3]=10;				
					}
					table[0]=nshi/16;
					table[1]=nshi%16;			
				}
				if(menu==4)
				{
					table[2]=nfen/16;
					table[3]=nfen%16;
					if(flag>0)
					{
						table[0]=nshi/16;
						table[1]=nshi%16;				
					}
					else
					{
						table[0]=10;
						table[1]=10;	
					}
				}
				if(menu==5)
				{
					table[0]=gk;
					table[1]=gk;
					table[2]=gk;
					table[3]=gk;	
				}
				if(menu==6)
				{
					table[0]=nk;
					table[1]=nk;
					table[2]=nk;
					table[3]=nk;	
				}
				if(menu==7)
				{
					table[0]=sw;
					table[1]=sw;
					table[2]=sw;
					table[3]=sw;	
				}

				set_display();			
			}				

	}
}																						

uint t=0;
uint t1=0;
uchar t2;
void InitTimer1() interrupt 1  // 1毫秒@11.0592MHz
{
	static t_lum_count=0;
    TL0 = (65536-50000)/256;        //设置定时初值			//50ms	 
    TH0 = (65536-50000)%256;        //设置定时初值
	t++;
	t1++;
	t_lum_count++;
	if(t_lum_count>2)		   	//定时采样光敏AD
	{
		t_lum_count=0;	
		Flag_Lum_AD=1;
	}
	if(t==40)					 //11.0592Mhz   20		 22.1182Mhz   40
	{
		t=0;
		w_flag=0;
		flag=!flag;			   //时钟中间两点闪烁及选择闪烁标志位
		
			if((nx==0)&&(shi==nshi && fen==nfen))bell=!bell;		
			else{bell=1;}
//		if(nx==0)bell=!bell;
	}
	if(t1>0 && t1<350)flag1=1;				   //11.0592Mhz
	if(t1>350 && t1<500)flag1=0;	 //430
	if(t1>500)t1=0;			   //430


}

//void Uart() interrupt 3
//{
//	if (TI)
//	{
//	    TI = 0;                 //清除TI位
//	    busy = 0;               //清忙标志
//	}
//}
