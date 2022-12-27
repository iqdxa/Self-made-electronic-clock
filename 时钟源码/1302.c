#include "STC15F2K.h"	         //���õ�Ƭ��ͷ�ļ�

#define uchar unsigned char  //�޷����ַ��� �궨��	������Χ0~255
#define uint  unsigned int	 //�޷������� �궨��	������Χ0~65535

/******************
	ds1302 �ڲ�RAM   RAM0  1100 000R/W   1�� 0д
					 RAM1  1100 001R/W  
					       ....... 
					 RAM30 1111 110R/W   
********************/

//sbit clk = P3^2;	  //ds1302ʱ���߶���
//sbit io =  P0^1;	  //������
//sbit rst = P0^0;	  //��λ��

sbit clk = P3^2;	  //ds1302ʱ���߶���			15W408AS
sbit io =  P5^5;	  //������
sbit rst = P5^4;	  //��λ��
						
						//��  ��   ʱ   ��   ��  ��   ���� 	
uchar code write_add[]={0x80,0x82,0x84};   //д��ַ,0x86,0x88,0x8c,0x8a
//uchar code read_add[] ={0x81,0x83,0x85};   //����ַ,0x87,0x89,0x8d,0x8b
uchar code init_ds[]  ={0x55,0x17,0x15,0x01,0x01,0x13,0x13};   
int fen,shi,miao;//,ri,yue,week,nian=0x20;

int nfen=0;
int nshi=0;

uchar nk=0;
uchar gk=0;
bit nx=0;
bit sw=0;	//�¶�ʱ���л�


/*************дһ�����ݵ���Ӧ�ĵ�ַ��***************/
void write_ds1302(uchar add,uchar dat)
{
	uchar i;		
	rst = 1;			 //�Ѹ�λ���ø�
	for(i=0;i<8;i++)
	{				     //��λ��ǰ
		clk = 0;		 //ʱ�����õͿ�ʼд����
		io = add & 0x01;    	
		add >>= 1;		 //�ѵ�ַ����һλ
		clk = 1;		 //ʱ�����ø�
	}	
	for(i=0;i<8;i++)
	{
		clk = 0;		 //ʱ�����õͿ�ʼд����
		io = dat & 0x01;
		dat >>= 1;		 //����������һλ
		clk = 1;		 //ʱ�����ø�
	}
	rst = 0;			 //��λ�ߺϵ�
	clk = 0;
	io = 0;
}

/*************�Ӷ�Ӧ�ĵ�ַ��һ�����ݳ���***************/
uchar read_ds1302(uchar add)
{
	uchar value,i;
	rst = 1;			 //�Ѹ�λ���ø�
	for(i=0;i<8;i++)
	{				     //��λ��ǰ
		clk = 0;		 //ʱ�����õͿ�ʼд����
		io = add & 0x01;    	
		add >>= 1;		 //�ѵ�ַ����һλ
		clk = 1;		 //ʱ�����ø�
	}		
	for(i=0;i<8;i++)
	{
		clk = 0;		 //ʱ�����õͿ�ʼ������
		value >>= 1;
		if(io == 1)
			value |= 0x80;
		clk = 1;		 //ʱ�����ø�
	}
	rst = 0;			 //��λ�ߺϵ�
	clk = 0;
	io = 0;
	return value;		 //���ض�����������
}

/*************��Ҫ��ʱ�� ������ ��������***************/
void read_time()
{
//	miao = read_ds1302(read_add[0]);	//����

	fen  = read_ds1302(0x83);	//����
	shi  = read_ds1302(0x85);	//��ʱ

//	ri   = read_ds1302(read_add[3]);	//����
//	yue  = read_ds1302(read_add[4]);	//����
//	nian = read_ds1302(read_add[5]);	//����
//	week = read_ds1302(read_add[6]);	//������
}

/*************��Ҫд��ʱ�� ������ ��д��ds1302��***************/
void write_time()
{
	write_ds1302(0x8e,0x00);			//��д����
//	write_ds1302(write_add[0],miao);	//д��
	write_ds1302(0x80,miao);		//д��
	write_ds1302(0x82,fen);		//д��
	write_ds1302(0x84,shi);		//дʱ
//	write_ds1302(write_add[3],ri);		//д��
//	write_ds1302(write_add[4],yue);		//д��
//	write_ds1302(write_add[5],nian);	//д��
//	write_ds1302(write_add[6],week);	//д����

	write_ds1302(0xc2,nshi);			//дʱ
	write_ds1302(0xc4,nfen);			//д��

	write_ds1302(0x8e,0x80);			//�ر�д����
}

void read_nao()
{
	nshi = read_ds1302(0xc3);	//������ʱ
	nfen = read_ds1302(0xc5);	//�����ӷ�
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
	
	nk = read_ds1302(0xc7);	             //					1100 0111 ��ȡRam3
	gk = read_ds1302(0xc9);				 //					1100 1001 ��ȡRam4
	nx = read_ds1302(0xcb);
	sw = read_ds1302(0xcd);
}





/*************�����ݱ��浽ds1302 RAM��**0-31*************/
void write_ds1302ram(uchar add,uchar dat)
{
	add <<= 1;     //��ַ�Ǵӵڶ�λ��ʼ��
	add &= 0xfe;   //�����λ����  ��д������
	add |= 0xc0;   //��ַ�����λΪ 1  
	write_ds1302(0x8e,0x00);
	write_ds1302(add,dat);	
	write_ds1302(0x8e,0x80);
}

/*************�����ݴ�ds1302 RAM������**0-31*************/
uchar read_ds1302ram(uchar add)
{
	add <<= 1;     //��ַ�Ǵӵڶ�λ��ʼ��
	add |= 0x01;   //�����λ��1  �Ƕ�����
	add |= 0xc0;   //��ַ�����λΪ 1  
	return(read_ds1302(add));	
}

/*************��ʼ��ds1302ʱ��***************/
void init_ds1302()
{
	uchar i;
	rst = 0;	//��һ�ζ�д����ʱҪ��IOƷ�õ�
	clk = 0;
	io = 0;
	i = read_ds1302ram(30);   
	if(i != 3)
	{	
		i = 3;
		write_ds1302ram(30,i);
		write_ds1302(0x8e,0x00);	            //��д����
		for(i=0;i<3;i++)
			write_ds1302(write_add[i],init_ds[i]);	//�����λֵ0 ����ds1302����
		write_ds1302(0x8e,0x80);	//��д����
	}
}

void init_ds1302_io()
{
	rst = 0;	//��һ�ζ�д����ʱҪ��IOƷ�õ�
	clk = 0;
	io = 0;	
}
