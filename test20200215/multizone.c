#include "all_includes.h"

#define		MULTI_EN 	SCI3.SCR.BIT.RE
#define		MULTI_BUF_SIZE	20

u8 data;


OS_EVENT       *MultiWait;
int			   multi_rx_rear= 0;
u8			   multi_rx_buffer[MULTI_BUF_SIZE]; 
u32			   multi_exist=0x00000000L;
multi_dev_t 	multi_dev[32];
u8		 	  g_old_multictl[32];


void multizone_init()
{
	multi_rx_rear = 0;
    MULTI_EN = 1;
}


__interrupt(vect=100) void INT_ERI3_SCI3(void)
{
    OSIntEnter();
	Debug[1] = 100;
    SCI3.SSR.BIT.ORER = 0;
    SCI3.SSR.BIT.FER = 0;
    SCI3.SSR.BIT.PER = 0;

    SCI3.RDR = 0xff;
    OSIntExit();
}

void multi_int_clr(void)
{
	SCI3.SSR.BIT.RDRF = 0;
}

void multi_int(void)
{
	multi_rx_buffer[multi_rx_rear] = SCI3.RDR;

	if(multi_rx_buffer[multi_rx_rear] != 0xff)
	{
		if(multi_rx_buffer[multi_rx_rear] == 0xfe)
		{
			multi_rx_buffer[0] = multi_rx_buffer[multi_rx_rear];
			multi_rx_rear = 0;	
		}
		else if(multi_rx_buffer[multi_rx_rear] == 0xfd)
		{
			MULTI_EN = 0;
			multi_rx_rear = 0;	
			OSSemPost(MultiWait);
		}

		multi_rx_rear++;
		if (multi_rx_rear >= MULTI_BUF_SIZE)
			multi_rx_rear = 0;
	}
}

__interrupt(vect=101) void INT_RXI3_SCI3(void)  
{
    OSIntEnter();

	multi_int_clr();
    multi_int();

    OSIntExit();
}


u32	multi_get_exist()
{
	return multi_exist;
}


void multi_put_data(u8 c)
{
	while ((SCI3.SSR.BIT.TDRE) == 0);
		SCI3.TDR = c;

	SCI3.SSR.BIT.TDRE = 0;
}


u8 multi_set_control(u8 addr,u8 control)
{
	u8 		err;
	u32		cur;

//	task_lock(SendLock);

	MULTI_EN = 1;
	multi_put_data(0xfe);
	multi_put_data(addr);
	multi_put_data(control);
	multi_put_data(0xfd);

	OSSemPend(MultiWait, 50, &err);

//	task_unlock(SendLock);
	if(err == 0)
	{
		if(addr == multi_rx_buffer[1])
		{
			MULTI_EN = 1;
			return multi_rx_buffer[2];
		}
	}
	else
	{
	}

	MULTI_EN = 1;
	return 0xff;
}

u32 multi_scan_exist()
{
	u8		ret;
	u8 		i;
	u32 	tmp = 0x00000000L;


	for(i = 0 ; i < 32 ;i++)
	{
		ret = multi_set_control(i,0x00);

		if(ret != 0xff)
		{
			tmp |= (u32)0x00000001L << i;
		}
	}
	return tmp;
}

DECLARE_TASK_FUNC(MultiZone)
{
	u8 		err;
	int 	i;
	u8		ret;
	u8		addr;
    u8 		ErrorOnCnt = 0;
    u8 		ErrorOffCnt = 0;
    u8 		ErrorOnData[8];
    u8 		ErrorOffData[8];
	u8		Oper = false;


    parg = parg; 

	MultiWait  = OSSemCreate(0);                    

	memset(multi_dev,0,sizeof(multi_dev_t) * 32);
	memset(g_old_multictl,0,32);
    multizone_init();


	multi_exist = multi_scan_exist();//0xfa1002ffL;
	for(i = 0 ;i < 32 ;i++)
	{
		if(multi_exist & (0x00000001L << i))
			multi_dev[i].active = true;
	}

	//Debug[0] = 0;
	while(1)
	{
		FLASH_LOOP();

		Oper = false;

		if(pEnv->system.multi_zone_enable[addr] == true || maintence_enter == true)
		{
			Oper = true;
			OSTimeDly(10);//
		}

		if(Oper == true)
		{
			ret = multi_set_control(addr,multi_dev[addr].control);
	
			if(ret != 0xff)
			{
				multi_dev[addr].sensor = ret;
				multi_dev[addr].fail = 0;
				if(multi_dev[addr].active == false && maintence_enter == false)
				{
					ErrorOffData[ErrorOffCnt * 2] = (addr / 10) + 0x30;
					ErrorOffData[ErrorOffCnt * 2+1] = (addr % 10) + 0x30;
					Jur_Add_Device(JUR_MULTI,addr,JUR_DEVICE_OK);				
					msg_send_comm(MULTI_ERROR_SIG,0,0,C_OFF,(BYTE *)ErrorOffData,2);					
				}
				multi_dev[addr].active = true;
			}
			else
			{
				multi_dev[addr].fail++;
				printf("Multi Error  %d\n\r",addr);
			}
	
			if(multi_dev[addr].fail > 10)
			{
				if(multi_dev[addr].active == true && maintence_enter == false)
				{
					ErrorOnData[ErrorOnCnt * 2] = (addr / 10) + 0x30;
					ErrorOnData[ErrorOnCnt * 2+1] = (addr % 10) + 0x30;
					Jur_Add_Device(JUR_MULTI,addr,JUR_DEVICE_ERR);				
					msg_send_comm(MULTI_ERROR_SIG,0,0,C_ON,(BYTE *)ErrorOnData,2);					
				}
	
				multi_dev[addr].active = false;
				multi_dev[addr].fail = 0;
				multi_dev[addr].control = 0;
				multi_dev[addr].sensor = 0;
			}
		}
	
		if(++addr > 0x1f)
		{
			ErrorOffCnt = 0;
			ErrorOnCnt = 0;
			addr = 0;
			OSTimeDly(50);
		}
	}	
}

