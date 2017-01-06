#include "gpio_iic.h"



static uint8_t sda_num;
static uint8_t scl_num;

#define TIME_DELAY 200//80M busclk 38Khz

void delay(uint16_t time)//ʹ��ʱ��Ҫ�ϸ���ʱ��MCU��ʱ��
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

//SCL Ϊ�ߵ�ƽʱ��SDA�ɸߵ�ƽ��͵�ƽ���䣬����start�ź� ��ʼ��������
/*IIC START:SCL = 1,SDA = 1->0*/
void iic_start(void)
{		
		SET_VALUE(sda_num);
		SET_VALUE(scl_num);
    delay(TIME_DELAY);    
    RESET_VALUE(sda_num);
    delay(TIME_DELAY);
}


//SCLΪ�ߵ�ƽʱ��SDA�ɵ͵�ƽ��ߵ�ƽ���䣬������������

/*IIC STOP:SCL = 1,SDA = 0->1*/
void iic_stop(void)
{
    RESET_VALUE(scl_num);
    RESET_VALUE(sda_num);
   	delay(TIME_DELAY);
    SET_VALUE(scl_num);
    delay(TIME_DELAY);
    //SCLΪ�ߵ�ƽʱ��SDA�ӵ͵�ƽ���䵽�ߵ�ƽ,����ֹͣ�ź�
		SET_VALUE(sda_num);
    delay(TIME_DELAY);
    delay(TIME_DELAY);
}

/*Write 1 Byte to IIC*/
void iic_write_byte(uint8_t data)
{
    uint8_t loop;
    //��SCL��ʱ��SDA���뱣���ȶ�,SCL��ʱ��SDA��������ı�
    //�˴���SCL���͵�Ŀ���ǣ���������Ҫ����������
    RESET_VALUE(scl_num);
    //delay(TIME_DELAY);
    for(loop=8; loop>0; loop--)
		{
				delay(TIME_DELAY);
		      //�ȷ������λ,��SCL�ߵ�ƽʱ,SDA���뱣���ȶ�,MSB
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
		  	RESET_VALUE(scl_num);//SCLΪ�͵�ƽʱ��SDA��������ı�
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
        //��ȡ1λ����
        value |= GET_VALUE(sda_num);
    		RESET_VALUE(scl_num);
        delay(TIME_DELAY);
    }
    SDA_OUT(sda_num);//added by xyl
		SET_VALUE(sda_num);
    delay(TIME_DELAY);//added by xyl

    return value;
}


//������ӻ�����ACK�ź�
void iic_send_ack(void)
{
   	RESET_VALUE(sda_num);
    delay(TIME_DELAY);
   	SET_VALUE(scl_num);
    delay(TIME_DELAY);
   	RESET_VALUE(scl_num);
    delay(TIME_DELAY);
}

//������ӻ�����NO-ACK�ź�
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

//��ȡ�ӻ���������ACK�ź�
//if return 1,exit, ACK 0 OK 
uint8_t iic_get_ack(void)//SDA in out ��������
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



