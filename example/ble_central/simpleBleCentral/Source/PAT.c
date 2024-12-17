
#include "PAT.h"
#include "sf_i2c.h"
#include "lcd.h"


#define soft_I2C 1

MOVE_STATE Rotate={0};
extern i2c_drv_struct_t i2c_drv ;
static int32_t X_Place_L;
//_____________________PTA________________
extern uint16_t cnt_sleep;
static void Motion_event_handler(gpio_pin_e pin, IO_Wakeup_Pol_e type)
{
    static uint32_t last_interrupt_time = 0;
		if((type == POL_FALLING))
		{
			cnt_sleep=0;
			common_sleep_exit();
		}
}


void delay_us(uint32_t us) {
    // 对于一个16MHz的系统，一次循环大约需要1/16微秒
    // 所以我们需要乘以16来获取正确的延时
    us <<= 2; // us *= 16;
    
    // 等待指定的微秒数
    while(us--) ;
}

void delay_ms(uint32_t ms) {
    // 将毫秒数转换为微秒数
    uint32_t us = ms * 1000;
    
    // 调用微秒延时函数
    delay_us(us);
}
/**************************************************/
// Function Name     :Rotate_Sensor_Into_Lowpower
// Formal Parameter  :NULL
// Return            :NULL
/**************************************************/
void OTS_Sensor_ReadMotion(short *dx, short *dy)
{
    short deltaX_l = 0, deltaY_l = 0, deltaXY_h = 0;
    short deltaX_h = 0, deltaY_h = 0;
		uint8_t motion_status[1]={0};
		PAT_I2C_Read(0x02,motion_status, 1);
    if (motion_status[0] & 0x80) // ???????7?
    {
				uint8_t temp[1]={0};
        PAT_I2C_Read(0x03,temp, 1);
				deltaX_l=(temp[0]);
        PAT_I2C_Read(0x04,temp, 1);
				deltaY_l=(temp[0]);
        PAT_I2C_Read(0x12,temp, 1);
				deltaXY_h=(temp[0]);
				
        deltaX_h = (deltaXY_h << 4) & 0xF00;
        if (deltaX_h & 0x800)
            deltaX_h |= 0xf000;

        deltaY_h = (deltaXY_h << 8) & 0xF00;
        if (deltaY_h & 0x800)
            deltaY_h |= 0xf000;
    }
		
		
    // inverse X and/or Y if necessary
    //		*dx = -(deltaX_h | deltaX_l);

    *dx = deltaX_h | deltaX_l;
    //		*dy = -(deltaY_h | deltaY_l);
    *dy = deltaY_h | deltaY_l;
    Rotate.X_Place += *dx; // 
    Rotate.Y_Place += *dy; // 

    if (Rotate.X_Place >= ONE_ROUND_X_NUM)
    {
        Rotate.X_Place -= ONE_ROUND_X_NUM;
    }
    else if (Rotate.X_Place <= -(ONE_ROUND_X_NUM))
    {
        Rotate.X_Place += ONE_ROUND_X_NUM;
    }

}

uint8_t OTS_Sensor_Init(void)
{
    unsigned char sensor_pid = 0;
    uint8_t read_id_ok = 0;
    PAT_I2C_Read(0x00,&sensor_pid,1);
		dbg_printf("sensor_pid=%d...\n",sensor_pid);
    if (sensor_pid == 0x31)
    {
        read_id_ok = 1;
        Rotate.IsNormal = 1;

        PAT_I2C_Write_Single(0x7F, 0x00); 
        PAT_I2C_Write_Single(0x06, 0x97);

        delay_ms(100); 

        PAT_I2C_Write_Single(0x06, 0x17); 

        PTA_RegWriteRead(0x09, 0x5A); 

        PTA_RegWriteRead(0x0D, 0xFF); 
        PTA_RegWriteRead(0x0E, 0xFF); 
        PTA_RegWriteRead(0x19, 0x04); 


        PTA_RegWriteRead(0x09, 0x00); 
				angle_get();
				dbg_printf("pat init ok\n");
    }
    else
    {
        Rotate.IsNormal = 0;
    }
    return read_id_ok;
}

void Rotate_Sensor_Into_Lowpower(void)
{
		i2c_drv_struct_t *dev=&i2c_drv;
    unsigned char sensor_pid[1] = {0};
    PAT_I2C_Read(0x00,sensor_pid,1);
		dbg_printf("sleep=%d\n",sensor_pid[0]);
    if (sensor_pid[0] == 0x31)
    {
        PTA_RegWriteRead(0x09, 0x5A);
        PTA_RegWriteRead(0x05, 0xB8); 
        PTA_RegWriteRead(0x09, 0x00);
    }
		stop_output(dev);
}

void Rotate_Sensor_Out_Lowpower(void)
{
		i2c_init();
    unsigned char sensor_pid[1] = {0};
    PAT_I2C_Read(0x00,sensor_pid,1);
		
    if (sensor_pid[0] == 0x31)
    {
        PTA_RegWriteRead(0x09, 0x5A);
        PTA_RegWriteRead(0x05,0x10);   
        PTA_RegWriteRead(0x09, 0x00);
    }
}


//_____________________PTA________________


void pat_init(void)
{
	// input motion IO init
  hal_gpio_pin_init(Motion_PIN, GPIO_INPUT); 
  hal_gpio_pull_set(Motion_PIN, STRONG_PULL_UP); 
	hal_gpioin_register(Motion_PIN, Motion_event_handler, Motion_event_handler);
	i2c_init();
	while(OTS_Sensor_Init()==0);
}


short angle_get(void)
{
	uint32_t status=0;
	short angle=0xFFF;
	

	status=hal_gpio_read(Motion_PIN); 
	//dbg_printf("status=%d\n",status);
	if (!Rotate.IsNormal)
        return angle;
	if(!status)
	{
		OTS_Sensor_ReadMotion(&Rotate.Move_X,&Rotate.Move_Y);
		
		Rotate.Move_X_500ms += Rotate.Move_X; 
		if ((Rotate.Move_X >= 1 || Rotate.Move_X <= -1) && (Rotate.X_Place != X_Place_L)) 
		{
				if (Rotate.X_Place > 0)
						angle = Rotate.X_Place * 360 / ONE_ROUND_X_NUM; 
				else
						angle = (ONE_ROUND_X_NUM + Rotate.X_Place) * 360 / ONE_ROUND_X_NUM; 
			
				X_Place_L = Rotate.X_Place;
		}
	}
	return angle;
}


int32_t PAT_I2C_Read( uint8_t Reg, uint8_t *pBuff, uint16_t nBuffSize)
{

	i2c_read_multi_byte(&i2c_drv, SLAVE_OWN_ADDRESS, Reg, pBuff, nBuffSize);
  return 0;
}


int32_t PAT_I2C_Write(uint8_t Reg, uint8_t *pBuff, uint16_t nBuffSize)
{
	

	i2c_write_multi_byte(&i2c_drv, SLAVE_OWN_ADDRESS, Reg, pBuff, nBuffSize);

  return 0;
}

int32_t PAT_I2C_Write_Single(uint8_t Reg, uint8_t data)
{
	uint8_t temp[1]={data};
	
	i2c_write_multi_byte(&i2c_drv, SLAVE_OWN_ADDRESS, Reg, temp, 1);
	
  return 0;
}

void PTA_RegWriteRead(uint8_t address, uint8_t wdata)
{
    uint8_t rdata;
    do
    {
        PAT_I2C_Write(address, &wdata,1); 
        PAT_I2C_Read(address,&rdata,1);		
			dbg_printf("rdata=%d!\n",rdata );
    } while (rdata != wdata);         
    return;
}