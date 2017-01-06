#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "gpio_iic.h"




#define SLAVE_ID 		0xA0
#define WRITE_OPERATION 0x00
#define READ_OPERATION 	0x01

//Only have 2 pages, each page 256 bytes
#define SELECT_PAGE0 	0x00 //PAGE 0
#define SELECT_PAGE1 	0x02 //PAGE 1

				 	
//1.因为FM24C04只有2页，每页有256字节，所以数据长度改为1字节表示，校验和用2字节表示，标志位用1字节表示
//2.但是因为数据对齐的原因所以还是定为2个字节，将来数据长度大于256字节也可以使用
//3.校验和使用4字节，将来扩展也方便
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