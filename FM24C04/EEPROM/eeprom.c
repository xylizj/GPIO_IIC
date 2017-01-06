#include "eeprom.h"


//!<==========================================================================================
//!<	two pages, 256 locations each page, 8 bits each location	FM24C04
//!<==========================================================================================
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

/*********************************************************************************
***	根据情况决定起始地址的位数，如果是16位或32位，则需要拆开分别发送 ***
*********************************************************************************/
static 
uint8_t iic_write_eeprom(uint8_t page_select,uint16_t start_addr, uint16_t byte_count, uint8_t data[])
{
    uint16_t i;
        
    iic_start();

    iic_write_byte(SLAVE_ID|page_select|WRITE_OPERATION);
    if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_SLAVEDEVICE;
    } 
    iic_write_byte((uint8_t)(start_addr&0x00FF));   //写数据的起始地址,先发低字节
    if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_ADDRESS;
    }
    /*iic_write_byte((uint8_t)(start_addr>>8));   
    if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_ADDRESS;
    }*/

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

//current address read, sequential read
static 
uint8_t iic_sequent_read_eeprom(uint8_t page_select,uint16_t byte_count, uint8_t data[])
{
    uint16_t i;

    iic_start();

	iic_write_byte(SLAVE_ID|page_select|READ_OPERATION);
	if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_SLAVEDEVICE;
    }
    
    if(byte_count>1)
    {
    	for(i=0; i<(byte_count-1); i++)     //读byte_Count-1个数据，最后一个数据时NO ACK，不能这样读
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

/*********************************************************************************
***	根据情况决定起始地址的位数，如果是16位或32位，则需要拆开分别发送 ***
*********************************************************************************/
static 
uint8_t iic_selective_read_eeprom(uint8_t page_select,uint16_t start_addr, uint16_t byte_count, uint8_t data[])
{
    uint8_t rc;
    uint16_t i;
    //!<===============================================================================
    //!<	perform a write operation to set the internal address 
		iic_start();
    iic_write_byte(SLAVE_ID|page_select|WRITE_OPERATION);//第一次写设备地址，写操作
    if(iic_get_ack())       
    {
        iic_stop();
        return ERR_WRITE_SLAVEDEVICE;
    }  
    iic_write_byte((uint8_t)(start_addr&0x00FF));   //写数据的起始地址,先发低字节
    if(iic_get_ack())
    {
        iic_stop();
        return ERR_WRITE_ADDRESS;
    }    
   //!<===============================================================================
    rc = iic_sequent_read_eeprom(page_select,byte_count, data);//followed by a sequent read to do a selective read operation    
   	if(ERR_OK!=rc)
		{
			return rc;
		}

    return ERR_OK;
}

static 
uint16_t count_checksum(uint16_t index, uint16_t length, uint8_t data_ptr[])
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
uint8_t buff_compare(uint8_t src_buff[], uint8_t dest_buff[], uint16_t bytes)
{
	uint16_t i;

	for(i=0; i<bytes; i++)
	{
		if(src_buff[i] != dest_buff[i])
			return ERR_MISMATCH;
	}
	
	return ERR_OK;	
}


static 
uint8_t store_to_mem(uint8_t page_select,uint8_t data[],uint16_t length)
{
	uint8_t i;
	uint8_t rc;
	uint32_t l_checksum;
	uint8_t len_buff[DATALENGTH_BYTES];
	uint8_t cs_buff[CHECKSUM_BYTES];
	
	if(MAX_DATA_LENGTH<length)//for FM24C04
	{
		return ERR_DATALENGTH_OVERSIZE;
	}
	
	//!<	write flag before writing
	rc = iic_write_eeprom(page_select,FLAG_ADDRESS,FLAG_BYTES,TO_DO_WRITING);
	if(ERR_OK!=rc)
	{
		return rc;
	}

	rc = iic_write_eeprom(page_select,DATA_START_ADDRESS,length,data);
	if(ERR_OK!=rc)
	{
		return rc;
	}
	//!<	write data length to (a fixed given)datalength address
	for(i=0; i<DATALENGTH_BYTES; i++)
		len_buff[i] = (uint8_t)(length>>(8*(DATALENGTH_BYTES-1-i)));
	rc = iic_write_eeprom(page_select,DATALENGTH_ADDRESS,DATALENGTH_BYTES,len_buff);
	if(ERR_OK!=rc)
	{
		return rc;
	}
	//!<	write checksum
	l_checksum = count_checksum(0, length, data);
	for(i=0; i<CHECKSUM_BYTES; i++)
		cs_buff[i] = (uint8_t)(l_checksum>>(8*(CHECKSUM_BYTES-1-i)));
	rc = iic_write_eeprom(page_select,CHECKSUM_ADDRESS,CHECKSUM_BYTES,cs_buff);
	if(ERR_OK!=rc)
	{
		return rc;
	}
	//!<	write flag after writing finished
	rc = iic_write_eeprom(page_select,FLAG_ADDRESS,FLAG_BYTES,WRITING_FINISHED);
	if(ERR_OK!=rc)
	{
		return rc;
	}
	
	return rc;	
}






//!<==========================================================================================
//!<	API to DEM
//!<==========================================================================================

uint8_t write_dem_memory(uint8_t data[],uint16_t length)
{
	uint8_t rc;
	
  iic_init(SELECT_EEPROM);
  RESET_VALUE(IIC_WP_NUM_EEPROM);//write protect disable,write enable
		
	rc = store_to_mem(SELECT_PAGE0,data,length);
	if(ERR_OK!=rc)//如果Page0写失败，那么就不要写page1了，以免数据全部丢失
	{
		return ERR_WRITE_PAGE0;
	}
	
	rc = store_to_mem(SELECT_PAGE1,data,length);
	if(ERR_OK!=rc)
	{
		return ERR_WRITE_PAGE1;
	}

	SET_VALUE(IIC_WP_NUM_EEPROM);//write protect
	
	return ERR_OK;
}


//如果PAGE0有效 PAGE1无效 PAGE0内容备份到PAGE1 使用PAGE0内容
//如果PAGE0有效 PAGE1有效 不用备份 使用PAGE0内容
//如果PAGE0无效 PAGE1有效 不用备份 使用PAGE1内容
#if READ_DEM_MEMORY_CHECK > 0
uint8_t read_dem_memory(uint8_t data[],uint16_t *p_length)
{
	uint8_t i;
	uint8_t rc0,rc1;
	uint8_t page0_flag_buff[FLAG_BYTES],page1_flag_buff[FLAG_BYTES];	
	uint8_t page0_flag_match,page1_flag_match;
	uint8_t len_buff[DATALENGTH_BYTES];
	
	iic_init(SELECT_EEPROM);

	page0_flag_match = ERR_DEFAULT;
	page1_flag_match = ERR_DEFAULT;
	rc0 = iic_selective_read_eeprom(SELECT_PAGE0,FLAG_ADDRESS,FLAG_BYTES,page0_flag_buff);
	rc1 = iic_selective_read_eeprom(SELECT_PAGE1,FLAG_ADDRESS,FLAG_BYTES,page1_flag_buff);
	if(ERR_OK!=rc0 && ERR_OK!=rc1)
	{
		return ERR_READ_BOTH_PAGE;	
	}
	if(ERR_OK==rc0)
	{
		page0_flag_match = buff_compare(page0_flag_buff, WRITING_FINISHED, (uint16_t)FLAG_BYTES);	
	}
	if(ERR_OK==rc1)
	{
		page1_flag_match = buff_compare(page1_flag_buff, WRITING_FINISHED, (uint16_t)FLAG_BYTES);	
	}
	
	if(ERR_OK==page0_flag_match)//PAGE0 valid
	{
		//try to use PAGE0
		rc0 = iic_selective_read_eeprom(SELECT_PAGE0,DATALENGTH_ADDRESS,DATALENGTH_BYTES,len_buff);
		if(ERR_OK==rc0)
		{				
			for(i=0; i<DATALENGTH_BYTES; i++)
				((uint8_t*)p_length)[i] = len_buff[i];
			rc0 = iic_selective_read_eeprom(SELECT_PAGE0,DATA_START_ADDRESS,*(uint16_t *)p_length,data);		
			if(ERR_OK==rc0)
			{//Then check if need to make a backup to PAGE1					
				if(ERR_OK!=page1_flag_match)//PAGE1 invalid, need backup
				{
					rc1 = store_to_mem(SELECT_PAGE1,data,*(uint16_t *)p_length);
					if(ERR_OK!=rc1)
					{
						return ERR_WRITE_PAGE1;
					}				
				}
			}
			else
			{
				return ERR_READ_PAGE0_DATA;
			}
		}
		else
		{
			return ERR_READ_PAGE0_LENGTH;	
		}
	}
	else//PAGE0 invalid
	{
		if(ERR_OK!=page1_flag_match)//PAGE0 PAGE1 both invalid
		{
			return ERR_BOTH_PAGE_INVALID;
		}
		else
		{
			//try to use PAGE1
			rc1 = iic_selective_read_eeprom(SELECT_PAGE1,DATALENGTH_ADDRESS,DATALENGTH_BYTES,len_buff);
			if(ERR_OK==rc1)
			{				
				for(i=0; i<DATALENGTH_BYTES; i++)
					((uint8_t*)p_length)[i] = len_buff[i];
				rc1 = iic_selective_read_eeprom(SELECT_PAGE1,DATA_START_ADDRESS,*(uint16_t *)p_length,data);		
				if(ERR_OK!=rc1)
				{
					return ERR_READ_PAGE1_DATA;
				}
			}
			else
			{
				return ERR_READ_PAGE1_LENGTH;	
			}		
		}
	}
	
	return ERR_OK;
}
#else
uint8_t read_dem_memory(uint8_t data[],uint16_t *p_length)
{
	uint8_t i;
	uint8_t rc0,rc1;
	uint8_t page0_flag_buff[FLAG_BYTES],page1_flag_buff[FLAG_BYTES];	
	uint8_t page0_flag_match,page1_flag_match;
	uint8_t len_buff[DATALENGTH_BYTES];

	iic_init(SELECT_EEPROM);
	
	page0_flag_match = ERR_DEFAULT;
	page1_flag_match = ERR_DEFAULT;
	iic_selective_read_eeprom(SELECT_PAGE0,FLAG_ADDRESS,FLAG_BYTES,page0_flag_buff);
	iic_selective_read_eeprom(SELECT_PAGE1,FLAG_ADDRESS,FLAG_BYTES,page1_flag_buff);
	page0_flag_match = buff_compare(page0_flag_buff, WRITING_FINISHED, (uint16_t)FLAG_BYTES);	
	page1_flag_match = buff_compare(page1_flag_buff, WRITING_FINISHED, (uint16_t)FLAG_BYTES);	
	
	if(ERR_OK==page0_flag_match)//PAGE0 valid
	{
		//try to use PAGE0
		iic_selective_read_eeprom(SELECT_PAGE0,DATALENGTH_ADDRESS,DATALENGTH_BYTES,len_buff);
		//*(uint16_t *)p_length = (uint16_t)(len_buff[0]<<8)+(uint16_t)len_buff[1];
		for(i=0; i<DATALENGTH_BYTES; i++)
			((uint8_t*)p_length)[i] = len_buff[i];
		iic_selective_read_eeprom(SELECT_PAGE0,DATA_START_ADDRESS,*(uint16_t *)p_length,data);		
		//Then check if need to make a backup to PAGE1					
		if(ERR_OK!=page1_flag_match)//PAGE1 invalid, need backup
		{
			store_to_mem(SELECT_PAGE1,data,*(uint16_t *)p_length);				
		}	
	}
	else//PAGE0 invalid
	{
		if(ERR_OK!=page1_flag_match)//PAGE0 PAGE1 both invalid
		{
			return ERR_BOTH_PAGE_INVALID;
		}
		else
		{
			//try to use PAGE1
			iic_selective_read_eeprom(SELECT_PAGE1,DATALENGTH_ADDRESS,DATALENGTH_BYTES,len_buff);
			if(ERR_OK==rc1)
			{				
				for(i=0; i<DATALENGTH_BYTES; i++)
					((uint8_t*)p_length)[i] = len_buff[i];
				iic_selective_read_eeprom(SELECT_PAGE1,DATA_START_ADDRESS,*(uint16_t *)p_length,data);		
			}		
		}
	}
	
	return ERR_OK;
}
#endif





