/********************* (C) COPYRIGHT 2010 e-Design Co.,Ltd. ********************
 File Name : main.c  
 Version   : DS203_SYS Ver 1.3x                                  Author : bure
*******************************************************************************/
#include "SPI_flash.h"
#include "Function.h"
#include "USB_lib.h"
#include "Config.h"
#include "Ident.h"
#include "BIOS.h"
#include "LCD.h"
#include "ASM.h"
#include "USB_pwr.h"

typedef  void (*pFunc)(void);

pFunc Jump_To_App;
u32 JumpAddr;

/*******************************************************************************
  版本修改说明
SYS V1.24: 修改了144MHz交替采样模式下的插值系数(BIOS.C)
           修改了V2.6硬件电路下模拟输入量程档位(BIOS.C)
SYS V1.30: 将Licence识别函数转移到了DFU模块中(ASM.s,Ident.c)
           修改了Licence权限控制力度(Main.c, Ident.c)
           修改了DFU,SYS库函数向量及其调用方式(ASM.s, VECTOR.c)
           添加了新的SYS库函数(BIOS.C)
SYS V1.31: 将内部U盘的初始化推迟到Licence识别后进行(Main.c,Config.c)
           修改了U盘扇区读写程序(SPI_flash.c)
           修改了SYS库函数向量表存放起始地址
SYS V1.32: 修改了Set(T_BASE_ARR, Value)函数中的BUG(BIOS.C)
SYS V1.33: 增加了对LCD控制芯片ILI9327与R61509V的兼容和自动识别(LCD.c, ASM.s)
SYS V1.34: 从该版本起 可并行使用 IAR 4.42与５.０版本 增加了文件夹　IAR_V5_Prpject
           修改了ASM.s文件
          

*******************************************************************************/

/*******************************************************************************
  main : Main routine.
*******************************************************************************/
int main(void)
{ 
  u32 Licence;
  u8  NumStr[9];
  
  Set_System();
  NVIC_Config();
  GPIO_Config();
  Timer_Config();
  FSMC_Config();
  SPI_Config();

  DMA_Config();
  ADC_Config();
  DAC_Config();
  
  Beep(20, 500); 
  LCD_Initial();
  Clear_Screen(0x0000);

  Display_Logo(70,150); 
  
  Display_Str( 6*8, 90, GRN, PRN,  HDW_VERSION );
  Display_Str(24*8, 90, GRN, PRN, "Serial No: ");
  Word2Hex(NumStr, __Dev_SN());            
  Display_Str(35*8, 90, YEL, PRN, NumStr);    // 显示读取到的设备序列号
  Display_Str(6*8, 70, GRN, PRN,   SYS_VERSION);

  if((KEY2_OFF)&&(KEY3_OFF)&&(KEY4_OFF)){       // 开机无按键按下
    Key_Buffer = 0;                             // 清除按键状态，开始等待新的按键
    Licence = Get_Lic(SYS_LIC_OFFSET);          // Get master licence
    if(__Chk_SYS(Licence)!= 1){                 // No master licence 
      Display_Str(6*8,50,GRN,PRN,     "Please input master licence: 00000000");
      Licence = Input_Lic(48+29*8, 50);         // Input Licence
      if(__Chk_SYS(Licence)!= 1){               // Licence error
        Display_Str(6*8,50,YEL,PRN,   "        Licence input error          ");
      } else {                                  // Licence ok
        if(Add_Lic(Licence, SYS_LIC_OFFSET)== 1)// Licence writen ok
          Display_Str(6*8,50,GRN,PRN, "         Licence writen ok           ");
        else  
          Display_Str(6*8,50,GRN,PRN, "        Licence writen error         ");
      }
      Display_Str(6*8,30,GRN,PRN,     "      Push any key to next step      ");
      while (Key_Buffer == 0){};
    }
    JumpAddr = *(vu32*) (APP1_BASE + 4);
    Jump_To_App = (pFunc) JumpAddr;
    __MSR_MSP(*(vu32*) APP1_BASE);
  } else if(KEY2_ON){                           // 开机按下K2键  
    JumpAddr = *(vu32*) (APP2_BASE + 4);
    Jump_To_App = (pFunc) JumpAddr;
    __MSR_MSP(*(vu32*) APP2_BASE);
  } else if(KEY3_ON){                           // 开机按下K3键 
    JumpAddr = *(vu32*) (APP3_BASE + 4);
    Jump_To_App = (pFunc) JumpAddr;
    __MSR_MSP(*(vu32*) APP3_BASE);
  } else if(KEY4_ON){                           // 开机按下K4键 
    JumpAddr = *(vu32*) (APP4_BASE + 4);
    Jump_To_App = (pFunc) JumpAddr;
    __MSR_MSP(*(vu32*) APP4_BASE);
  }
  USB_EN();
  USB_Init();  
  Jump_To_App();
}
/*********************************  END OF FILE  ******************************/
