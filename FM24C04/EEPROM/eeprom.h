#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "gpio_iic.h"




#define SLAVE_ID 		0xA0
#define WRITE_OPERATION 0x00
#define READ_OPERATION 	0x01

//Only have 2 pages, each page 256 bytes
#define SELECT_PAGE0 	0x00 //PAGE 0
#define SELECT_PAGE1 	0x02 //PAGE 1

				 	
//1.��ΪFM24C04ֻ��2ҳ��ÿҳ��256�ֽڣ��������ݳ��ȸ�Ϊ1�ֽڱ�ʾ��У�����2�ֽڱ�ʾ����־λ��1�ֽڱ�ʾ
//2.������Ϊ���ݶ����ԭ�����Ի��Ƕ�Ϊ2���ֽڣ��������ݳ��ȴ���256�ֽ�Ҳ����ʹ��
//3.У���ʹ��4�ֽڣ�������չҲ����
#define FLAG_BYTES				2
#define DATALENGTH_BYTES		2
#define CHECKSUM_BYTES			4

#define MAX_DATA_LENGTH 		(0x100-FLAG_BYTES-DATALENGTH_BYTES-CHECKSUM_BYTES)



#define FLAG_ADDRESS			(uint16_t)0x0000
#define DATALENGTH_ADDRESS		(uint16_t)(FLAG_ADDRESS+FLAG_BYTES)
#define CHECKSUM_ADDRESS		(uint16_t)(DATALENGTH_ADDRESS+DATALENGTH_BYTES+2)//???
#define DATA_START_ADDRESS		(uint16_t)(CHECKSUM_ADDRESS+CHECKSUM_BYTES)



#define READ_DEM_MEMORY_CHECK 	1

//!<==========================================================================================
//!<	API to DEM...Function Declaration
//!<==========================================================================================
extern uint8_t write_dem_memory(uint8_t data[],uint16_t length);
extern uint8_t read_dem_memory(uint8_t data[],uint16_t *p_length);












































#endif/*__EEPROM_H_*/