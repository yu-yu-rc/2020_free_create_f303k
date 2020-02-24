#include "mbed.h"
#include "stdio.h"
#include <string>
#define OLED_ID 0x3C<<1
#define i2c_wait_time 1

#define SCL     GPIO_PIN_RA10
#define SDA     GPIO_PIN_RB14
#define MIN 16
#define HOUR 13
#define MDAY 21
#define MON 1
#define YEAR 120
//A a(&i2c);
Serial pc(USBTX, USBRX);
Serial uart(D1, D0);

DigitalOut myled(LED3);
DigitalInOut sda(D4);
DigitalInOut scl(D6);
DigitalIn thermos_judge(D13);
DigitalIn clocks_judge(D2);
DigitalIn beats_judge(D8);
AnalogIn senser(A6);
DigitalOut digit3(D10);
DigitalOut digit2(D9);
DigitalOut digit1(D1);
DigitalOut digit4(D0);
DigitalOut bit0(D5);
DigitalOut bit1(D12);
DigitalOut bit2(D7);
DigitalOut bit3(D3);

Timer tim1;
Ticker ticker;
InterruptIn photo(D11);

#define SCL_SET_INPUT   scl.input()
#define SCL_SET_OUTPUT  scl.output()
#define SDA_SET_INPUT   sda.input()
#define SDA_SET_OUTPUT  sda.output()
//OLED oled(&i2c);

    void OLED_test();
    void OLED_clear(int);
    void OLED_init();
    void OLED_bar(char, char, char);
    void OLED_disp(char*, char, char);
    void OLED_disp_multi(char, char, char, char);
    void OLED_line_clear(char, char);
    void Misaki_Ascii(char, char, char );
    void OLED_printf(char moji[], char);
    void i2c_init(void);

    char i2c_wait(void);
    void i2c_start(void);
    void i2c_stop(void);
    void i2c_writebyte(char dat);
    
    void segment_init(void);
    void set_digit(char);
    void set_num(char);
    void zero();
    void one();
    void two();
    void three();
    void four();
    void five();
    void six();
    void seven();
    void eight();
    void nine();
    
    unsigned char i2c_readbyte(char noack);
    
    
    bool clocks = 0, thermos = 0, beats = 0;
    bool beats_flag = 0, thermos_flag = 0, clocks_flag = 0;
    char clocks_min = 0, clocks_hour = 0, cnt = 0;
    bool tim_flag = 0;
    
    //---------------------------------------------------------------------
    // ??EEPROM????
    //  unsigned char i2c_addr?Slave Address
    //  unsigned long addr    ?Address(0-0x1FFFF)
    //  unsigned char* data   ?data
    //  unsigned char bytecnt ?byte Count(1-)
    // 20msec
    void i2c_write(
        unsigned char i2c_addr,  char* data, unsigned short byte
    );
    char data[6] = {0};

void init(void){
    i2c_init();
    bool state = false;
    pc.printf("start init...\r\n");
    OLED_init();
    pc.printf("init is completed. state : %d\r\n", state);
    OLED_clear(0);
    pc.printf("clear is completed\r\n");
        // setup time structure for Thu, 5 May 2011 10:00:00
    uart.baud(19200);

}

void segment_ticker(void){
    static char num = 0;
    static char digit = 1;
    static char i = 1;
//    set_digit(digit);
//    set_num(num);
//    num++;
//    digit++;
    if(clocks_flag){
        switch (i){
            case 1:
                set_num(clocks_min % 10);
                break;
            case 2:
                set_num(clocks_min / 10);
                break;
            case 3:
                set_num(clocks_hour % 10);
                break;
            case 4:
                set_num(clocks_hour / 10);
                break;
        }
        pc.printf("%d, %d\n\r", clocks_min, clocks_hour);
        set_digit(i);
        i++;
        if(i == 5)
            i = 1;
    }
    if(num == 10)
        num = 0;
    if(digit == 5)
        digit = 1;
}

void beats_interrupt(void){
    pc.printf("interrupt is oceerd\r\n");
    if(beats_flag)
        cnt++;       
}
            
    

int main(void){
    init(); 
    //oled用変数　_sは_stringの略
    char moji_0[16] = {0};
    char year_s[16] = {0};
    char month_s[16] = {0};
    char day_s[16] = {0};
    char beats_s[16] = {0};
    char beats_s1[16] = {0};
    char beats_s2[16] = {0};
    char beats_s3[16] = {0};
    char hour_s[16] = {0};
    char min_s[16] = {0};
    char sec_s[16] = {0};
    char tmp_s[16] = {0};
    char thermos_s[16] = {0};
    char rxData;
    int tim_value = 0;
    float thermo_value = 0;
    int ond = 0;
    char tmp;
    bool tim_flag = 0;
    struct tm ta;
    ta.tm_sec = 00;    // 0-59
    ta.tm_min = MIN;    // 0-59
    ta.tm_hour = HOUR;   // 0-23
    ta.tm_mday = MDAY;   // 1-31
    ta.tm_mon = MON;     // 0-11
    ta.tm_year = YEAR;  // year since 1900
    
    time_t seconds = mktime(&ta);
    printf("Time as seconds since January 1, 1970 = %d\n", seconds);
    set_time(seconds);
                seconds = time(NULL);
    segment_init();
    photo.rise(beats_interrupt);
    photo.mode(PullUp);
    //ticker.attach(segment_ticker, 0.008);   
    while(1){

        seconds = time(NULL);
        struct tm *t = localtime(&seconds);
        clocks = clocks_judge;
        beats = beats_judge;
        thermos = thermos_judge;
        if(thermos == 1 && beats == 0 && clocks == 0){
            if(thermos_flag == 0){
                OLED_clear(0);
                beats_flag = 0;
                clocks_flag = 0;
            }
            thermo_value = senser.read();
            ond = (3.36*thermo_value-0.5)*10;
            pc.printf("ondo = %f, %d\r\n ", thermo_value, ond);
            //pc.printf("%d, %d, %d\r\n", thermos, beats, clocks);
            sprintf(thermos_s, "ondo is %-03d", ond);
            OLED_printf(thermos_s, 1);
            thermos_flag = 1;
        }else if(beats == 1 && thermos == 0 && clocks == 0){
            if(beats_flag == 0){
                OLED_clear(0);
                thermos_flag = 0;
                clocks_flag = 0;
                tim1.reset();
                tim1.start();
                cnt = 0;
                tim_value = 0;
                tim_flag = 0;
            }
            if(tim_value >= 60){
                if(tim_flag == 0)
                    tmp = cnt;
                tim_flag = 1;
                sprintf(beats_s, "////////////////");
                OLED_printf(beats_s, 1);
                sprintf(beats_s, "Heart rate:%02d", tmp);
                OLED_printf(beats_s, 2);
                sprintf(beats_s, "////////////////");
                OLED_printf(beats_s, 3);
            }else{
                tim_value = tim1.read();
                pc.printf("%d, %d, %d\r\n", thermos, beats, clocks);
                sprintf(beats_s, "Measuring...");
                OLED_printf(beats_s, 1);
                sprintf(beats_s, "Heart rate:%02d", cnt);
                OLED_printf(beats_s, 2);
                sprintf(beats_s, "Remining:%02d", 60 - tim_value);
                OLED_printf(beats_s, 3);
                beats_flag = 1;
            }
        }else if(clocks == 1 && beats == 0 && thermos == 0){
            if(clocks_flag == 0){
                OLED_clear(0);
                beats_flag = 0;
                thermos_flag = 0;
            }
            clocks_min = t->tm_min;
            clocks_hour = t -> tm_hour;
            sprintf(sec_s, "second is %02d", t->tm_sec);
            OLED_printf(sec_s, 6);
            pc.printf("%d, %d, %d\r\n", thermos, beats, clocks);
            printf("%04d/%02d/%02d %02d:%02d:%02d",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
            sprintf(year_s, "year is %d", t->tm_year + 1900);
            OLED_printf(year_s, 1);
            
            sprintf(month_s, "month is %02d", t->tm_mon + 1);
            OLED_printf(month_s, 2);
            sprintf(day_s, "day is %02d", t->tm_mday);
            OLED_printf(day_s, 3);
            sprintf(hour_s, "hour is %02d", t->tm_hour);
            OLED_printf(hour_s, 4);
            sprintf(min_s, "minit is %02d", t->tm_min);
            OLED_printf(min_s, 5);
            
            clocks_flag = 1;
            
        }else{
            if(beats_flag || clocks_flag || thermos_flag){
                OLED_clear(0);
                beats_flag = 0;
                clocks_flag = 0;
                thermos_flag = 0;
            }
            sprintf(moji_0, "hello world");
            OLED_printf(moji_0, 1);
            myled = 1;

            //uart.putc(rxData);
            //rxData = uart.getc();
            //oled.OLED_test();
        }
    }

}

void set_digit(char digit){
    switch (digit){
        case 0:
            digit1 = 0;
            digit2 = 0;
            digit3 = 0;
            digit4 = 0;
            break;
        case 1:
            digit1 = 0;
            digit2 = 1;
            digit3 = 1;
            digit4 = 1;
            break;
        case 2:
            digit1 = 1;
            digit2 = 0;
            digit3 = 1;
            digit4 = 1;
            break;
        case 3:
            digit1 = 1;
            digit2 = 1;
            digit3 = 0;
            digit4 = 1;
            break;
        case 4:
            digit1 = 1;
            digit2 = 1;
            digit3 = 1;
            digit4 = 0;
            break;
    }
}

void set_num(char num){
        
    switch (num){
        case 0:
            zero();
            break;
        case 1:
            one(); 
            break;
        case 2:
            two();
            break;
        case 3:
            three();
            break;
        case 4:
            four();
            break;
        case 5:
            five();
            break;
        case 6:
            six();
            break;
        case 7:
            seven();
            break;
        case 8:
            eight();
            break;
        case 9:
            nine();
            break;
    }
}

void OLED_test(void){
    char data[6] = {114, 123, 2, 54, 3, 66};
    i2c_write(OLED_ID, data, 6);
}



void OLED_clear(int l){
    char i, j, k;
    for(i = 0; i < 6; i++){
        
        switch(i){
            case 0 : data[i] = 0x00; break;
            case 1 : data[i] = 0xB0; break;
            case 2 : data[i] = 0x00; break;
            case 3 : data[i] = 0x21; break;
            case 4 : data[i] = 0x00; break;
            case 5 : data[i] = 0x7F; break;
        }
    }
    char send[129] = {0x40};
    for(k = 1; k < 129; k++){
        send[k] = l;
    }
    for(i = 0; i < 8; i++){
        data[1] = (0xB0 | i);
        i2c_write(OLED_ID, data, 6);
        for(j = 0; j < 16; j++){
            i2c_write(OLED_ID, send, 129);
        }
    }
}

void OLED_init(){
    char initial[43] = {0x00,0xAE,0x00,0xA8,0x3F,0x00,0xD3,0x00,0x00,0x40,0x00,0xA0,0x00,0xC0,0x00,0xDA,0x12,0x00,0x81,0xFF,0x00,0xA4,0x00,0xA6,0x00,0xD5,0x80,0x00,0x20,0x10,0x00,0x22,0x00,0x07,0x00,0x21,0x00,0x7F,0x00,0x8D,0x14,0x00,0xAF};
    i2c_write(OLED_ID, initial, 43);
        data[0] = 0x00;
    data[1] = 0xB0;
    data[2] = 0x00;
    data[3] = 0x21;
    data[4] = 0x00;
    data[5] = 0x7F;
}

void OLED_bar(char start, char line, char length){
    char smp[129] = {
        0x40, 
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
        0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,
    };
    char i;
    for(i = 0; i < 6; i++){
        switch(i){
            case 0 : data[i] = 0x00; break;
            case 1 : data[i] = 0xB0; break;
            case 2 : data[i] = 0x00; break;
            case 3 : data[i] = 0x21; break;
            case 4 : data[i] = 0x00; break;
            case 5 : data[i] = 0x7F; break;
        }
    }
    if(line > 7){
        line = 0;
    }
    if(length > 128 - start){
        length = length%(128 - start);
    }
    data[1] |= line;
    data[4] |= start;
    i2c_write(OLED_ID, data, 6);
    i2c_write(OLED_ID, smp, length + 1);
}

void OLED_disp(char *send, char l, char m){  //data(9byte containing 0x40), line(0-7), SpaceBit(0-128))
    char i, n = 9;
    for(i = 0; i < 6; i++){
        switch(i){
            case 0 : data[i] = 0x00; break;
            case 1 : data[i] = 0xB0; break;
            case 2 : data[i] = 0x00; break;
            case 3 : data[i] = 0x21; break;
            case 4 : data[i] = 0x00; break;
            case 5 : data[i] = 0x7F; break;
        }
    }
    data[1] |= l;
    data[4] |= m;
    i2c_write(OLED_ID, data, 6);
    if(m > 120){
        n = 9 - (m - 120);
    }
    i2c_write(OLED_ID, send, n);
}

void OLED_disp_multi(char send, char l, char m, char n){  //data(containing 0x40), line(0-7), SpaceBit(0-128))
    data[0] = 0x00;
    data[1] = 0xB0;
    data[2] = 0x00;
    data[3] = 0x21;
    data[4] = 0x00;
    data[5] = 0x7F;
    data[1] |= l;
    data[4] |= m;
    i2c_write(OLED_ID, data, 6);
    if(m > 129 - n){
        n = n - (m - (129 - n));
    }
    i2c_write(OLED_ID, &send, n);
}

void OLED_line_clear(char l, char s){ //line space(8bit))
    char i;
    char clear[9] = {0x40};
    
    if(l > 7){
        l = l%6;
    }
    if(s > 128){
        s = s%128;
    }
    
    OLED_disp(clear, l, s);
    for(i = s / 8 + 1; i < 16; i++){
        OLED_disp(clear, l, i * 8);
    }
}

void Misaki_Ascii(char w, char l, char s){ //num line space
    char i;
    char ascii1_3[256] = { //all askii series use 752bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 
        0x00, 0x00, 0x00, 0xBE, 0x00, 0x00, 0x00, 0x00, //!
        0x00, 0x0A, 0x06, 0x00, 0x0A, 0x06, 0x00, 0x00, //"
        0x00, 0x40, 0xC4, 0x7C, 0xC6, 0x7C, 0x46, 0x04, //#
        0x00, 0x48, 0x54, 0xF4, 0x5E, 0x54, 0x24, 0x00, //$
        0x00, 0x84, 0x4A, 0x24, 0x10, 0x48, 0xA4, 0x42, //%
        0x00, 0x40, 0xAC, 0x92, 0xAA, 0x44, 0xB0, 0x80, //&
        0x00, 0x0A, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, //'
        0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x82, 0x00, //(
        0x00, 0x82, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, //)
        0x00, 0x00, 0x44, 0x28, 0xFE, 0x28, 0x44, 0x00, //*
        0x00, 0x10, 0x10, 0x10, 0xFE, 0x10, 0x10, 0x10, //+
        0x00, 0xA0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, //,
        0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, //-
        0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, //.
        0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, ///
        0x00, 0x7C, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, //0
        0x00, 0x00, 0x00, 0x84, 0xFE, 0x80, 0x00, 0x00, //1
        0x00, 0xC4, 0xA2, 0xA2, 0x92, 0x92, 0x8C, 0x00, //2
        0x00, 0x44, 0x82, 0x92, 0x92, 0x92, 0x6C, 0x00, //3
        0x00, 0x60, 0x50, 0x48, 0x44, 0xFE, 0x40, 0x00, //4
        0x00, 0x5E, 0x8A, 0x8A, 0x8A, 0x8A, 0x72, 0x00, //5
        0x00, 0x7C, 0x92, 0x92, 0x92, 0x92, 0x64, 0x00, //6
        0x00, 0x02, 0x02, 0xC2, 0x32, 0x0A, 0x06, 0x00, //7
        0x00, 0x6C, 0x92, 0x92, 0x92, 0x92, 0x6C, 0x00, //8
        0x00, 0x4C, 0x92, 0x92, 0x92, 0x92, 0x7C, 0x00, //9
        0x00, 0x00, 0x00, 0x6C, 0x6C, 0x00, 0x00, 0x00, //:
        0x00, 0x00, 0x00, 0xAC, 0x6C, 0x00, 0x00, 0x00, //;
        0x00, 0x00, 0x00, 0x10, 0x28, 0x44, 0x82, 0x00, //<
        0x00, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, //=
        0x00, 0x82, 0x44, 0x28, 0x10, 0x00, 0x00, 0x00, //>
        0x00, 0x04, 0x02, 0xA2, 0x12, 0x12, 0x0C, 0x00  //?
    };
    char ascii2_3[256] = {
        0x00, 0x38, 0x44, 0xB2, 0xAA, 0x9A, 0x24, 0x18, //@
        0xC0, 0x30, 0x2C, 0x22, 0x2C, 0x30, 0xC0, 0x00, //A
        0x00, 0xFE, 0x92, 0x92, 0x92, 0x92, 0x6C, 0x00, //B
        0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x00, //C
        0x00, 0xFE, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, //D
        0x00, 0xFE, 0x92, 0x92, 0x92, 0x92, 0x82, 0x00, //E
        0x00, 0xFE, 0x12, 0x12, 0x12, 0x12, 0x02, 0x00, //F
        0x00, 0x38, 0x44, 0x82, 0x92, 0x92, 0x74, 0x00, //G
        0x00, 0xFE, 0x10, 0x10, 0x10, 0x10, 0xFE, 0x00, //H
        0x00, 0x00, 0x00, 0x82, 0xFE, 0x82, 0x00, 0x00, //I
        0x00, 0x40, 0x80, 0x80, 0x80, 0x80, 0x7E, 0x00, //J
        0x00, 0xFE, 0x20, 0x10, 0x28, 0x44, 0x82, 0x00, //K
        0x00, 0xFE, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, //L
        0x00, 0xFE, 0x04, 0x18, 0x60, 0x18, 0x04, 0xFE, //M
        0x00, 0xFE, 0x04, 0x08, 0x10, 0x20, 0xFE, 0x00, //N
        0x00, 0x38, 0x44, 0x82, 0x82, 0x44, 0x38, 0x00, //O
        0x00, 0xFE, 0x12, 0x12, 0x12, 0x12, 0x0C, 0x00, //P
        0x00, 0x38, 0x44, 0x82, 0xA2, 0x44, 0xB8, 0x00, //Q
        0x00, 0xFE, 0x12, 0x12, 0x32, 0x52, 0x8C, 0x00, //R
        0x00, 0x4C, 0x92, 0x92, 0x92, 0x92, 0x64, 0x00, //S
        0x00, 0x02, 0x02, 0x02, 0xFE, 0x02, 0x02, 0x02, //T
        0x00, 0x7E, 0x80, 0x80, 0x80, 0x80, 0x7E, 0x00, //U
        0x00, 0x06, 0x18, 0x60, 0x80, 0x60, 0x18, 0x06, //V
        0x00, 0x3E, 0xC0, 0x30, 0x0C, 0x30, 0xC0, 0x3E, //W
        0x00, 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, //X
        0x00, 0x02, 0x04, 0x08, 0xF0, 0x08, 0x04, 0x02, //Y
        0x00, 0x82, 0xC2, 0xA2, 0x92, 0x8A, 0x86, 0x00, //Z
        0x00, 0x00, 0x00, 0x00, 0xFE, 0x82, 0x82, 0x00, //[
        0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, //reverce/
        0x00, 0x82, 0x82, 0xFE, 0x00, 0x00, 0x00, 0x00, //]
        0x00, 0x00, 0x00, 0x04, 0x02, 0x04, 0x00, 0x00, //^
        0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80  //_
    };
    char ascii3_3[256] = {
        0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, //`
        0x00, 0x00, 0x40, 0xA8, 0xA8, 0xA8, 0xF0, 0x00, //a
        0x00, 0x00, 0xFE, 0x90, 0x88, 0x88, 0x70, 0x00, //b
        0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x50, 0x00, //c
        0x00, 0x00, 0x70, 0x88, 0x88, 0x90, 0xFE, 0x00, //d
        0x00, 0x00, 0x70, 0xA8, 0xA8, 0xA8, 0x30, 0x00, //e
        0x00, 0x00, 0x00, 0x08, 0xFC, 0x0A, 0x02, 0x00, //f
        0x00, 0x00, 0x10, 0xA8, 0xA8, 0xA8, 0x78, 0x00, //g
        0x00, 0x00, 0xFE, 0x10, 0x08, 0x08, 0xF0, 0x00, //h
        0x00, 0x00, 0x00, 0x00, 0xFA, 0x00, 0x00, 0x00, //i
        0x00, 0x00, 0x40, 0x80, 0x80, 0x7A, 0x00, 0x00, //j
        0x00, 0x00, 0xFE, 0x20, 0x50, 0x88, 0x00, 0x00, //k
        0x00, 0x00, 0x00, 0x02, 0xFE, 0x00, 0x00, 0x00, //l
        0x00, 0x00, 0xF8, 0x08, 0xF0, 0x08, 0xF0, 0x00, //m
        0x00, 0x00, 0xF8, 0x10, 0x08, 0x08, 0xF0, 0x00, //n
        0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00, //o
        0x00, 0x00, 0xF8, 0x28, 0x28, 0x28, 0x10, 0x00, //p
        0x00, 0x00, 0x10, 0x28, 0x28, 0x28, 0xF8, 0x00, //q
        0x00, 0xF8, 0x10, 0x08, 0x08, 0x08, 0x10, 0x00, //r
        0x00, 0x00, 0x90, 0xA8, 0xA8, 0xA8, 0x48, 0x00, //s
        0x00, 0x00, 0x08, 0x7C, 0x88, 0x88, 0x40, 0x00, //t
        0x00, 0x00, 0x78, 0x80, 0x80, 0x40, 0xF8, 0x00, //u
        0x00, 0x00, 0x18, 0x60, 0x80, 0x60, 0x18, 0x00, //v
        0x00, 0x00, 0x38, 0xC0, 0x30, 0xC0, 0x38, 0x00, //w
        0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00, //x
        0x00, 0x00, 0x88, 0xB0, 0x40, 0x30, 0x08, 0x00, //y
        0x00, 0x00, 0x88, 0xC8, 0xA8, 0x98, 0x88, 0x00, //z
        0x00, 0x00, 0x00, 0x10, 0x6C, 0x82, 0x82, 0x00, //{
        0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, //|
        0x00, 0x82, 0x82, 0x6C, 0x10, 0x00, 0x00, 0x00, //}
        0x00, 0x00, 0x02, 0x04, 0x02, 0x04, 0x00, 0x00, //~
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 
    };
    char send[9] = {0x40};
    if(w < 0x20)
        w = 0x20;
    if(w > 0x7F)
        w = 0x7F;
    if(w >= 0x20 && w <= 0x3F){
        for(i = 1; i < 9; i++){
            send[i] = ascii1_3[(w - 0x20) * 8 + (i - 1)];
        } 
    }
    if(w >= 0x40 && w <= 0x5F){
        for(i = 1; i < 9; i++){
            send[i] = ascii2_3[(w - 0x40) * 8 + (i - 1)];
        } 
    }
    if(w >= 0x60 && w <= 0x7F){
        for(i = 1; i < 9; i++){
            send[i] = ascii3_3[(w - 0x60) * 8 + (i - 1)];
        } 
    }
    OLED_disp(send, l, s);
}

void OLED_printf(char moji[], char l){
    for(int i = 0; moji[i]; i++){
        Misaki_Ascii(moji[i], l, 8 * i);
    }
    
    return ;
}

//------------------------------------????
void i2c_delay(){//usec
    wait_us(i2c_wait_time);
}

//------------------------------------SCL?High?
void i2c_scl_high(void)
{
    SCL_SET_INPUT;     //input
    i2c_delay();
}

//------------------------------------SCL?Low?
void i2c_scl_low(void)
{
    SCL_SET_OUTPUT;
    scl = 0;      //output low
    i2c_delay();
}

//------------------------------------SDA?High?
void i2c_sda_high(void)
{
    SDA_SET_INPUT;
    i2c_delay();
}

//------------------------------------SDA?Low?
void i2c_sda_low(void)
{
    SDA_SET_OUTPUT;
    sda = 0;      //output low
    i2c_delay();
}

//------------------------------------??(??:????????????)
void i2c_write_bit(unsigned char data,char cnt)
{
    while(cnt > 0)
    {
        if(data & 0x80)
            i2c_sda_high();
        else
            i2c_sda_low();

        i2c_scl_high();
        i2c_scl_low();
        data <<= 1;
        cnt--;
    }
}

//------------------------------------??(??:??????)
unsigned char i2c_read_bit(char cnt)
{
    unsigned char data=0;   
    SDA_SET_INPUT;
        while(cnt > 0)
    {
        data <<=1;
        i2c_scl_high();
        if (sda)
            data |= 1;

        i2c_scl_low();
        cnt--;
    }
    return(data);
}

//---------------------------------------------------------------------
void i2c_init(void)
{
    i2c_sda_high();
    i2c_scl_high();
}


//---------------------------------------------------------------------
void i2c_start(void)
{
    i2c_sda_high();
    i2c_scl_high();
    i2c_sda_low();
    i2c_scl_low();
}


//---------------------------------------------------------------------
void i2c_stop(void)
{
    i2c_sda_low();
    i2c_scl_high();
    i2c_sda_high();
}


//---------------------------------------------------------------------
void i2c_writebyte(char dat)
{
    // 1?????+ACK??
    i2c_write_bit(dat,8);

    // read ack.
     i2c_read_bit(1);
}



//---------------------------------------------------------------------
unsigned char i2c_readbyte(char noack)
{
    unsigned char rxdata;

    rxdata = i2c_read_bit(8);

    if(noack)
        i2c_write_bit(0xff,1);  // NACK
    else
        i2c_write_bit(0x00,1);  // ACK

    return(rxdata);
}



//---------------------------------------------------------------------
//
void i2c_write(unsigned char i2c_addr, char* data, unsigned short byte)
{
    i2c_start();
    i2c_writebyte(i2c_addr);    // SLA + W
    i2c_delay();
    //i2c_writebyte(0x00);
    while(byte > 0){
    i2c_writebyte(*data++);
         byte--;
    }
    i2c_stop();
}

void segment_init(void){
    set_digit(0);
    zero();
}
 
void zero(){
    bit0 = 0;
    bit1 = 0;
    bit2 = 0;
    bit3 = 0;
}
void one(){
    bit0 = 1;
    bit1 = 0;
    bit2 = 0;
    bit3 = 0;
}
void two(){
    bit0 = 0;
    bit1 = 1;
    bit2 = 0;
    bit3 = 0;
}
void three(){
    bit0 = 1;
    bit1 = 1;
    bit2 = 0;
    bit3 = 0;
}
void four(){
    bit0 = 0;
    bit1 = 0;
    bit2 = 1;
    bit3 = 0;
}
void five(){
    bit0 = 1;
    bit1 = 0;
    bit2 = 1;
    bit3 = 0;
}
void six(){
    bit0 = 0;
    bit1 = 1;
    bit2 = 1;
    bit3 = 0;
}
void seven(){
    bit0 = 1;
    bit1 = 1;
    bit2 = 1;
    bit3 = 0;
}
void eight(){
    bit0 = 0;
    bit1 = 0;
    bit2 = 0;
    bit3 = 1;
}
void nine(){
    bit0 = 1;
    bit1 = 0;
    bit2 = 0;
    bit3 = 1;
}
