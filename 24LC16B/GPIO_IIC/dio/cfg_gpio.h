#ifndef _CFG_GPIO_H_
#define _CFG_GPIO_H_

///////////////////////////////////////////////////////////////////////////
//2010-10-02, lzy, Create according to EM7 V3.3



/*******SIU Config********/
/* Fields used for all ports  */
/* -------------------------  */
/* PA field - port assignment */
#define FS_GPIO_IO_FUNCTION          0x0000
#define FS_GPIO_PRIMARY_FUNCTION     0x0C00
#define FS_GPIO_ALTERNATE_FUNCTION   0x0800

/* OBE, IBE fields - output/input buffer enable */
#define FS_GPIO_OUTPUT_BUFFER_ENABLE 0x0200
#define FS_GPIO_INPUT_BUFFER_ENABLE  0x0100
#define FS_GPIO_OUTPUT_MODE          0x0200
#define FS_GPIO_INPUT_MODE           0x0100
#define FS_GPIO_READBACK_ENABLE      0x0100
#define FS_GPIO_READBACK_DISABLE     0x0000


/* Fields used for Input       */
/* -------------------------   */
/* HYS field - hysteresis      */
#define FS_GPIO_HYSTERESIS_ENABLE  	0x0010
#define FS_GPIO_HYSTERESIS_DISABLE 	0x0000

/* WPE and WPS fields - treated as 1 field for weak pull configuration */
#define FS_GPIO_WEAK_PULL_UP      	0x0003
#define FS_GPIO_WEAK_PULL_DOWN    	0x0002
#define FS_GPIO_WEAK_PULL_DISABLE 	0x0000


/* Fields used for Output      */
/* -------------------------   */
/* DSC field - drive strength control */
#define FS_GPIO_DRIVE_STRENGTH_10PF 	0x0000
#define FS_GPIO_DRIVE_STRENGTH_20PF 	0x0040
#define FS_GPIO_DRIVE_STRENGTH_30PF 	0x0080
#define FS_GPIO_DRIVE_STRENGTH_50PF 	0x00C0

/* ODE field - output drain control */
#define FS_GPIO_OUTPUT_DRAIN_ENABLE  	0x0020
#define FS_GPIO_OUTPUT_DRAIN_DISABLE 	0x0000
#define FS_GPIO_OPEN_DRAIN_ENABLE  		0x0020
#define FS_GPIO_OPEN_DRAIN_DISABLE 		0x0000

/* SRC field - slew rate control */
#define FS_GPIO_MINIMUM_SLEW_RATE 		0x0000
#define FS_GPIO_MEDIUM_SLEW_RATE  		0x0004
#define FS_GPIO_MAXIMUM_SLEW_RATE 		0x000C

/* MA0 - MA3 function */
#define ADC_MUX_OUTPUT_PAD_CONFIG   FS_GPIO_ALTERNATE_FUNCTION


/* DIO pad set */
#define DO_PAD_CONFIG   FS_GPIO_IO_FUNCTION+\
                        FS_GPIO_OUTPUT_MODE+FS_GPIO_WEAK_PULL_UP
#define DI_PAD_CONFIG   FS_GPIO_IO_FUNCTION+\
                        FS_GPIO_INPUT_MODE+FS_GPIO_WEAK_PULL_UP

#define DO_PAD_EEP_CONFIG   FS_GPIO_IO_FUNCTION+\
                        FS_GPIO_OUTPUT_MODE
#define DI_PAD_EEP_CONFIG   FS_GPIO_IO_FUNCTION+\
                        FS_GPIO_INPUT_MODE


#define CONFIG_GPIO(index, cfg)          (SIU.PCR[index].R =(uint16_t)cfg)

/* DIO set and get value */
#define SET_STATUS(index)                (SIU.GPDO[index].R=1)
#define RESET_STATUS(index)              (SIU.GPDO[index].R=0)
#define GET_STATUS(index)                (SIU.GPDI[index].R)




/*******SIU Config********/




#endif


