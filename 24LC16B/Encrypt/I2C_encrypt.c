#include "gpio_iic.h"

#define Somenop(); asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");\
                   asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");\
                   asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");\
                   asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
/******in lib:*************/
extern unsigned char alpuc_process(unsigned char *, unsigned char *);//

//extern unsigned char _alpu_rand(void);
extern void _alpu_delay_ms(unsigned int);
extern unsigned char _i2c_write(unsigned char , unsigned char, unsigned char *, int);
extern unsigned char _i2c_read(unsigned char , unsigned char, unsigned char *, int);


void _alpu_delay_ms(unsigned int i)
{
  unsigned long index;
  
  for(index=0; index<(i*2000);index++)//1ms delay
  {
    Somenop();//0.5us
  }
}

unsigned char _alpu_rand(void)
{
    static unsigned long seed; // 2byte, must be a static variable
    seed = seed + 11;//rand()
    seed = seed * 1103515245 + 12345;
    
    return (seed/65536) % 32768;    
}

uint8_t iic_write_mul_byte(uint8_t device_addr,uint8_t sub_addr, uint16_t byte_count, uint8_t data[])
{
    uint16_t i;
    
    iic_start();

    iic_write_byte(device_addr & 0xFE);/* send salver address , R/W=0 , write mode */
    if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_CTRLBYTE;
    } 
    iic_write_byte((uint8_t)(sub_addr));   
    if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_ADDRESS;
    }

    for(i=0; i<byte_count; i++)      
    {
        iic_write_byte(data[i]);
        if(iic_get_ack())
        {
            iic_stop();
            return ERR_WRITE_DATA;
        }
    }

    iic_stop();
    
    return ERR_OK;
}


uint8_t iic_read_mul_byte(uint8_t device_addr,uint8_t sub_addr, uint16_t byte_count, uint8_t data[])
{
    uint16_t i;

    iic_start();
    iic_write_byte(device_addr & 0xFE);/* send salver address , R/W=0 , write mode */
    if(iic_get_ack())       
    {
        iic_stop();
        return ERR_WRITE_CTRLBYTE;
    }  
    iic_write_byte((uint8_t)(sub_addr));   
    if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_ADDRESS;
    }

    /* send Repeat Start signal */
    iic_start();
	/* send salver address again, R/W=1 , read mode */ 
	iic_write_byte(device_addr|0x01);
	if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_CTRLBYTE;
    }
    
    if(byte_count>1)
    {
    	for(i=0; i<byte_count-1; i++)     //读byte_Count-1个数据，最后一个数据时NO ACK，不能这样读
	    {
	        data[i] = iic_read_byte();
	        iic_send_ack();
	    }	
    }
    data[byte_count-1] = iic_read_byte(); //读的最后一个数据，不判断ACK，直接结束
	iic_send_notack();
    
    iic_stop();
  
    return ERR_OK;
}

unsigned char _i2c_write(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int ByteNo)
{
    iic_write_mul_byte(device_addr, sub_addr,ByteNo, buff);
    return 0;
}

unsigned char _i2c_read(unsigned char device_addr, unsigned char sub_addr, unsigned char *buff, int ByteNo)
{
    iic_read_mul_byte(device_addr, sub_addr,ByteNo, buff);
    return 0;
}



/*void I2CInit_encrypt(void)
{
  iic_init(SELECT_ENCRYPT);   //GPIO To IIC, No Init
}*/


/*一次加密认证时间*/
unsigned char EncryptProcess(void)
{
    unsigned char i;
    unsigned char error_code;
    unsigned char dx_data[8]; //加密正确的话应和tx_data相等
    unsigned char tx_data[8]; //随机数或您系统的数据
    
    iic_init(SELECT_ENCRYPT);
      
    for (i=0; i<8; i++)
    {
       tx_data[i] = _alpu_rand();
    } 
  
    error_code = alpuc_process(tx_data,dx_data);

		return error_code;
}


