#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "gpio_iic.h"

#define WP_DISABLE() RESET_VALUE(IIC_WP_NUM_EEPROM);//write protect disable,write enable
#define	WP_ENABLE()  SET_VALUE(IIC_WP_NUM_EEPROM);//write protect

#define CONTROL_CODE 	0b1010
#define WRITE_OPERATION 0b0
#define READ_OPERATION 	0b1

typedef union _control_byte_
{
	uint8_t byte;
	struct
	{
		uint8_t ctrl_code:4;		
		uint8_t block_select:3;
		uint8_t read_write:1;
	}bits;
}control_byte_t;

//24LC16B have 8 blocks, each block 256 bytes(Organized as 8 blocks of 256 bytes (8 x 256 x 8))
//Page write buffer for up to 16 bytes
//2 ms typical write cycle time for page write
#define	BLOCK_SIZE 		256
#define	BLOCK_NO_MAX	8
#define	EEP_SIZE 		(BLOCK_SIZE*BLOCK_NO_MAX)
#define	PAGE_SIZE 		16
#define	PAGE_NO_MAX		(BLOCK_SIZE/PAGE_SIZE)


#define FLAG_BYTES				2
#define DATALENGTH_BYTES		2
#define CHECKSUM_BYTES			4

#define MAX_DATA_LENGTH 		(EEP_SIZE-FLAG_BYTES-DATALENGTH_BYTES-CHECKSUM_BYTES)



#define FLAG_ADDRESS			(uint16_t)(PAGE_SIZE-CHECKSUM_BYTES-DATALENGTH_BYTES-FLAG_BYTES)
//in order to let DATA_START_ADDRESS aligned(PAGE_SIZE)
#define DATALENGTH_ADDRESS		(uint16_t)(FLAG_ADDRESS+FLAG_BYTES)
#define CHECKSUM_ADDRESS		(uint16_t)(DATALENGTH_ADDRESS+DATALENGTH_BYTES)
#define DATA_START_ADDRESS		(uint16_t)(CHECKSUM_ADDRESS+CHECKSUM_BYTES)



#define READ_DEM_MEMORY_CHECK 	0

//!<==========================================================================================
//!<	API to DEM...Function Declaration
//!<==========================================================================================
extern uint8_t write_dem_memory(uint8_t data[],uint16_t length);
extern uint8_t read_dem_memory(uint8_t data[],uint16_t *p_length);












































#endif/*__EEPROM_H_*/