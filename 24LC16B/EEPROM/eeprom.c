#include "eeprom.h"


static
uint8_t TO_DO_WRITING[FLAG_BYTES] = 
{
#if FLAG_BYTES>0
	0x11,
#endif
#if FLAG_BYTES>1
	0x22,
#endif
#if FLAG_BYTES>2
	0x33,
#endif
#if FLAG_BYTES>3
	0x44,
#endif			
};

static
uint8_t WRITING_FINISHED[FLAG_BYTES] = 
{
#if FLAG_BYTES>0
	0xAA,
#endif
#if FLAG_BYTES>1
	0x55,
#endif
#if FLAG_BYTES>2
	0x5A,
#endif
#if FLAG_BYTES>3
	0xA5,
#endif			
};


uint8_t eep_iic_atomic_write(uint8_t byte)
{	
	iic_write_byte(byte);

	if(iic_get_ack())
  {       
      iic_stop();
      return ERR_DEFAULT;
  }
    
  return ERR_OK;	
}

//时间参数参考文档《串行 EEPROM 器件的建议用法01028b_cn》第5页

static 
uint8_t eep_iic_page_write(uint8_t block_no,uint8_t page_no,uint8_t page_start_addr, uint8_t byte_count, uint8_t data[])
{
    uint8_t i;
    uint8_t rc;
    control_byte_t ctrl_byte;
    uint16_t pagewrite_to_cnt;
    
    if(page_no >= PAGE_NO_MAX)
    	return ERR_PAGENO_OUTOFRANGE;
    if((page_start_addr+byte_count) > PAGE_SIZE)
    	return ERR_PAGEADDR_OUTOFRANGE;
    
    iic_start();
	
	ctrl_byte.bits.ctrl_code = CONTROL_CODE;
	ctrl_byte.bits.block_select = block_no;
	ctrl_byte.bits.read_write = WRITE_OPERATION;
	rc = eep_iic_atomic_write(ctrl_byte.byte);
	if(ERR_OK != rc)
		return ERR_WRITE_CTRLBYTE;
	
	rc = eep_iic_atomic_write((page_no<<4) | page_start_addr);
	if(ERR_OK != rc)
		return ERR_WRITE_ADDRESS;
	
    for(i=0; i<byte_count; i++)      
    {
			rc = eep_iic_atomic_write(data[i]);
			if(ERR_OK != rc)
				return ERR_WRITE_DATA;
    }

    iic_stop();
	pagewrite_to_cnt = 1320;//16 bytes write time 6.63 ms max...80Mhz 400~500 instructions, 6600*80/400=1320
	do
	{
		iic_start();	
		iic_write_byte(ctrl_byte.byte);
		pagewrite_to_cnt --;
	}while(iic_get_ack() && pagewrite_to_cnt);	
	if(0 == pagewrite_to_cnt)
    {       
        iic_stop();
        return ERR_DEFAULT;
    }

    iic_stop();
    
    return ERR_OK;
}


//in one block, do not cross block boundary!!
static 
uint8_t eep_iic_block_write(uint8_t block_no,uint8_t block_start_addr, uint16_t byte_count, uint8_t data[])
{
	uint8_t rc;
	uint8_t page_no;
	uint8_t page_start_addr;
	uint8_t page_aligned_left;
	uint8_t page_cnt;
	uint8_t single_cnt;
	uint8_t next_page_cnt;
	
    if(block_no >= BLOCK_NO_MAX)
    	return ERR_BLOCKNO_OUTOFRANGE;
	if((block_start_addr+byte_count) > BLOCK_SIZE)
	{
		return ERR_BLOCKADDR_OUTOFRANGE;
	}
	
	page_no = block_start_addr / PAGE_SIZE;
	page_start_addr = block_start_addr % PAGE_SIZE;
	page_aligned_left = PAGE_SIZE - page_start_addr;
	page_cnt =  (uint8_t)(byte_count / PAGE_SIZE);
	single_cnt = (uint8_t)(byte_count % PAGE_SIZE);

	if (page_start_addr == 0) /* aligned  */
	{
		while (page_cnt--)
		{
			rc = eep_iic_page_write(block_no, page_no, 0, PAGE_SIZE, data);
			if(ERR_OK!=rc)
			{
				return ERR_PAGE_WRITE;
			}
			page_no += 1;
			if(page_no == PAGE_NO_MAX)
			{
				page_no = 0;
				block_no += 1;
			}
			data += PAGE_SIZE;
		}
		rc = eep_iic_page_write(block_no, page_no, 0, single_cnt, data);
		if(ERR_OK!=rc)
		{
			return ERR_PAGE_WRITE;
		}
	}
	else
	{
		if (page_cnt == 0)
	    {
	      if (single_cnt <= page_aligned_left)
	      {
		  		rc = eep_iic_page_write(block_no, page_no, page_start_addr, single_cnt, data);
					if(ERR_OK!=rc)
					{
						return ERR_PAGE_WRITE;
					}
	      }
	      else
	      {
	        next_page_cnt = single_cnt - page_aligned_left;
					rc = eep_iic_page_write(block_no, page_no, page_start_addr, page_aligned_left, data);
					if(ERR_OK!=rc)
					{
						return ERR_PAGE_WRITE;
					}
					page_no += 1;
					if(page_no == PAGE_NO_MAX)
					{
						page_no = 0;
						block_no += 1;
					}
					data += page_aligned_left;
					rc = eep_iic_page_write(block_no, page_no, 0, next_page_cnt, data);
					if(ERR_OK!=rc)
					{
						return ERR_PAGE_WRITE;
					}
	      }
	    }
	    else
	    {
	    	rc = eep_iic_page_write(block_no, page_no, page_start_addr, page_aligned_left, data);
				if(ERR_OK!=rc)
				{
					return ERR_PAGE_WRITE;
				}
	    	page_no += 1;
				if(page_no == PAGE_NO_MAX)
				{
					page_no = 0;
					block_no += 1;
				}
	    	data += page_aligned_left;
	    	
	    	byte_count -= page_aligned_left;	
				page_cnt =  (uint8_t)(byte_count / PAGE_SIZE);
				single_cnt = (uint8_t)(byte_count % PAGE_SIZE);
	    	
	    	while (page_cnt--)
	    	{	    		
	    		rc = eep_iic_page_write(block_no, page_no, 0, PAGE_SIZE, data);
					if(ERR_OK!=rc)
					{
						return ERR_PAGE_WRITE;
					}
	    		page_no += 1;
					if(page_no == PAGE_NO_MAX)
					{
						page_no = 0;
						block_no += 1;
					}
	    		data += PAGE_SIZE;
	    	}
	    	
	    	if(0 != single_cnt)
	    	{
	    		rc = eep_iic_page_write(block_no, page_no, 0, single_cnt, data);	    		
					if(ERR_OK!=rc)
					{
						return ERR_PAGE_WRITE;
					}
	    	}	    	
	    }
	}
	
	return ERR_OK;
}



static 
uint8_t eep_iic_bulk_write(uint16_t eep_start_addr, uint16_t byte_count, uint8_t data[])
{
	uint8_t rc;
	uint8_t block_no;
	uint8_t block_start_addr;
	uint8_t block_aligned_left;
	uint8_t block_cnt;
	uint8_t single_cnt;
	uint8_t next_block_cnt;
	
	if((eep_start_addr+byte_count) > EEP_SIZE)
	{
		return ERR_EEPADDR_OUTOFRANGE;
	}
	
	block_no = (uint8_t)(eep_start_addr/BLOCK_SIZE);
	block_start_addr = (uint8_t)(eep_start_addr%BLOCK_SIZE);
	block_aligned_left = BLOCK_SIZE - block_start_addr;
	block_cnt =  (uint8_t)(byte_count / BLOCK_SIZE);
	single_cnt = (uint8_t)(byte_count % BLOCK_SIZE);
	
	if(block_start_addr == 0)
	{
		while (block_cnt--)
		{
			rc = eep_iic_block_write(block_no, 0, BLOCK_SIZE, data);
			if(ERR_OK!=rc)
			{
				return ERR_BLOCK_WRITE;
			}
			block_no +=  1;
			data += BLOCK_SIZE;
		}
		rc = eep_iic_block_write(block_no, 0, single_cnt, data);		
		if(ERR_OK!=rc)
		{
			return ERR_BLOCK_WRITE;
		}
	}
	else
	{
		if (block_cnt == 0)
		{
			if (single_cnt <= block_aligned_left)
			{
				rc = eep_iic_block_write(block_no, block_start_addr, single_cnt, data);
				if(ERR_OK!=rc)
				{
					return ERR_BLOCK_WRITE;
				}
			}
			else
			{
				next_block_cnt = single_cnt - block_aligned_left;
				rc = eep_iic_block_write(block_no, block_start_addr, block_aligned_left, data);
				if(ERR_OK!=rc)
				{
					return ERR_BLOCK_WRITE;
				}
				block_no += 1;
				data += block_aligned_left;
				rc = eep_iic_block_write(block_no, 0, next_block_cnt, data);
				if(ERR_OK!=rc)
				{
					return ERR_BLOCK_WRITE;
				}
			}
		}
		else
		{
			rc = eep_iic_block_write(block_no, block_start_addr, block_aligned_left, data);
			if(ERR_OK!=rc)
			{
				return ERR_BLOCK_WRITE;
			}
    	block_no += 1;
    	data += block_aligned_left;
	    	
	    byte_count -= block_aligned_left;	
			block_cnt =  (uint8_t)(byte_count / BLOCK_SIZE);
			single_cnt = (uint8_t)(byte_count % BLOCK_SIZE);
	    	
			while (block_cnt--)
			{	    		
				rc = eep_iic_block_write(block_no, 0, BLOCK_SIZE, data);
				if(ERR_OK!=rc)
				{
					return ERR_BLOCK_WRITE;
				}
				block_no += 1;
				data += BLOCK_SIZE;
			}
	    	
    	if(0 != single_cnt)
    	{
    		rc = eep_iic_block_write(block_no, 0, single_cnt, data);
    		if(ERR_OK!=rc)
				{
					return ERR_BLOCK_WRITE;
				}
			}	    				
		}
	}
	
	return ERR_OK;
}


//current address read, sequential read
static 
uint8_t eep_iic_sequent_read(uint8_t block_no,uint16_t byte_count, uint8_t data[])
{
    control_byte_t ctrl_byte;
    uint8_t rc;
    uint16_t i;

    iic_start();

	ctrl_byte.bits.ctrl_code = CONTROL_CODE;
	ctrl_byte.bits.block_select = block_no;
	ctrl_byte.bits.read_write = READ_OPERATION;
	
	rc = eep_iic_atomic_write(ctrl_byte.byte);
	if(ERR_OK != rc)
		return ERR_WRITE_CTRLBYTE;
    
    if(byte_count>1)
    {
    	for(i=0; i<(byte_count-1); i++) 
	    {
	        data[i] = iic_read_byte();
	        iic_send_ack();
	    }	
    }
    data[byte_count-1] = iic_read_byte(); 
	iic_send_notack();
    
    iic_stop();
   
    return ERR_OK;
}



static 
uint8_t eep_iic_random_read(uint16_t eep_start_addr, uint16_t byte_count, uint8_t data[])
{
    uint16_t i;
    uint8_t rc;
    control_byte_t ctrl_byte;
	uint8_t block_no;
	uint8_t block_start_addr;

	block_no = (uint8_t)(eep_start_addr/BLOCK_SIZE);
	block_start_addr = (uint8_t)(eep_start_addr%BLOCK_SIZE);
	
	iic_start();

    //!<perform a write operation to set the internal address 
	ctrl_byte.bits.ctrl_code = CONTROL_CODE;
	ctrl_byte.bits.block_select = block_no;
	ctrl_byte.bits.read_write = WRITE_OPERATION;
	rc = eep_iic_atomic_write(ctrl_byte.byte);
	if(ERR_OK != rc)
		return ERR_WRITE_CTRLBYTE;
	 
	rc = eep_iic_atomic_write(block_start_addr);
	if(ERR_OK != rc)
		return ERR_WRITE_CTRLBYTE;
	
	rc = eep_iic_sequent_read(block_no, byte_count, data);
	if(ERR_OK != rc)
		return ERR_DEFAULT;

	return ERR_OK;
}

static 
uint32_t count_checksum(uint16_t index, uint16_t length, uint8_t data_ptr[])
{
	uint16_t j;
	uint32_t checksum;

	checksum = 0;
	for(j=index; j<(index+length); j++)
	{
		checksum += data_ptr[j]; 
	}

	return checksum;
}


static 
uint8_t compare_buff(uint8_t src_buff[], uint8_t dest_buff[], uint16_t bytes)
{
	uint16_t i;

	for(i=0; i<bytes; i++)
	{
		if(src_buff[i] != dest_buff[i])
			return ERR_MISMATCH;
	}
	
	return ERR_OK;	
}






//!<==========================================================================================
//!<	API to DEM
//!<==========================================================================================
uint8_t write_dem_memory(uint8_t data[],uint16_t length)
{
	uint8_t rc;
	uint16_t eep_start_addr;
	uint8_t len_buff[DATALENGTH_BYTES];
	uint8_t i;
	uint32_t checksum;
	uint8_t cs_buff[CHECKSUM_BYTES];
	
	if(length > MAX_DATA_LENGTH)
	{
		return ERR_DATALENGTH_OVERSIZE;
	}

	WP_ENABLE();//always write protect when init
	WP_OUT(IIC_WP_NUM_EEPROM);
	iic_init(SELECT_EEPROM);	
	WP_DISABLE();
		
	//!<write flag before writing
	eep_start_addr = FLAG_ADDRESS;
	rc = eep_iic_bulk_write(eep_start_addr, FLAG_BYTES,TO_DO_WRITING);
	if(ERR_OK!=rc)
	{
		return ERR_DEM_SAVE;
	}
	//!<write data
	eep_start_addr = DATA_START_ADDRESS;
	rc = eep_iic_bulk_write(eep_start_addr,length,data);
	if(ERR_OK!=rc)
	{
		return ERR_DEM_SAVE;
	}
	//!<write data length to (a fixed given)datalength address
	for(i=0; i<DATALENGTH_BYTES; i++)
		len_buff[i] = (uint8_t)(length>>(8*(DATALENGTH_BYTES-1-i)));
	eep_start_addr = DATALENGTH_ADDRESS;
	rc = eep_iic_bulk_write(eep_start_addr,DATALENGTH_BYTES,len_buff);
	if(ERR_OK!=rc)
	{
		return ERR_DEM_SAVE;
	}
	//!<write checksum
	checksum = count_checksum(0, length, data);
	for(i=0; i<CHECKSUM_BYTES; i++)
		cs_buff[i] = (uint8_t)(checksum>>(8*(CHECKSUM_BYTES-1-i)));
	eep_start_addr = CHECKSUM_ADDRESS;
	rc = eep_iic_bulk_write(eep_start_addr,CHECKSUM_BYTES,cs_buff);
	if(ERR_OK!=rc)
	{
		return ERR_DEM_SAVE;
	}
	//!<write flag after writing finished
	eep_start_addr = FLAG_ADDRESS;
	rc = eep_iic_bulk_write(eep_start_addr, FLAG_BYTES,WRITING_FINISHED);
	if(ERR_OK!=rc)
	{
		return ERR_DEM_SAVE;
	}

	WP_ENABLE();
	
	return ERR_OK;
}


uint8_t read_dem_memory(uint8_t data[],uint16_t *p_length)
{
	uint8_t rc;
	uint8_t flag_buff[FLAG_BYTES];
	uint8_t len_buff[DATALENGTH_BYTES];
	uint8_t chksum_buff[CHECKSUM_BYTES];
	uint32_t checksum;
	
	WP_ENABLE();//always write protect when init
	WP_OUT(IIC_WP_NUM_EEPROM);
	iic_init(SELECT_EEPROM);
	
	rc = eep_iic_random_read(FLAG_ADDRESS,FLAG_BYTES,flag_buff);	
	if(ERR_OK != rc)
	{
		return ERR_READ_FLAG;
	}
	
	rc = compare_buff(flag_buff, WRITING_FINISHED, (uint16_t)FLAG_BYTES);	
	if(ERR_OK != rc)
	{
		return ERR_WRONG_FLAG;
	}
	
	rc = eep_iic_random_read(DATALENGTH_ADDRESS,DATALENGTH_BYTES,len_buff);
	if(ERR_OK != rc)
	{
		return ERR_READ_DATALEN;
	}

	rc = eep_iic_random_read(CHECKSUM_ADDRESS,CHECKSUM_BYTES,chksum_buff);
	if(ERR_OK != rc)
	{
		return ERR_READ_DATALEN;
	}

	*p_length = *(uint16_t *)(&len_buff[0]);		
	rc = eep_iic_random_read(DATA_START_ADDRESS, *p_length,data);		
	if(ERR_OK != rc)
	{
		return ERR_READ_DATA;
	}

	checksum = count_checksum(0, *p_length, data);
	if(checksum != *(uint32_t *)chksum_buff)
	{
		return ERR_CHKSUM_WRONG;
	}

	return ERR_OK;
}