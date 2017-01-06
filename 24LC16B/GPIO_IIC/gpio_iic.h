#ifndef __GPIO_IIC_H_
#define __GPIO_IIC_H_

#include "MPC5634M_MLQB80.h"
#include "cfg_gpio.h"


#define SELECT_EEPROM 	1
#define SELECT_ENCRYPT 	2


/*****************************************************************************************
***	GPIO PIN select for IIC 	EEPROM FM24C04
*****************************************************************************************/
//#define IIC_SDA_NUM_EEPROM 		221
//#define IIC_SCL_NUM_EEPROM 		220
//#define IIC_WP_NUM_EEPROM 		224
//#define PWR_CTL_NUM_EEPROM 		179                        

//TCU V1.6
#define IIC_SDA_NUM_EEPROM 		92
#define IIC_SCL_NUM_EEPROM 		91
#define IIC_WP_NUM_EEPROM 		202
//#define PWR_CTL_NUM_EEPROM 		179                        

/*****************************************************************************************
***	GPIO PIN select for IIC  	Encrypt ALPU-C
*****************************************************************************************/
#define IIC_SDA_NUM_ENCRYPT 	90	
#define IIC_SCL_NUM_ENCRYPT 	89
//#define IIC_WP_NUM_ENCRYPT	224// No Write Protection
//#define PWR_CTL_NUM_ENCRYPT 	179



#define SDA_OUT(index) 			SIU.PCR[index].R =(FS_GPIO_IO_FUNCTION+FS_GPIO_OUTPUT_MODE+FS_GPIO_OPEN_DRAIN_ENABLE)
#define SCL_OUT(index) 			SIU.PCR[index].R =(FS_GPIO_IO_FUNCTION+FS_GPIO_OUTPUT_MODE)
#define WP_OUT(index)				SIU.PCR[index].R =(FS_GPIO_IO_FUNCTION+FS_GPIO_OUTPUT_MODE+FS_GPIO_WEAK_PULL_DISABLE)
//#define PWR_CTL_OUT(index) 	SIU.PCR[index].R =DO_PAD_CONFIG

#define SDA_IN(index) 			SIU.PCR[index].R =(FS_GPIO_IO_FUNCTION+FS_GPIO_INPUT_MODE)
#define SCL_IN(index) 			SIU.PCR[index].R =(FS_GPIO_IO_FUNCTION+FS_GPIO_INPUT_MODE)

#define GET_VALUE(index) 		GET_STATUS(index)
#define SET_VALUE(index) 		SET_STATUS(index)//HIGH
#define RESET_VALUE(index) 		RESET_STATUS(index)//LOW




#define ERR_OK				 		0
#define ERR_WRITE_CTRLBYTE 			1
#define ERR_WRITE_ADDRESS 			2
#define ERR_WRITE_DATA	 			3
#define ERR_DATALENGTH_OVERSIZE 	4
#define ERR_MISMATCH				5
#define ERR_READ_FLAG				6
#define ERR_WRONG_FLAG				7
#define ERR_READ_DATALEN			8
#define ERR_READ_DATA				9
#define ERR_READ_CHKSUM				10
#define ERR_CHKSUM_WRONG			11
#define ERR_PAGE_WRITE			12
#define ERR_BLOCK_WRITE			13
#define ERR_BULK_WRITE			14

#define ERR_PAGENO_OUTOFRANGE		17
#define ERR_PAGEADDR_OUTOFRANGE		18
#define ERR_BLOCKNO_OUTOFRANGE		19
#define ERR_BLOCKADDR_OUTOFRANGE	20
#define ERR_EEPADDR_OUTOFRANGE		21
#define ERR_DEM_SAVE				22
#define ERR_DEFAULT					254




extern void iic_init(uint8_t select);
extern void iic_start(void);
extern void iic_stop(void);
extern void iic_write_byte(uint8_t data);
extern uint8_t iic_read_byte(void);
extern void iic_send_ack(void);
extern void iic_send_notack(void);
extern uint8_t iic_get_ack(void);























#endif/*__GPIO_IIC_H_*/