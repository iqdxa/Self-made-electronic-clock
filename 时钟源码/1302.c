#include "STC15F2K.h"	         //调用单片机头文件

#define uchar unsigned char  //无符号字符型 宏定义	变量范围0~255
#define uint  unsigned int	 //无符号整型 宏定义	变量范围0~65535

/******************
	ds1302 内部RAM   RAM0  1100 000R/W   1读 0写
					 RAM1  1100 001R/W  
					       ....... 
					 RAM30 1111 110R/W   
********************/

//sbit clk = P3^2;	  //ds1302时钟线定义
//sbit io =  P0^1;	  //数据线
//sbit rst = P0^0;	  //复位线

sbit clk = P3^2;	  //ds1302时钟线定义			15W408AS
sbit io =  P5^5;	  //数据线
sbit rst = P5^4;	  //复位线
						
						//秒  分   时   日   月  年   星期 	
uchar code write_add[]={0x80,0x82,0x84};   //写地址,0x86,0x88,0x8c,0x8a
//uchar code read_add[] ={0x81,0x83,0x85};   //读地址,0x87,0x89,0x8d,0x8b
uchar code init_ds[]  ={0x55,0x17,0x15,0x01,0x01,0x13,0x13};   
int fen,shi,miao;//,ri,yue,week,nian=0x20;

int nfen=0;
int nshi=0;

uchar nk=0;
uchar gk=0;
bit nx=0;
bit sw=0;	//温度时间切换


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

/*************从对应的地址读一个数据出来***************/
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

/*************把要的时间 年月日 都读出来***************/
void read_time()
{
//	miao = read_ds1302(read_add[0]);	//读秒

	fen  = read_ds1302(0x83);	//读分
	shi  = read_ds1302(0x85);	//读时

//	ri   = read_ds1302(read_add[3]);	//读日
//	yue  = read_ds1302(read_add[4]);	//读月
//	nian = read_ds1302(read_add[5]);	//读年
//	week = read_ds1302(read_add[6]);	//读星期
}

/*************把要写的时间 年月日 都写入ds1302里***************/
void write_time()
{
	write_ds1302(0x8e,0x00);			//打开写保护
//	write_ds1302(write_add[0],miao);	//写秒
	write_ds1302(0x80,miao);		//写秒
	write_ds1302(0x82,fen);		//写分
	write_ds1302(0x84,shi);		//写时
//	write_ds1302(write_add[3],ri);		//写日
//	write_ds1302(write_add[4],yue);		//写月
//	write_ds1302(write_add[5],nian);	//写年
//	write_ds1302(write_add[6],week);	//写星期

	write_ds1302(0xc2,nshi);			//写时
	write_ds1302(0xc4,nfen);			//写分

	write_ds1302(0x8e,0x80);			//关闭写保护
}

void read_nao()
{
	nshi = read_ds1302(0xc3);	//读闹钟时
	nfen = read_ds1302(0xc5);	//读闹钟分
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
	
	nk = read_ds1302(0xc7);	             //					1100 0111 读取Ram3
	gk = read_ds1302(0xc9);				 //					1100 1001 读取Ram4
	nx = read_ds1302(0xcb);
	sw = read_ds1302(0xcd);
}





/*************把数据保存到ds1302 RAM中**0-31*************/
void write_ds1302ram(uchar add,uchar dat)
{
	add <<= 1;     //地址是从第二位开始的
	add &= 0xfe;   //把最低位清零  是写的命令
	add |= 0xc0;   //地址最高两位为 1  
	write_ds1302(0x8e,0x00);
	write_ds1302(add,dat);	
	write_ds1302(0x8e,0x80);
}

/*************把数据从ds1302 RAM读出来**0-31*************/
uchar read_ds1302ram(uchar add)
{
	add <<= 1;     //地址是从第二位开始的
	add |= 0x01;   //把最高位置1  是读命令
	add |= 0xc0;   //地址最高两位为 1  
	return(read_ds1302(add));	
}

/*************初始化ds1302时间***************/
void init_ds1302()
{
	uchar i;
	rst = 0;	//第一次读写数据时要把IO品拿低
	clk = 0;
	io = 0;
	i = read_ds1302ram(30);   
	if(i != 3)
	{	
		i = 3;
		write_ds1302ram(30,i);
		write_ds1302(0x8e,0x00);	            //打开写保护
		for(i=0;i<3;i++)
			write_ds1302(write_add[i],init_ds[i]);	//把最高位值0 允许ds1302工作
		write_ds1302(0x8e,0x80);	//关写保护
	}
}

void init_ds1302_io()
{
	rst = 0;	//第一次读写数据时要把IO品拿低
	clk = 0;
	io = 0;	
}
