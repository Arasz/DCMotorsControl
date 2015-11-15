#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#include "MotorControl.h"



void GPIOInit()
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	// Configure GPIOF pin 0 and 4 as inputs
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; // Unlock GPIOF
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0x0;
	// Set up as input
	ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_DIR_MODE_IN);
	// Configure pull up
	ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}




int main(void)
{
	volatile uint32_t DutyCycle = 5;

	// Set clock at 40 MHz
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_USE_PLL);
	
	GPIOInit();
	MCInitControlHardware(DutyCycle);

	MotorState motorState = ROT_CW;

	volatile uint32_t lastButtonState = 0x0;
	volatile uint32_t switch2;
	volatile uint32_t switch1;

	bool up = false;

	while(1)
	{
		switch1 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
		switch2 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);

		if(switch2 == 0x0 && lastButtonState!=0x0)
		{
			if(motorState == ROT_CW)
				motorState = ROT_CCW;
			else
				motorState = ROT_CW;
		}

		if(switch1==0)
		{
			if(DutyCycle <= 5)
			{
				DutyCycle = 5;
				up = true;
			}
			else if (DutyCycle >= 100)
			{
				DutyCycle = 80;
				up = false;
			}
			MCPwmDutyCycleSet(MotorA, DutyCycle);
			MCPwmDutyCycleSet(MotorB, 100-DutyCycle);
			if(up == false)
				DutyCycle--;
			else
				DutyCycle++;
		}

		MCChangeMotorState(MotorA, motorState);
		MCChangeMotorState(MotorB, motorState);

		lastButtonState = switch2;

		SysCtlDelay(600000);
	}
}
