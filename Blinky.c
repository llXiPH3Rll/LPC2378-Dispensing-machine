// ************************************************************************************************ //
//																									//
//	ESD Project:	Toffee Dispensing Machine														//
//																									//
//	Project Made By:																				//
//					Rishiraj Rajkhowa	- 2019B5A30828G												//
//					Vedant Bang			- 2019AAPS0251G												//
//					Rohit Mundada		- 2019AAPS0343G												//
//																									//
//	Software Used:	Keil uVision4																	//
//																									//
// ************************************************************************************************ //

// ************************************************************************************************ //
//																									//
//											INLCLUDES												//
//																									//
// ************************************************************************************************ //

#include <stdio.h>
#include <LPC23xx.H> /* LPC23xx definitions                */
#include "LCD.h"     /* Graphic LCD function prototypes    */
#include <string.h>

// ************************************************************************************************ //
//																									//
//											DEFINES (MACROS)										//
//																									//
// ************************************************************************************************ //

#define MULTIPLIER 25

// ************************************************************************************************ //
//																									//
//											INITIALIZATIONS											//
//																									//
// ************************************************************************************************ //

// Global Variables
int count = 0;
int db_entry;

char keypad[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'H'},
    {'P', '0', 'S', 'G'}};

char id[14];

char id_db[6][14] = {
    "2019AAPS0343G\0",
    "2019B5A30828G\0",
    "2019AAPS0251G\0",
    "2004AAA3A800G\0",
    "2019A1PS0987G\0",
    "2019B4A70755G\0"};

char names_db[6][13] = {
    "Rohit\0",
    "Rishiraj\0",
    "Vedant\0",
    "Anupama maam\0",
    "John Doe\0",
    "Rishi\0"};

int money_db[6] = {100, 100, 100, 100, 100, 100};

// Function Declarations
void delay(void);
void delay_programmable(int);
void delay2(void);
void set_count(void);
void ir_setup(void);
void gpio_setup(void);
void keypad_setup(void);
char get_key_pressed(void);
int get_keypad_data(void);
void lcd_disp(char);

// ************************************************************************************************ //
//																									//
//											UTIL FUNCTIONS											//
//																									//
// ************************************************************************************************ //

void delay(void)
{
    int x;

    for (x = 0; x < 5500000; x++)
        ;
}

void delay_programmable(int a)
{
    int i, j;

    for (j = 0; j < 1000 * a; j++)
    {
        for (i = 0; i < 100; i++)
            ;
    }
}

void delay2(void)
{
    int i, j;

    for (i = 100; i != 0; i--)
    {
        for (j = 10000; j != 0; j--)
            ;
    }
}

void set_count(void)
{
    if (count < 1)
        count = 1;
    if (count > 5)
        count = 5;
}

// ************************************************************************************************ //
//																									//
//										INTERRUPT HANDLING FOR GPIO									//
//																									//
// ************************************************************************************************ //

__irq void EXT3_IRQHandler(void)
{
    unsigned char out[16];
    int i;

    if (IO2_INT_STAT_R == 0x00000001)
    { // PLUS button
        IO2_INT_CLR = 0x00000001;

        count++;

        set_count();

        lcd_clear();
        set_cursor(0, 0);
        sprintf((char *)&out, "Count: %d", count);
        lcd_print(out);
    }
    else if (IO2_INT_STAT_R == 0x00000002)
    { // MINUS button
        IO2_INT_CLR = 0x00000002;

        count--;

        set_count();

        lcd_clear();
        set_cursor(0, 0);
        sprintf((char *)&out, "Count: %d", count);
        lcd_print(out);
    }
    else if (IO2_INT_STAT_R == 0x00000004)
    { // ENTER button
        IO2_INT_CLR = 0x00000004;

        lcd_clear();
        set_cursor(0, 0);
        lcd_print("You selected");
        set_cursor(0, 1);

        if (count > 1)
            sprintf((char *)&out, "%d toffees", count);

        else
            sprintf((char *)&out, "%d toffee", count);

        lcd_print(out);

        money_db[db_entry] -= count * MULTIPLIER;

        if (money_db[db_entry] < 0)
        {
            lcd_clear();
            set_cursor(0, 0);

            lcd_print("Insufficient");

            set_cursor(0, 1);

            lcd_print("Balance! Bye");

            delay();

            money_db[db_entry] += count * MULTIPLIER;
        }
        else
        {
            for (i = 0; i < count; i++)
            {
                FIO2CLR |= 0x00000010; // Start motor

                delay();

                FIO2SET |= 0x00000010; // Stop motor

                delay();
            }

            lcd_clear();
            // lcd_print("Have a nice day!");

            delay();
        }

        main();
    }
    // else if (IO2_INT_STAT_F == 0x00000008)
    // { // IR Sensor Interrupt
    //     IO2_INT_CLR = 0x00000008;

    //     delay();

    //     if (~(FIO2PIN | 0xFFFFFFF7))
    //     {
    //         lcd_clear();
    //         lcd_print("Toffees Empty");

    //         delay();
    //     }
    // }

    VICVectAddr = 0; // Acknowledge Interrupt
}

// ************************************************************************************************ //
//																									//
//											SETUP FUNCTIONS											//
//																									//
// ************************************************************************************************ //

void ir_setup(void)
{
    // P2.3 (P100)  - INPUT     - IR Sensor

    PINSEL4 = 0x00000000; // Use GPIO as regular GPIO
    PINMODE4 = 0xFFFFFFFF;
    FIO2DIR = 0xFFFFFFF7;      // GPIO input mode
    IO2_INT_EN_F = 0x00000008; // Enable interrupt on falling edge

    VICVectCntl17 = 14;
    EXTMODE |= 0x00000008;                          // Make EXT3 edge sensitive
    EXTPOLAR |= 0x00000008;                         // Rising Edge
    VICVectAddr17 = (unsigned long)EXT3_IRQHandler; // Assign this function to be executed on interrupt
    VICIntEnable = 0x00020000;
}

void gpio_setup(void)
{
    // P2.0 (P107)  - INPUT     - Plus
    // P2.1 (P106)  - INPUT     - Minus
    // P2.2 (P105)  - INPUT     - Enter
    // P2.4 (P99)   - OUTPUT    - Relay

    PINSEL4 = 0x00000000; // Use GPIO as regular GPIO
    PINMODE4 = 0xFFFFFFFF;
    FIO2DIR = 0xFFFFFFF0;      // GPIO input mode
    IO2_INT_EN_R = 0x00000007; // Enable interrupt on rising edge

    VICVectCntl17 = 14;
    EXTMODE |= 0x00000008;                          // Make EXT3 edge sensitive
    EXTPOLAR |= 0x00000008;                         // Rising Edge
    VICVectAddr17 = (unsigned long)EXT3_IRQHandler; // Assign this function to be executed on interrupt
    VICIntEnable = 0x00020000;
    FIO2SET |= 0x00000010;
}

// Keypad Initialization
void keypad_setup(void)
{
    // P3.0 TO P3.3 (P137, P140, P144, P2)  - OUTPUT    - row
    // P3.4 TO P3.7 (P9, P12, P16, P19)     - INPUT     - column

    PINSEL6 = 0X00;
    FIO3DIR = 0X0F;
}

// ************************************************************************************************ //
//																									//
//											KEYPAD													//
//																									//
// ************************************************************************************************ //

// Key Pad Readings
char get_key_pressed(void)
{
    int a;

    FIO3PIN = 0X0;
    while ((FIO3PIN & 0XF0) == 0XF0)
        ;

    FIO3PIN = 0XE;
    a = FIO3PIN & 0XFF;

    switch (a)
    {
    case 0XEE:
        while ((FIO3PIN & 0XFF) == 0XEE)
            ;
        return '1';
    case 0XDE:
        while ((FIO3PIN & 0XFF) == 0XDE)
            ;
        return '2';
    case 0XBE:
        while ((FIO3PIN & 0XFF) == 0XBE)
            ;
        return '3';
    case 0X7E:
        while ((FIO3PIN & 0XFF) == 0X7E)
            ;
        return 'A';
    default:
        break;
    }

    FIO3PIN = 0XD;
    a = FIO3PIN & 0XFF;

    switch (a)
    {
    case 0XED:
        while ((FIO3PIN & 0XFF) == 0XED)
            ;
        return '4';
    case 0XDD:
        while ((FIO3PIN & 0XFF) == 0XDD)
            ;
        return '5';
    case 0XBD:
        while ((FIO3PIN & 0XFF) == 0XBD)
            ;
        return '6';
    case 0X7D:
        while ((FIO3PIN & 0XFF) == 0X7D)
            ;
        return 'B';
    default:
        break;
    }

    FIO3PIN = 0X0B;
    a = FIO3PIN & 0XFF;

    switch (a)
    {
    case 0XEB:
        while ((FIO3PIN & 0XFF) == 0XEB)
            ;
        return '7';
    case 0XDB:
        while ((FIO3PIN & 0XFF) == 0XDB)
            ;
        return '8';
    case 0XBB:
        while ((FIO3PIN & 0XFF) == 0XBB)
            ;
        return '9';
    case 0X7B:
        while ((FIO3PIN & 0XFF) == 0X7B)
            ;
        return 'H';
    default:
        break;
    }

    FIO3PIN = 0X07;
    a = FIO3PIN & 0XFF;

    switch (a)
    {
    case 0XE7:
        while ((FIO3PIN & 0XFF) == 0XE7)
            ;
        return 'P';
    case 0XD7:
        while ((FIO3PIN & 0XFF) == 0XD7)
            ;
        return '0';
    case 0XB7:
        while ((FIO3PIN & 0XFF) == 0XB7)
            ;
        return 'S';
    case 0X77:
        while ((FIO3PIN & 0XFF) == 0X77)
            ;
        return 'G';
    default:
        break;
    }

    return '\0';
}

int get_keypad_data(void)
{
    int i = 0;
    keypad_setup();

    do
    {
        id[i] = get_key_pressed();

        // handling key debounce
        if (id[i] == '\0')
        {
            set_cursor(i, 1);
            continue;
        }

        lcd_disp(id[i]);
        i++;

        if (i == 13)
            break;

    } while (1);

    id[i] = '\0';

    return i;
}

void lcd_disp(char s)
{
    int c = 0, f = 0, g, h;

    g = s & 0xf0;
    h = s & 0x0f;
    c = g << 20;
    f = h << 24;

    delay_programmable(5);

    IOPIN1 = 0x90000000 | c;
    IOPIN1 = c;

    delay_programmable(5);

    IOPIN1 = 0x90000000 | f;
    IOPIN1 = f;

    delay_programmable(5);
}

// ************************************************************************************************ //
//																									//
//											MAIN DRIVER CODE										//
//																									//
// ************************************************************************************************ //

int main(void)
{
    int i, j;
    unsigned char out[16];
    int flag;

    lcd_init();
    gpio_setup();

    while (1)
    {
    start: // LABEL
        lcd_clear();

        delay2();

        lcd_print("Enter ID Number:");

        delay2();

        set_cursor(0, 1);

        i = get_keypad_data();

        if (i == 13)
            break;

        lcd_clear();
    };

    // checking ID from database
    for (j = 0; j < sizeof(id_db) / sizeof(id_db[0]); j++)
    {
        if (!strcmp(id_db[j], id))
        {
            flag = 1;
            db_entry = j;
            break;
        }
        else
        {
            flag = 0;
        }
    }

    if (flag)
    {
        lcd_clear();
        set_cursor(0, 0);

        lcd_print("Hello");

        set_cursor(0, 1);
        sprintf((char *)&out, "%s!", names_db[db_entry]);

        lcd_print(out);
    }
    else
    {
        lcd_clear();
        set_cursor(0, 0);

        lcd_print("   User does    ");

        set_cursor(0, 1);

        lcd_print("   not exist!   ");

        delay();

        goto start;
    }

    delay();

    if (money_db[db_entry] == 0)
    {
        lcd_clear();
        lcd_print("No Balance!");

        delay();

        goto start;
    }
    else
    {
        lcd_clear();
        set_cursor(0, 0);
        lcd_print("Select toffees");

        gpio_setup();

        delay();
    }
}
