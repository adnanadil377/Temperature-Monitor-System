#include <lpc214x.h> 
#include <stdio.h>

#define REAL_AD0GDR (*((volatile unsigned int *)0xE0034004))

#define RS (1<<20)
#define EN (1<<21)
#define AC_RELAY (1<<22)

#define BUZZER (1<<24)   // P1.24

void delay(unsigned int count) {
    int j,i;
    for(i=0;i<count;i++)
        for(j=0;j<100;j++);
}

void lcd_cmd(unsigned char cmd) {
    IO0CLR = RS;
    IO0SET = (cmd & 0xF0) << 12;
    IO0SET = EN; delay(1); IO0CLR = EN;

    IO0CLR = 0x000F0000;
    IO0SET = (cmd & 0x0F) << 16;
    IO0SET = EN; delay(1); IO0CLR = EN;

    IO0CLR = 0x000F0000;
}

void lcd_data(unsigned char data) {
    IO0SET = RS;
    IO0SET = (data & 0xF0) << 12;
    IO0SET = EN; delay(1); IO0CLR = EN;

    IO0CLR = 0x000F0000;
    IO0SET = (data & 0x0F) << 16;
    IO0SET = EN; delay(1); IO0CLR = EN;

    IO0CLR = 0x000F0000;
}

void lcd_print(char *str) {
    while(*str)
        lcd_data(*str++);
}

void init_lcd() {

    PINSEL0 = 0x00000000;

    IO0DIR |= 0x007F0000;   // LCD + Relay
    IO1DIR |= BUZZER;       // Buzzer output

    lcd_cmd(0x02);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x01);
    delay(5);
}

int read_adc() {

    unsigned int val;

    AD0CR &= 0xF8FFFFFF;
    AD0CR |= (1<<24);

    while((REAL_AD0GDR & 0x80000000) == 0);

    val = (REAL_AD0GDR >> 6) & 0x3FF;

    return val;
}

int main() {

    int temp_int;
    char buffer[16];

    PINSEL1 = 0x01000000;
    AD0CR = 0x00200402;

    init_lcd();

    while(1) {

        int adc_val = read_adc();

        temp_int = (adc_val * 330) / 1024;

        lcd_cmd(0x80);
        sprintf(buffer,"Temp:%d C ",temp_int);
        lcd_print(buffer);

        lcd_cmd(0xC0);

        if(temp_int >= 50)
        {
            IO0SET = AC_RELAY;    // AC ON
            IO1SET = BUZZER;      // Buzzer ON

            lcd_print("OVERHEAT !");
        }
        else
        {
            IO0CLR = AC_RELAY;    // AC OFF
            IO1CLR = BUZZER;      // Buzzer OFF

            lcd_print("Temp Normal");
        }

        delay(100);
    }
}