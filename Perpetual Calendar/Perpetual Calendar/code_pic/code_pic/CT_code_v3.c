#include <16F887.h>
#fuses HS
#use delay(clock=20M) 
#define RTC_SDA  PIN_C4 
#define RTC_SCL  PIN_C3 
#use i2c(master, I2C1, FAST=100000)
#include "i2c_lcd.c"
#include <ds1307.c> 
#use rs232(baud=9600, xmit=PIN_B6, rcv=PIN_B7, bits=8, parity=N, stop=1)
#define LED1 PIN_E0  
#define BUZZER PIN_A4 
#define tang PIN_C0 // tang 
#define giam PIN_C1 // giam 
#define select PIN_C2 // select
#define md_system PIN_D1 // mode
#define DHT22_PIN PIN_B0   

int1 flag_enable;
int1 toggle=false;
float temperature = 0, humidity = 0;
byte gio,phut,giay; 
byte thu,ngay,thang,nam; 
byte set_gio,set_phut,set_giay; 
byte chinh=0; 
byte ct=0; 
//!char buffer[20];
int1 bct=0; 
int1 np=1;
unsigned int16 TEMP = 0;
unsigned int16 buzzertime = 0;
volatile int16 timer_count = 0;
int8 mode_bt =0;

#INT_TIMER1
VOID timer1_isr()
{
   set_timer1(3036); 
   TEMP++;
   buzzertime++;
   if (buzzertime == 40)   // 1s
   {
      toggle = !toggle;
      buzzertime = 0;
   }
   if (TEMP == 10)   // 1s
   {
      TEMP = 0;
   }
}
//========================================
#INT_TIMER0
void timer0_isr() {
    set_timer0(100); 
    timer_count++;
}
//========================================
void bao_thuc();
//========================================
void quet_phim();
//========================================
void chedo_mode();
//======================================== 
void timer0_init() {
   setup_timer_0(RTCC_INTERNAL | RTCC_DIV_128); 
   set_timer0(100);
   enable_interrupts(INT_TIMER0);
//!   enable_interrupts(GLOBAL);
}
//========================================
void start_signal() {
    output_drive(DHT22_PIN);
    output_low(DHT22_PIN);
    delay_ms(18);
    output_high(DHT22_PIN);
    delay_us(30);
    output_float(DHT22_PIN); 
}
short check_response() {
    int timeout = 0;
    while (!input(DHT22_PIN)) {
        if (++timeout > 1000) return 0; 
        delay_us(1);
    }

    timeout = 0;
    while (input(DHT22_PIN)) {
        if (++timeout > 1000) return 0;
        delay_us(1);
    }

    return 1; 
}
//========================================
unsigned int8 read_data() {
    unsigned int8 data = 0;
    for (int i = 0; i < 8; i++) {
        while (!input(DHT22_PIN)); 
        delay_us(30);
        if (input(DHT22_PIN)) {
            data |= (1 << (7 - i));
            while (input(DHT22_PIN)); 
        }
    }
    return data;
}
//========================================
short read_dht22() {
    unsigned int8 humidity_high, humidity_low, temp_high, temp_low, checksum;
    unsigned int16 raw_humidity, raw_temp;

    start_signal();
    if (check_response()) {
        humidity_high = read_data();
        humidity_low = read_data();
        temp_high = read_data();
        temp_low = read_data();
        checksum = read_data();

        if (checksum == (humidity_high + humidity_low + temp_high + temp_low)) {
            raw_humidity = ((unsigned int16)humidity_high << 8) | humidity_low;
            raw_temp = ((unsigned int16)(temp_high & 0x7F) << 8) | temp_low;

            humidity = raw_humidity / 10.0;
            temperature = raw_temp / 10.0;
            if (temp_high & 0x80)
                temperature = -temperature;
            return 1;
        }
    }
    return 0;
}
//========================================
void display_time_lcd(unsigned int8 gio, unsigned int8 phut, unsigned int8 giay) {
    char buffer[17];
    // Format: hh:mm:ss
    sprintf(buffer, "%02u:%02u:%02u", gio, phut, giay);
    char line1[] = "Real time:";
    I2C_LCD_Set_Cursor(0, 0);
    I2C_LCD_Write_String(line1);
    I2C_LCD_Set_Cursor(1, 5);
    I2C_LCD_Write_String(buffer);
}
//========================================
void display_float(char label[], float value, int line) {
   int8 nguyen = (int8)value;
   int8 thapphan = (int8)((value - nguyen) * 10); 

   char buffer[16];
   I2C_LCD_Set_Cursor(line, 0);
   I2C_LCD_Write_String(label);
   sprintf(buffer, "%d.%d", nguyen, thapphan);
   I2C_LCD_Set_Cursor(line, 10);
   I2C_LCD_Write_String(buffer);
}
//========================================
void display_date_lcd(unsigned int8 ngay, unsigned int8 thang, unsigned int16 nam) {
    char buffer[17];

    // Format: dd/mm/yyyy
    sprintf(buffer, "%02u/%02u/%04lu", ngay, thang, (unsigned long)(nam+2000));
    
    char line1[] = "Real date:";
    I2C_LCD_Set_Cursor(0, 0);
    I2C_LCD_Write_String(line1);
    I2C_LCD_Set_Cursor(1, 2);
    I2C_LCD_Write_String(buffer);
}
//========================================
void main()
{
   ds1307_init (); 
   I2C_LCD_Init();
   SETUP_ADC_PORTS (NO_ANALOGS);
   set_tris_b(0b10000001); // RB7 input, RB6 output, RB0 input
   set_tris_c(0b00011111); // RC0-4 input (I2C + button)
   set_tris_d(0b00000010); // RD1 input
   set_tris_a(0b11000000); // RA6-7 for oscillator, RA4 output
   set_tris_e(0b00000000);
   set_gio = gio;
   set_phut = phut;
   set_giay = giay-10;
   flag_enable= 0;
   toggle = false;
//!   ds1307_set_date_time(22,7,0,0,26,5,25);
   I2C_LCD_Clear();
   I2C_LCD_BackLight(1);
   char line1[] = "   START ";
   char line2[] = "   DO AN 1 ";
   I2C_LCD_Set_Cursor(0,0);
   I2C_LCD_Write_String(line1);
   I2C_LCD_Set_Cursor(1,0);
   I2C_LCD_Write_String(line2);
   delay_ms(1000);
   ds1307_get_date (thu,ngay,thang,nam);
   ds1307_get_time (gio, phut, giay);
   I2C_LCD_Clear();
   output_low(LED1);
   output_high(BUZZER);
   ds1307_get_time (gio, phut, giay);
   timer0_init();
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_8); 
   set_timer1(3036); 
   enable_interrupts(INT_TIMER1);
   enable_interrupts(GLOBAL);
   read_dht22();
   WHILE (true) 
   {  
      bao_thuc();
      if (TEMP == 10)
      {
         if (!read_dht22()) {
            I2C_LCD_Set_Cursor(0, 0);
            char line2[] = "Error sensor ";
            I2C_LCD_Write_String(line2);
         }
         
      }
      printf("%0.1f;%0.1f;%02d:%02d:%04d:%02d:%02d:%02d\r\n", 
         humidity, temperature, 
         ngay, thang, nam, 
         gio, phut, giay);
      if(chinh == 0){
         ds1307_get_time (gio, phut, giay);
         ds1307_get_date (thu,ngay,thang,nam);
      }
      chedo_mode ();
      quet_phim();
   }
}
//========================================
int8 max_day_in_month(int8 month, int16 year) {
   switch (month) {
      case 1: case 3: case 5: case 7: case 8: case 10: case 12:
         return 31;
      case 4: case 6: case 9: case 11:
         return 30;
      case 2:
         if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
            return 29;
         else
            return 28;
      default:
         return 30; 
   }
}
//========================================
void quet_phim() 
{
   if (chinh != 0) 
   {
      ct++; 
      if (ct == 20) 
      {
         ct = 10; 
         bct = ! bct; 
      }
   }
   if (np) 
   {
      if (!input (tang)) 
      {
         ct = 0; 
         bct = 0; 
         np = 0; 
         SWITCH (chinh) 
         {
            CASE 1:
            {
               if(mode_bt == 4)
               {
                  gio++;
                  if (gio >= 24) gio = 0;
               }
               else if(mode_bt == 5){
                  ngay++;
                  int8 max_day = max_day_in_month(thang, nam + 2000);
                  if (ngay > max_day) ngay = 1;
               }
               else{
                  if(flag_enable){
                     flag_enable = 0;
                  }
               }
               break;
            }

            CASE 2:
            {
               if(mode_bt == 4)
               {
                  phut++;
                  if (phut >= 60) phut = 0;
               }
               else if(mode_bt == 5){
                  thang++;
                  if (thang > 12)thang = 1;
                  int8 max_day = max_day_in_month(thang, nam + 2000);
                  if (ngay > max_day) ngay = max_day; 
               }
               else{
                  set_gio++;
                  if (set_gio >= 24) set_gio = 0;
               }
               break;
            }

            CASE 3:
            {
               if(mode_bt == 4)
               {
                  giay++;
                  if (giay >= 60) giay = 0;
               }
               else if(mode_bt == 5){
                  nam++;
                  if (nam > 99) nam = 0;
                  int8 max_day = max_day_in_month(thang, nam + 2000);
                  if (ngay > max_day) ngay = max_day; 
               }
               else{
                  set_phut++;
                  if (set_phut >= 60) set_phut = 0;
                  
               }
               break;
            }
            CASE 4:
            {
               if(mode_bt == 6)
               {
                  set_giay++;
                  if (set_giay >= 60) set_giay = 0;
               }
            }
         }
      }

      if (!input (giam))
      {
         ct = 0; 
         bct = 0; 
         np = 0; 
         SWITCH (chinh) 
         {
            CASE 1: 
            {
               if(mode_bt == 4)
               {
                  gio--; 
                  if (gio == 255) gio = 23;
               }
               else if(mode_bt == 5){
                  ngay--;
                  int8 max_day = max_day_in_month(thang, nam + 2000);
                  if (ngay == 0) ngay = max_day;
               }
               else{
                  if(flag_enable){
                     flag_enable = 0;
                  }
               }
               break;
            }

            CASE 2:
            {  
               if(mode_bt == 4)
               {
                  phut--;
                  if (phut == 255) phut = 59;
               }
               else if(mode_bt == 5){
                  thang--;
                  if (thang == 0) thang = 12;
                  int8 max_day = max_day_in_month(thang, nam + 2000);
                  if (ngay > max_day) ngay = max_day;
               }
               else{
                  set_gio--; 
                  if (set_gio == 255) set_gio = 23;
                  
               }
               break;
            }

            CASE 3:
            {
               if(mode_bt == 4)
               {
                  giay--;
                  if (giay == 255) giay = 59;
               }
               else if(mode_bt == 5){
                  nam--;
                  if (nam == 255) nam = 99;
                  int8 max_day = max_day_in_month(thang, nam + 2000);
                  if (ngay > max_day) ngay = max_day;
               }
               else{
                  set_phut--;
                  if (set_phut == 255) set_phut = 59;
                  
               }
               break;
            }
            CASE 4:
            {  
               if(mode_bt == 6){
                  set_giay--;
                  if (set_giay == 255) set_giay = 59;
               }
            }
         }
      }

      //----------------------------------------
      if (!input (select))
      {
         ct = 0;
         bct = 0;
         chinh++; 
         np = 0; 
         if (chinh == 4) 
         {
            if(mode_bt == 4)
            {
               chinh = 0;
               ds1307_dat_gpg (gio, phut, giay); 
            }
            else if (mode_bt == 5)
            {
               chinh = 0;
               ds1307_dat_NTN(ngay,thang,nam);
            } 
         }
         if (chinh == 5) 
         {
            if(mode_bt == 6)
            {
               chinh = 0;
            }
         }
      }
   }

   ELSE 
   {
      if (input (select)&&input (tang)&&input (giam)) 
      np = 1; 
   }
}
//========================================
void chedo_mode() 
{
   if (!input (md_system))
   {
      delay_ms (20);
      if ( ! input (md_system) )
      {
         I2C_LCD_Clear();
         mode_bt++;
         chinh = 0;
         WHILE ( ! input (md_system)) ;
      }
   }
   if (mode_bt == 0)
   {
      char line1[] = "Le Duy Bao";
      char line2[] = "Nguyen Duy Cuong";
      I2C_LCD_Set_Cursor(0,0);
      I2C_LCD_Write_String(line1);
      I2C_LCD_Set_Cursor(1,0);
      I2C_LCD_Write_String(line2);
   }
      else if (mode_bt == 1)
   {
      display_time_lcd(gio, phut, giay);
   }

   else if (mode_bt == 2)
   {
      display_date_lcd(ngay, thang, nam);
   }

   else if (mode_bt == 3)
   {
      char line1[] = "Nhiet Do: ";
      display_float(line1,temperature, 0);
      I2C_LCD_Set_Cursor(0,14);
      I2C_LCD_Write_Data("oC");
      char line2[] = "Do am: ";
      display_float(line2,humidity, 1);
      I2C_LCD_Set_Cursor(1,14);
      I2C_LCD_Write_Data("%");
   }

   else if (mode_bt == 4)
   {
      I2C_LCD_Set_Cursor(0, 0);
      char line1[] = "Set time:";
      I2C_LCD_Write_String(line1);

      I2C_LCD_Set_Cursor(1, 0);
      if (chinh == 1 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((gio / 10) + '0');
         I2C_LCD_Write_Data((gio % 10) + '0');
      }
   
      I2C_LCD_Write_Data(':');
   
      if (chinh == 2 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((phut / 10) + '0');
         I2C_LCD_Write_Data((phut % 10) + '0');
      }
   
      I2C_LCD_Write_Data(':');

      if (chinh == 3 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((giay / 10) + '0');
         I2C_LCD_Write_Data((giay % 10) + '0');
      }
   }
   else if (mode_bt == 5)
   {
      I2C_LCD_Set_Cursor(0, 0);
      char line1[] = "Set date:";
      I2C_LCD_Write_String(line1);

      I2C_LCD_Set_Cursor(1, 0);

      if (chinh == 1 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((ngay / 10) + '0');
         I2C_LCD_Write_Data((ngay % 10) + '0');
      }
   
      I2C_LCD_Write_Data('/');
      if (chinh == 2 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((thang / 10) + '0');
         I2C_LCD_Write_Data((thang % 10) + '0');
      }
      I2C_LCD_Write_Data('/');
      if (chinh == 3 && bct == 0){
         char line1[] = "    ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         unsigned int16 tempe = nam + 2000;
         I2C_LCD_Write_Data((tempe / 1000) % 10 + '0');
         I2C_LCD_Write_Data((tempe / 100) % 10 + '0');
         I2C_LCD_Write_Data((tempe / 10) % 10 + '0');
         I2C_LCD_Write_Data(tempe % 10 + '0');
      }
   }
   else if (mode_bt == 6)
   {
      I2C_LCD_Set_Cursor(0, 0);
      char line1[] = "BAO THUC:";
      I2C_LCD_Write_String(line1);
      if (chinh == 1 && bct == 0){
         char line1[] = "   ";
         I2C_LCD_Set_Cursor(0, 10);
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Set_Cursor(0, 10);
         if(flag_enable){
            char line1[] = "ON";
            I2C_LCD_Set_Cursor(0, 10);
            I2C_LCD_Write_String(line1);
         }
         else{
            char line1[] = "OFF";
            I2C_LCD_Set_Cursor(0, 10);
            I2C_LCD_Write_String(line1);
         }
      }
      I2C_LCD_Set_Cursor(1, 0);
      if (chinh == 2 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((set_gio / 10) + '0');
         I2C_LCD_Write_Data((set_gio % 10) + '0');
      }
   
      I2C_LCD_Write_Data(':');
   
      if (chinh == 3 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((set_phut / 10) + '0');
         I2C_LCD_Write_Data((set_phut % 10) + '0');
      }
   
      I2C_LCD_Write_Data(':');

      if (chinh == 4 && bct == 0){
         char line1[] = "  ";
         I2C_LCD_Write_String(line1);
      } 
      else {
         I2C_LCD_Write_Data((set_giay / 10) + '0');
         I2C_LCD_Write_Data((set_giay % 10) + '0');
      }
   }
   else if (mode_bt == 7)
   {
      mode_bt = 0;
   }
}
void bao_thuc()
{
   IF (set_gio == gio&&set_phut == phut&&set_giay==giay)
   {
      flag_enable = 1;
   }

   IF (flag_enable == 1)
   {
      if(toggle){
         output_high(BUZZER);
         output_low(LED1);
      }
      else
      {
         output_low(BUZZER);
         output_high(LED1);
      }
   }
  ELSE
  {
    output_high(LED1);
    output_low(BUZZER);
  }
}
//========================================

