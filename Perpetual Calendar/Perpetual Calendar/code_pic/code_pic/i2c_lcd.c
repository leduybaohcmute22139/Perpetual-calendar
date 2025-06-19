typedef unsigned char u8;   
static u8 lcd_light = 0x08;
#define LCD_ADDR      0x27  
#define LCD_BACKLIGHT 0x08  
#define LCD_ENABLE    0x04
#define LCD_RW        0x02
#define LCD_RS        0x01


extern void delay_us(unsigned int us);
extern void delay_ms(unsigned int ms);

extern void i2c_start(void);
extern void i2c_write(u8 data);
extern void i2c_stop(void);
void I2C_LCD_Clear(void);
void lcd_write_nibble(u8 data) {
    u8 data_en;

    data_en = data | lcd_light | LCD_ENABLE;
    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(data_en);
    i2c_stop();

    delay_us(1);

    data_en = data | lcd_light;
    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(data_en);
    i2c_stop();

    delay_us(50);
}

void lcd_send_byte(u8 value, u8 mode) {
    u8 high_nibble = (value & 0xF0) | (mode ? LCD_RS : 0);
    u8 low_nibble = ((value << 4) & 0xF0) | (mode ? LCD_RS : 0);

    lcd_write_nibble(high_nibble);
    lcd_write_nibble(low_nibble);
}

void I2C_LCD_Init(void) {
    delay_ms(50);

    lcd_write_nibble(0x30);
    delay_ms(5);
    lcd_write_nibble(0x30);
    delay_us(100);
    lcd_write_nibble(0x30);
    delay_ms(5);
    lcd_write_nibble(0x20);

    lcd_send_byte(0x28, 0);
    lcd_send_byte(0x0C, 0);
    lcd_send_byte(0x06, 0);
    I2C_LCD_Clear();
}

void I2C_LCD_Clear(void) {
    lcd_send_byte(0x01, 0);
    delay_ms(2);
}

void I2C_LCD_Set_Cursor(u8 row, u8 col) {
    u8 address;

    switch(row) {
        case 0: address = 0x00 + col; break;
        case 1: address = 0x40 + col; break;
        case 2: address = 0x14 + col; break;
        case 3: address = 0x54 + col; break;
        default: address = 0x00 + col;
    }
    lcd_send_byte(0x80 | address, 0);
}

void I2C_LCD_Write_Data(int8 lcd_data_x) {
    lcd_send_byte(lcd_data_x, 1);
    delay_us(20);
}
void I2C_LCD_Write_Char(char c) {
    lcd_send_byte((u8)c, 1);
    delay_us(20);
}
void I2C_LCD_Write_String(char* str) {
    while(*str) {
        I2C_LCD_Write_Char(*str++);
    }
}

void I2C_LCD_BackLight(u8 state) {
    if(state) {
        lcd_light = LCD_BACKLIGHT;
    } else {
        lcd_light = 0x00;
    }

    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(lcd_light);
    i2c_stop();
}

