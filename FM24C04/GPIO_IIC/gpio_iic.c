#include "gpio_iic.h"



static uint8_t sda_num;
static uint8_t scl_num;

#define TIME_DELAY 200//80M busclk 38Khz

void delay(uint16_t time)//使用时需要严格按照时序，MCU的时钟
{
	while(time--);	
}


void iic_init(uint8_t select)
{
		//always write protect when init
		SET_VALUE(IIC_WP_NUM_EEPROM);//write protect
		WP_OUT(IIC_WP_NUM_EEPROM); 	

    if(SELECT_EEPROM==select)
    {
	    sda_num = IIC_SDA_NUM_EEPROM;
	    scl_num = IIC_SCL_NUM_EEPROM;
    }    
    else if(SELECT_ENCRYPT==select)
    {
	    sda_num = IIC_SDA_NUM_ENCRYPT;
	    scl_num = IIC_SCL_NUM_ENCRYPT;
    }    
	
    SET_VALUE(sda_num);
    SET_VALUE(scl_num);
		SDA_OUT(sda_num);
    SCL_OUT(scl_num);
}

//SCL 为高电平时，SDA由高电平向低电平跳变，产生start信号 开始传送数据
/*IIC START:SCL = 1,SDA = 1->0*/
void iic_start(void)
{		
		SET_VALUE(sda_num);
		SET_VALUE(scl_num);
    delay(TIME_DELAY);    
    RESET_VALUE(sda_num);
    delay(TIME_DELAY);
}


//SCL为高电平时，SDA由低电平向高电平跳变，结束传送数据

/*IIC STOP:SCL = 1,SDA = 0->1*/
void iic_stop(void)
{
    RESET_VALUE(scl_num);
    RESET_VALUE(sda_num);
   	delay(TIME_DELAY);
    SET_VALUE(scl_num);
    delay(TIME_DELAY);
    //SCL为高电平时，SDA从低电平跳变到高电平,产生停止信号
		SET_VALUE(sda_num);
    delay(TIME_DELAY);
    delay(TIME_DELAY);
}

/*Write 1 Byte to IIC*/
void iic_write_byte(uint8_t data)
{
    uint8_t loop;
    //在SCL高时，SDA必须保持稳定,SCL低时，SDA可以任意改变
    //此处将SCL拉低的目的是，接下来就要发送数据了
    RESET_VALUE(scl_num);
    //delay(TIME_DELAY);
    for(loop=8; loop>0; loop--)
		{
				delay(TIME_DELAY);
		      //先发送最高位,在SCL高电平时,SDA必须保持稳定,MSB
		    if(data & (1<<(loop-1)))//modified
				{
					SET_VALUE(sda_num);	
				}
		    else
				{
					RESET_VALUE(sda_num);	
				}
				delay(TIME_DELAY);//added
				SET_VALUE(scl_num);
				delay(TIME_DELAY);
		  	RESET_VALUE(scl_num);//SCL为低电平时，SDA可以任意改变
		}	
}

/*Read 1 byte from IIC*/
uint8_t iic_read_byte(void)
{
    uint8_t loop;
    uint8_t value;
    
		value = 0;
		SET_VALUE(sda_num);
    SDA_IN(sda_num);//added by xyl
    delay(TIME_DELAY);//added by xyl
    for(loop = 8; loop > 0;loop --)
    {
    		SET_VALUE(scl_num);
        delay(TIME_DELAY);
        value <<= 1;
        //读取1位数据
        value |= GET_VALUE(sda_num);
    		RESET_VALUE(scl_num);
        delay(TIME_DELAY);
    }
    SDA_OUT(sda_num);//added by xyl
		SET_VALUE(sda_num);
    delay(TIME_DELAY);//added by xyl

    return value;
}


//主机向从机发送ACK信号
void iic_send_ack(void)
{
   	RESET_VALUE(sda_num);
    delay(TIME_DELAY);
   	SET_VALUE(scl_num);
    delay(TIME_DELAY);
   	RESET_VALUE(scl_num);
    delay(TIME_DELAY);
}

//主机向从机发送NO-ACK信号
void iic_send_notack(void)
{
   	SET_VALUE(sda_num);
    delay(TIME_DELAY);
   	SET_VALUE(scl_num);
    delay(TIME_DELAY);
   	RESET_VALUE(scl_num);
    delay(TIME_DELAY); 
   	RESET_VALUE(sda_num);
   	delay(TIME_DELAY);
}

//获取从机给主机的ACK信号
//if return 1,exit, ACK 0 OK 
uint8_t iic_get_ack(void)//SDA in out 加入里面
{
    uint8_t ret;
    
    SDA_IN(sda_num);//added by xyl
    delay(TIME_DELAY);//added by xyl
   	SET_VALUE(scl_num);
    delay(TIME_DELAY);
    ret = GET_VALUE(sda_num);
   	RESET_VALUE(scl_num);
    delay(TIME_DELAY);
    SDA_OUT(sda_num);//added by xyl
    delay(TIME_DELAY);//added by xyl
    
    return ret;
}



