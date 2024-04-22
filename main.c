#include "__TM4C123GH6PM_H__.h"
void UARTInit(void)
{
    SYSCTL_RCGCUART_R |= 0x01;
    GPIO_PORTA_AFSEL_R |= 0x03;
    GPIO_PORTA_DEN_R |= 0x03;
    GPIO_PORTA_PUR_R |= 0x00;
    GPIO_PORTA_PDR_R |= 0x00;
    GPIO_PORTA_PCTL_R |= 0x011;
    GPIO_PORTA_CR_R |= 1;
    UART0_CTL_R |= 0x00;
    UART0_IBRD_R = 104;
    UART0_FBRD_R = 11;
    UART0_LCRH_R = 0x60;
    UART0_CTL_R |= 0x301;
    UART0_DR_R |= 0x00;
}
void UART_Tx (char data)
{
    while((UART0_FR_R & 0x20) != 0);
    UART0_DR_R = data;
}

char UART_Rx ()
{
    char data;
    if((UART0_FR_R & 0x10));
    data = UART0_DR_R;
    return ((unsigned char) data);
}
void ADCInit(void)
{
    SYSCTL_RCGCADC_R |= 0x01; // Enable ADC0
    SYSCTL_RCGCGPIO_R |= 0x11; // Enable GPIO Port E
    GPIO_PORTE_AFSEL_R |= 0x08; // Enable alternate function on PE3
    GPIO_PORTE_DEN_R &= ~0x08; // Disable digital function on PE3
    GPIO_PORTE_AMSEL_R |= 0x08; // Enable analog function on PE3
    ADC0_ACTSS_R &= ~0x08; // Disable sample sequencer 3
    ADC0_EMUX_R &= ~0xF000; // Set EM3 as a software trigger
    ADC0_SSMUX3_R = 0x00; // Select channel AIN0 (PE3)
    ADC0_SSCTL3_R |= (1<<1)|(1<<2); // Set END0 and IE0
    ADC0_ACTSS_R |= 0x08; // Enable sample sequencer 3
}

void PWMInit(void)
{
    //setting up PWM----------------------------------
    SYSCTL_RCGCPWM_R |= 0x01; // Enable PWM0
    SYSCTL_RCGCGPIO_R |= 0x02; // Enable GPIO Port B
    GPIO_PORTB_DIR_R |= 0x40; // Set PB6 as output
    GPIO_PORTB_DEN_R |= 0x40; // Enable digital function on PB6
    GPIO_PORTB_AFSEL_R |= 0x40; // Enable alternate function on PB6
    GPIO_PORTB_PCTL_R |= 0x04000000; // Configure PB6 as PWM0
    SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // Use PWM divider
    SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; // Clear PWM divider field
    SYSCTL_RCC_R |= SYSCTL_RCC_PWMDIV_2; // Set PWM divider to 64
    PWM0_CTL_R = 0x00; // Disable PWM0
    PWM0_0_GENA_R = 0x0000008C; // Configure PWM0, Generator 0, as a down-counting mode
    PWM0_0_LOAD_R = 16000; // Set PWM0 load value (period = 64000)
    PWM0_0_CMPA_R = 1600; // Set PWM0 compare A value
    PWM0_CTL_R |= 0x01; // Enable PWM0
    PWM0_ENABLE_R |= 0x01; // Enable PWM0, output on PB6
}

void delay(unsigned long i)
{
    while(i != 0)
        i--;
}
int main(void)
{
    ADCInit();
    PWMInit();
    UARTInit();
    unsigned int adc_val;
    while(1)
    {
        ADC0_PSSI_R |= (1<<3);
        while((ADC0_RIS_R & 8) == 0);
        adc_val = ADC0_SSFIFO0_R;
        UART_Tx(adc_val);
        ADC0_ISC_R |= (1<<3); // clear interrupt status
    }
}
