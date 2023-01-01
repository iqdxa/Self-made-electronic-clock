#include "STC15F2K.h"
#include "intrins.h"
#include <math.h>

#define uchar unsigned char		//无符号字符型 宏定义	变量范围0~255
#define uint  unsigned int		//无符号整型 宏定义	变量范围0~65535

#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟

sfr ADC_LOW2    =   0xBE;           //ADC低2位结果

bit nx;
bit sw;
bit nx=0;
bit sw=0;				//温度时间切换

sbit clk = P3^2;	  	//ds1302时钟线定义
sbit io =  P5^5;	  	//数据线
sbit rst = P5^4;	  	//复位线
sbit DS4=P3^4;
sbit DS3=P3^5;
sbit DS2=P3^6;
sbit DS1=P3^7;
sbit sw1=P3^0;
sbit sw2=P3^1;

//						秒		分	时	日		月	年	星期
uchar code init_ds[]  ={0x00,0x00,0x00,0x01,0x01,0x00,0x13}; 
uchar code write_add[]={0x80,0x82,0x84,0x86,0x88,0x8c,0x8a};   //写地址
uchar code read_add[] ={0x81,0x83,0x85,0x87,0x89,0x8d,0x8b};   //读地址 
uchar dat1[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xf8,0X80,0X90,0xff,0xc6};//正立无小数点
uchar dat2[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0X00,0X10,0xff};//正立有小数点
uchar dat3[]={0x40,0x4F,0x24,0x06,0x0B,0x12,0x10,0x47,0X00,0X02,0xff};//倒立有小数点
uchar dat4[]={0xC0,0xCF,0xA4,0x86,0x8B,0x92,0x90,0xC7,0X80,0X82,0xff};//倒立无小数点
uchar Differ_Time=0;//不同数字所需要延时时间
uchar table[4]={0};
uchar nk=0;
uchar gk=0;
uchar menu=0;
uchar i=0;
uchar point_display=0;

uint tem=0,lum=0;
uint ld;
uint fen;
uint shi;
uint miao;
uint fen,shi,miao,ri,yue,week,nian=0x20;
uint is_not_display_tem=0;
uint is_not_display_lum=0;
uint is_not_collect=0;
uint two_point_flag=0;
uint cycle_display_flag=0;
uint AD_collect_flag=0;

void Delayms(uint t)
{
	uchar i;
	while(t--)for(i=0;i<123;i++);
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

//local小数点位置,is_not_time是否是显示时间的格式
void display(uint local,uint is_not_time)
{
	P2=0XFF;
	DS1=1;
	DS2=0;
	DS3=0;
	DS4=0;
	if(local==0)P2=dat2[table[0]];
	else P2=dat1[table[0]];
	Delay_select(table[0]);
	Delayms(Differ_Time);

	P2=0XFF;
	DS1=0;
	DS2=1;
	DS3=0;
	DS4=0;
	if(is_not_time==1)
	{
		if(point_display==1)P2=dat2[table[1]];
		else if(point_display==0)P2=dat1[table[1]];
	}
	else if(is_not_time==0)
	{
		if(local==1)P2=dat2[table[1]];
		else P2=dat1[table[1]];
	}
	Delay_select(table[1]);
	Delayms(Differ_Time);

	P2=0XFF;
	DS1=0;
	DS2=0;
	DS3=1;
	DS4=0;
	//如果是显示时间，则第三个数码管一定要显示小数点，且小数点闪烁
	//如果不是显示时间，则根据需求显示
	if(is_not_time==1)
	{
		if(point_display==1)P2=dat3[table[2]];
		else if(point_display==0)P2=dat4[table[2]];
	}
	else if(is_not_time==0)
	{
		if(local==2)P2=dat3[table[2]];
		else P2=dat4[table[2]];
	}
	Delay_select(table[2]);
	Delayms(Differ_Time);

	P2=0XFF;
	DS1=0;
	DS2=0;
	DS3=0;
	DS4=1;
	if(local==3)P2=dat2[table[3]];
	else P2=dat1[table[3]];
	Delay_select(table[3]);
	Delayms(Differ_Time);

	DS1=0;
	DS2=0;
	DS3=0;
	DS4=0;
}

/*************写一个数据到对应的地址里***************/
void write_ds1302(uchar add,uchar dat)
{
	uchar i;
	rst = 1;			 //把复位线拿高
	for(i=0;i<8;i++)
	{				     //低位在前
		clk = 0;		 //时钟线拿低开始写数据
		io = add & 0x01;
		add >>= 1;		 //把地址右移一位
		clk = 1;		 //时钟线拿高
	}	
	for(i=0;i<8;i++)
	{
		clk = 0;		 //时钟线拿低开始写数据
		io = dat & 0x01;
		dat >>= 1;		 //把数据右移一位
		clk = 1;		 //时钟线拿高
	}
	rst = 0;			 //复位线合低
	clk = 0;
	io = 0;
}

//从对应的地址读一个数据出来
uchar read_ds1302(uchar add)
{
	uchar value,i;
	rst = 1;			 //把复位线拿高
	for(i=0;i<8;i++)
	{				     //低位在前
		clk = 0;		 //时钟线拿低开始写数据
		io = add & 0x01;    	
		add >>= 1;		 //把地址右移一位
		clk = 1;		 //时钟线拿高
	}		
	for(i=0;i<8;i++)
	{
		clk = 0;		 //时钟线拿低开始读数据
		value >>= 1;
		if(io == 1)
			value |= 0x80;
		clk = 1;		 //时钟线拿高
	}
	rst = 0;			 //复位线合低
	clk = 0;
	io = 0;
	return value;		 //返回读出来的数据
}

void read_time()
{
	miao = read_ds1302(read_add[0]);	//读秒
	fen  = read_ds1302(read_add[1]);	//读分
	shi  = read_ds1302(read_add[2]);	//读时
	ri   = read_ds1302(read_add[3]);	//读日
	yue  = read_ds1302(read_add[4]);	//读月
	nian = read_ds1302(read_add[5]);	//读年
	week = read_ds1302(read_add[6]);	//读星期
}

/*************把要写的时间 年月日 都写入ds1302里***************/
void write_time()
{
	write_ds1302(0x8e,0x00);			//关闭写保护
	write_ds1302(write_add[0],miao);	//写秒
	write_ds1302(write_add[1],fen);		//写分
	write_ds1302(write_add[2],shi);		//写时
	write_ds1302(write_add[3],ri);		//写日
	write_ds1302(write_add[4],yue);		//写月
	write_ds1302(write_add[5],nian);	//写年
	write_ds1302(write_add[6],week);	//写星期
	write_ds1302(0x8e,0x80);			//打开写保护
}

void write_setting()
{
	write_ds1302(0x8e,0x00);
	write_ds1302(0xcc,sw);
	write_ds1302(0xca,nx);
	write_ds1302(0xc8,gk);
	write_ds1302(0xc6,nk);
	write_ds1302(0x8e,0x80);
}

void read_setting()
{
	nk = read_ds1302(0xc7);					//1100 0111 读取Ram3
	gk = read_ds1302(0xc9);					//1100 1001 读取Ram4
	nx = read_ds1302(0xcb);
	sw = read_ds1302(0xcd);
}

//把数据保存到ds1302 RAM中**0-31
void write_ds1302ram(uchar add,uchar dat)
{
	add <<= 1;     //地址是从第二位开始的
	add &= 0xfe;   //把最低位清零  是写的命令
	add |= 0xc0;   //地址最高两位为 1  
	write_ds1302(0x8e,0x00);
	write_ds1302(add,dat);	
	write_ds1302(0x8e,0x80);
}

//把数据从ds1302 RAM读出来**0-31
uchar read_ds1302ram(uchar add)
{
	add <<= 1;     //地址是从第二位开始的
	add |= 0x01;   //把最高位置1  是读命令
	add |= 0xc0;   //地址最高两位为 1  
	return(read_ds1302(add));
}

void GetADCResult(unsigned char ch,unsigned int *value)
{
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
	_nop_();                        		//Must wait before inquiry
	_nop_();
	_nop_();
	_nop_();
	_nop_();                        		//Must wait before inquiry
	_nop_();
	while(!(ADC_CONTR & ADC_FLAG));			//Wait complete flag
	ADC_CONTR &= ~ADC_FLAG;         		//Close ADC
	
	*value = 0;
	*value = ADC_RES;
	*value = ((*value)*4 + ADC_LOW2);		//Return ADC result
}

void choose_display(uchar choose)
{
	if(choose == 't')
	{
		read_time();
		table[0]=shi/16;
		table[1]=shi%16;
		table[2]=fen/16;
		table[3]=fen%16;
		display(1,1);
		Delayms(ld);
	}
	else if(choose == 'T')
	{
		if(tem>=10)
		{
			table[0]=tem/1000;
			table[1]=tem%1000/100;
			table[2]=tem%1000%100/10;
			table[3]=11;
		}
		else if(tem<10)
		{
			table[0]=tem%1000/100;
			table[1]=tem%1000%100/10;
			table[2]=tem%1000%100%10;
			table[3]=11;
		}
		display(1,0);
		Delayms(ld);
	}
	else if(choose == 'l')
	{
		table[0]=lum/1000;
		table[1]=lum%1000/100;
		table[2]=lum%1000%100/10;
		table[3]=lum%1000%100%10;
		display(3,0);
		Delayms(ld);
	}
	//菜单四和五设置时间的显示
	else if(choose == 's')
	{
		display(1,1);
		Delayms(ld);
	}
}

void display_menu()
{
	//菜单一循环显示
	//菜单二只显示时间
	//菜单三只显示温度
	//菜单四只显示光强度
	//菜单五调整时
	//菜单六调整分
	if(menu==0)
	{
		if(is_not_display_tem==1)choose_display('T');      	//一定条件下显示温度
		else if(is_not_display_lum==1)choose_display('l'); 	//一定条件下显示发光强度
		else choose_display('t'); 							//其他条件下显示时间
	}
	if(menu==1)choose_display('t');							//只显示时间
	if(menu==2)choose_display('T');							//只显示温度
	if(menu==3)choose_display('l');							//只显示光强
	//调整时
	if(menu==4)
	{
		//闪烁
		if(point_display==1)
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
		choose_display('s');
	}
	//调整分
	if(menu==5)
	{
		table[2]=fen/16;
		table[3]=fen%16;
		//闪烁
		if(point_display>0)
		{
			table[0]=shi/16;
			table[1]=shi%16;
		}
		else
		{
			table[0]=10;
			table[1]=10;
		}
		choose_display('s');
	}
}

//收集温度和光强度
void collect_tem_and_lum()
{
	//允许采样
	if(is_not_collect==1)
	{
		is_not_collect=0;
		GetADCResult(2,&lum);		//光敏AD采样
		GetADCResult(3,&tem);		//热敏AD采样
		tem = (unsigned int) ( ( 3950.0 / ( 11.33657 + log( 6.04 * (float)tem / ( 1024.0 - (float)tem ) ) ) - 278.15) * 100 );
	}
	//光控制显示亮度
	ld=(lum/50)*5;
}

void key()
{
	if(sw1==0)
	{
		menu++;
		if(menu==6){menu=0;write_time();}
		while(sw1==0);//确保松手之前不变
	}
	//在时间显示页面，按下sw2就可以初始化时间
	if(menu==0 || menu==1)
	{
		if(sw2==0)
		{
			write_ds1302(0x8e,0x00);	//关闭写保护
			for(i=0;i<3;i++)write_ds1302(write_add[i],init_ds[i]);	//把最高位值0 允许ds1302工作
			write_ds1302(0x8e,0x80);	//打开写保护
			while(sw2==0);
		}
	}
	if(menu==4)
	{
		if(sw2==0)
		{
			if(fen >= 0x59)fen=0;
			else fen=fen+0x01;
			if((fen & 0x0f) >= 0x0a)fen = (fen & 0xf0) + 0x10;
			while(sw2==0);
			miao=0;//秒钟归零
		}
	}
	if(menu==5)
	{
		if(sw2==0)
		{
			shi+=0x01;
			if((shi & 0x0f) >= 0x0a)shi = (shi & 0xf0) + 0x10;
			if(shi >= 0x24)shi = 0;
			while(sw2==0);
			miao=0;//秒钟归零
		}
	}
	write_setting();	 //断电保存
}

void init()
{
	uchar i;

	P2M0=0xFF;
	P2M1=0x00;
	P3M0 = 0xF0;
	P3M1 = 0x00;
	
	//init
	TMOD= 0x01;
	TL0 = (65536-50000)/256;        //设置定时初值
	TH0 = (65536-50000)%256;        //设置定时初值
	ET0 = 1;
	TR0 = 1;
	EA = 1;
	
	//InitADC
	P1ASF = 0x7f;                	//Open channels ADC function 0100 0000 p1.6使用AD功能
	ADC_RES  = 0;                    		//Clear previous result
	ADC_LOW2 = 0;
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
	
	//init_ds1302_io
	rst = 0;	//第一次读写数据时要把IO品拿低
	clk = 0;
	io = 0;
	
	//init_ds1302
	//初始化ds1302时间
	rst = 0;	//第一次读写数据时要把IO品拿低
	clk = 0;
	io = 0;
	i = read_ds1302ram(30);   
	if(i != 3)
	{
		i = 3;
		write_ds1302ram(30,i);
		write_ds1302(0x8e,0x00);	//关闭写保护
		for(i=0;i<3;i++)
			write_ds1302(write_add[i],init_ds[i]);	//把最高位值0 允许ds1302工作
		write_ds1302(0x8e,0x80);	//打开写保护
	}
}

void main()
{
	init();
	read_setting();
	
	while(1)
	{
		key();
		collect_tem_and_lum();
		display_menu();
	}
}

void InitTimer1() interrupt 1  // 1毫秒@11.0592MHz
{
	TL0 = (65536-50000)/256;        //设置定时初值
	TH0 = (65536-50000)%256;        //设置定时初值
	
	two_point_flag++;
	cycle_display_flag++;
	AD_collect_flag++;
	
	//定时采样光敏和热敏AD采样flag
	if(AD_collect_flag>20)
	{
		AD_collect_flag=0;
		is_not_collect=1;
	}
	
	//时钟中间两点闪烁及选择闪烁flag
	if(two_point_flag==30)
	{
		two_point_flag=0;
		point_display=!point_display;//时钟中间两点闪烁及选择闪烁标志位
	}
	
	//菜单一循环显示flag
	if(cycle_display_flag>0 && cycle_display_flag<300)
	{
		is_not_display_tem=0;
		is_not_display_lum=0;
	}
	if(cycle_display_flag>300 && cycle_display_flag<500)
	{
		is_not_display_tem=1;
		is_not_display_lum=0;
	}
	if(cycle_display_flag>500 && cycle_display_flag<700)
	{
		is_not_display_tem=0;
		is_not_display_lum=1;
	}
	if(cycle_display_flag>700)
	{
		cycle_display_flag=0;//重新开始
	}
}
