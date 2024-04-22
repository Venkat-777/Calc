#include "__TM4C123GH6PM_H__.h"

int A = 0;//GLOBAL A
int B = 0;// GLOBAL B

//Different states
typedef enum {
    IN,
    ST,
    A_,
    B_,
    DI,
} state;
state currState = IN;
void delay(unsigned long lim) {
    while(lim)
    {
        lim--;
    }
}
void Delay(void)
{
    volatile unsigned long i;
    for(i=0;i<100000;i++);
}
void Delay2(void)
{
    volatile unsigned long i;
    for(i=0;i<25000;i++);
}
//instruction
void lcd_in(int i)
{
    GPIO_PORTF_DATA_R = 0x0;
    Delay();
    GPIO_PORTB_DATA_R = i;
    GPIO_PORTF_DATA_R |= (1 << 3);
    Delay();
    GPIO_PORTF_DATA_R ^= (1 << 3);
    Delay();
}
//data
void lcd_data(unsigned char data)
{
    GPIO_PORTF_DATA_R |= 0x02;
    GPIO_PORTF_DATA_R &= ~0x04;
    GPIO_PORTB_DATA_R = data;
    GPIO_PORTF_DATA_R |= 0x08;
    delay(100000);
    GPIO_PORTF_DATA_R &= ~0x08;
    delay(100000);
}
//init fxn
void lcd_init()
{
    lcd_in(0x38);
    delay(10000);
    lcd_in(0x38);
    delay(10000);
    lcd_in(0x01);
    delay(10000);
    lcd_in(0x06);
    delay(10000);
    lcd_in(0x0F);
    delay(10000);
}
char getKey(void)
{
    const char matx[4][4] = { {'1', '2', '3', 'A'},{'4', '5', '6', 'B'},{'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
    unsigned int row;
    unsigned int col;
    for (row = 0; row < 4; row++)
    {
        GPIO_PORTC_DATA_R = 1 << (row+4);
        for (col = 0; col < 4; col++)
        {
            if ((GPIO_PORTE_DATA_R & (1 << (col + 1))))
            {
                int i;
                char c = matx[row][col];
                while ((GPIO_PORTE_DATA_R & (1 << (col + 1))));
                delay(300000);
                return c;
            }
        }
    }
    return '%';
}
void reset()
{
    A = 0;
    B = 0;
    currState = ST;
    lcd_in(0x01);
}
void  print(int i)
{
    if(i == 0)
    {
        return;
    } else
    {
        char c;
        int dig = i % 10;
         print(i / 10);
        c = '0' + dig;
        lcd_data(c);
    }
}
void displayResult()
{
    int result = A * B;
    lcd_in(0xC0);
     print(result);
    lcd_in(0x80);
}
void clearTopRow()
{
    lcd_in(0x80);
    int i;
    for (i = 0; i < 16; i++)
    {
        lcd_data(' ');
    }
    lcd_in(0x80);
}
int getNumberOfDigits(int number)
{
    if (number == 0) return 1;
    int digits = 0;
    while (number != 0)
    {
        number /= 10;
        digits++;
    }
    return digits;
}
void initial_State()
{
    currState = ST;
    reset();
}

void start_State()
{
    currState = A_;
    clearTopRow();
    A = 0;
    B = 0;
}

void A_State(char key)
{
    if (key == '*')
    {
        currState = B_;
        clearTopRow();
    }
    else if (key >= '0' && key <= '9')
    {
        A = A * 10 + (key - '0');
        lcd_data(key);
        if (getNumberOfDigits(A) == 8)
        {
            currState = B_;
            clearTopRow();
        }
    }
    else if (key == 'C')
    {
        currState = IN;
    }
    else if (key == '#')
    {
        currState = DI;
    }
}
void B_State(char key)
{
    if (key == '#') currState = DI;
    else if (key == 'C') currState = IN;
    else if (key >= '0' && key <= '9')
    {
        B = B * 10 + (key - '0');
        lcd_data(key);
        if (getNumberOfDigits(B) == 8)  currState = DI;
    }
}
void displayState()
{
    currState = ST;
    displayResult();
}
int main(void)
{
    SYSCTL_RCGCGPIO_R = 0x3F;

    GPIO_PORTF_DEN_R = 0xE;
    GPIO_PORTF_DIR_R = 0xE;

    GPIO_PORTB_DATA_R = 0x00;
    GPIO_PORTB_DEN_R = 0xFF;
    GPIO_PORTB_DIR_R = 0xFF;

    GPIO_PORTC_DEN_R = 0xF0;
    GPIO_PORTC_DIR_R = 0xF0;
    GPIO_PORTE_DEN_R = 0x1E;
    GPIO_PORTE_DIR_R = 0x00;
    A = 0;
    B = 0;
    lcd_init();
    Delay();
    while (1)
    {
        char c = getKey();
        switch(currState)
        {
        case IN:
            initial_State();
            break;
        case ST:
            start_State();
            break;
        case A_:
            A_State(c);
            break;
        case B_:
            B_State(c);
            break;
        case DI:
            displayState();
            break;
        }
    }
}
