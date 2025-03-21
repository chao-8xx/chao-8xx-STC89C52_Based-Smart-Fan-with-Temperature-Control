#include <REGX52.H>
#include "LCD1602.h"
#include "DS18B20.h"
#include "Delay.h"
#include "AT24C02.h"
#include "Key.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Buzzer.h"


sbit Motor = P1^0;

unsigned char Counter, Compare; // 计数值和比较值，用于输出PWM
unsigned char KeyNum, Speed;
float T, TShow;
unsigned char  THigh;

/* 温度超过阈值自动报警模块 */
void Alarm_System() {
    P2 = 0xFF;
    Delay(1000);
    P2 = 0x00;
    Delay(500);
    Buzzer_Time(1000);
}

/* 定时器0中断服务例程 */
void Timer0_Routine() interrupt 1 {
    static unsigned int T0Count;
    TL0 = 0x18;        // 设置定时初值
    TH0 = 0xFC;        // 设置定时初值
    T0Count++;
    if (T0Count >= 20) {
        T0Count = 0;
        Key_Loop();    // 每20ms调用一次按键驱动函数
    }
}

/* 定时器1中断服务例程 */
void Timer1_Routine() interrupt 3 {
    static unsigned int T1Count;
    TL0 = 0x9C;        // 设置定时初值
    TH0 = 0xFF;        // 设置定时初值
    T1Count++;
    T1Count%= 100;    // 计数值变化范围限制在0~99
    if (T1Count < Compare) { // 计数值小于比较值
        Motor = 1;        // 输出1
    } else {                // 计数值大于比较值
        Motor = 0;        // 输出0
    }
}


 /* 温度读取及显示函数 */
				void Show_Temperature()
{
        DS18B20_ConvertT();    // 转换温度
        T = DS18B20_ReadT();   // 读取温度
        if (T < 0) {          // 如果温度小于0
            LCD_ShowChar(1, 3, '-');
            TShow = -T;        // 将温度变为正数
        } else {              // 如果温度大于等于0
            LCD_ShowChar(1, 3, '+');
            TShow = T;
        }
        LCD_ShowNum(1, 4, TShow, 3);    // 显示温度整数部分
        LCD_ShowChar(1, 7, '.');
        LCD_ShowNum(1, 8, (unsigned long)(TShow * 100) % 100, 2); // 显示温度小数部分
        LCD_ShowNum(2, 4, THigh,3); // 显示阈值数据
}
		/*独立按键控制模块函数*/
		void Control_System()
		{
			KeyNum=Key();				//接收返回值
			if(KeyNum)
			{
				if(KeyNum==1)
				{
					if(T<=125)
					{
						THigh++;			//按键S1使高阈值增大
					}
				
				}
				if(KeyNum==2)
				{
					if(T<=125)
					{
						THigh--;
					}									//按键S2使高阈值减小
				}
				if(KeyNum==3){Speed++;}			//按键S3使风速增大
				if(KeyNum==4)
				{
					if(Speed!=0)
						Speed--;								//当风速不为0时，按键S4使风速减小
				}
			}
		}
		 

void main() {
    DS18B20_ConvertT();    // 上电先转换一次温度，防止第一次读数据错误
    Delay(1000);           // 等待转换完成
    THigh = AT24C02_ReadByte(0); // 读取温度阈值数据
    if (THigh > 125 ) {
        THigh = 24;       // 如果阈值非法，则设为默认值
    }
    LCD_Init();
	  Timer1_Init();
    Timer0_Init();
    LCD_ShowString(1, 1, "T:");
    LCD_ShowString(2, 1, "TH:");
	  LCD_ShowString(2, 9, "Speed:");

    while (1) {
    Show_Temperature();
		Control_System();
		Speed%=4;
		if(Speed==0){Compare=0;}
		if(Speed==1){Compare=35;}
		if(Speed==2){Compare=65;}
		if(Speed==3){Compare=88;}
   	LCD_ShowNum(2, 15, Speed,1); // 显示风速


        if (T > THigh) {    // 越界判断
            LCD_ShowString(1, 13, "OV:H");
            Alarm_System();
					  Speed++;
        }  else {
            LCD_ShowString(1, 13, "    ");
        }
			}
}


