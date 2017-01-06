#include "MPC5634M_MLQB80.h"
#include "eeprom.h"





void InitPLL1(void)
{
	
	/* Fsys = Fref x (EMFD/[(EPREDIV+1)*2^(ERFD+1)]) */
	/* Fsys = 8MHz x (40/[(0+1)*2^(1+1)]) = 80MHz */
	/* page 587, new method, set to 80MHz */

      FMPLL.ESYNCR1.B.EMODE = 1;			/* Write once bit, enhance model */
    FMPLL.ESYNCR1.B.EPREDIV = 0x01;		/* EPREDIV = 1 Divide by 2*/ 
    //FMPLL.ESYNCR1.B.EPREDIV = 0x04;		/* Divide by 5 modified by xyl */ 
		FMPLL.ESYNCR1.B.EMFD    = 40;       /* EMFD = 40 */
	  FMPLL.ESYNCR2.B.ERFD    = 0x00;     /* ERFD = 0 */
  	  while (FMPLL.SYNSR.B.LOCK != 1) {}; /* Wait for FMPLL to LOCK  */
  
  	  FMPLL.ESYNCR1.B.CLKCFG = 0x07;      /* Change clk to PLL normal mode from crystal */ 
  	  while (FMPLL.SYNSR.B.LOCK != 1) {}; /* Wait for FMPLL to LOCK  */

}

#define DATA_LENGTH 	0x4A0 //Must not be larger than MAX_DATA_LENGTH

uint8_t read_buff[DATA_LENGTH];
uint8_t write_buff[DATA_LENGTH];

uint16_t write_length;
uint16_t read_length;

uint8_t page0_read_buff[DATA_LENGTH];
uint8_t page1_read_buff[DATA_LENGTH];

uint8_t write_dem_result;
uint8_t read_dem_result;
uint8_t encryptResult;

uint8_t write_en;
uint8_t read_en;
uint8_t encry_en;

int main(void) 
{
	uint16_t i;
	uint16_t *ptr = &read_length;
	InitPLL1();
	
	
	for(i=0;i<DATA_LENGTH;i++)
		write_buff[i] = 0x14+i;
	
	write_length = DATA_LENGTH;
	
	
	/* Loop forever */
	for (;;) 
	{
			if(write_en)
			{
				write_en = 0;	
				write_dem_result = write_dem_memory(write_buff, write_length);
			}
			if(read_en)
			{
				read_en = 0;
				read_dem_result = read_dem_memory(read_buff,ptr);
			}
			
			if(encry_en)
			{
				encry_en = 0;
				encryptResult = EncryptProcess();
			}

	}
}



