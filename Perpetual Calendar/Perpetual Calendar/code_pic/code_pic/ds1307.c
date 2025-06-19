BYTE bin2bcd(BYTE binary_value); 
BYTE bcd2bin(BYTE bcd_value);
void doc_byte_1307(byte diachi,byte &dulieu);
void ghi_byte_1307(byte diachi,byte dulieu);
//========================================
void ds1307_init(void) // khoi tao cho ds1307 
{
   ghi_byte_1307(7,0x90); // tao xung 1 hz tai chan sout DS1307
   byte a; // byte luu tam gia tri o nho 0 cua ds
   
   doc_byte_1307(0,a); // doc o nho 0
   byte b=a&0x80; // byte dung de so sanh
   if (b!=0)
   {
      a&=0x7f;
      ghi_byte_1307(0,a);
   }
} 
//======================================== 
void ds1307_dat_tgbt(byte gio_bt,phut_bt)
{ 
   i2c_start();  
   i2c_write(0xD0); // I2C write address 
   i2c_write(0x20);
   i2c_write(gio_bt); // REG 0 
   i2c_write(phut_bt); // REG 1 
   i2c_stop(); 
} 
//========================================
void ds1307_doc_tgbt(BYTE &gio_bt,BYTE &phut_bt)
{
   i2c_start();
   i2c_write(0xD0);
   i2c_write(0x20);
   //i2c_stop();
   i2c_start();
   i2c_write(0xD1);
   gio_bt = i2c_read(1);
   phut_bt = i2c_read(1);
   i2c_stop();
}
//========================================
void ds1307_dat_TNTN(byte thu,ngay,thang,nam)
{ 
   i2c_start();  
   i2c_write(0xD0); // I2C write address 
   i2c_write(0x03); // Start at REG 0 - Seconds 
   thu   = bcd2bin(i2c_read(1) & 0x07); 
   ngay  = bcd2bin(i2c_read(1) & 0x3F); 
   thang = bcd2bin(i2c_read(1) & 0x1F); 
   nam   = bcd2bin(i2c_read(0));
   i2c_write(0x90);            // REG 7 - Enable squarewave output pin 
   i2c_stop(); 
} 
//========================================
void ds1307_dat_NTN(byte ngay,thang,nam)
{ 
   i2c_start();  
   i2c_write(0xD0); // I2C write address 
   i2c_write(0x04); // Start at REG 0 - Seconds 
   i2c_write(bin2bcd(ngay)); // REG 0 
   i2c_write(bin2bcd(thang)); // REG 1 
   i2c_write(bin2bcd(nam)); // REG 2 
   i2c_write(0x90);            // REG 7 - Enable squarewave output pin 
   i2c_stop(); 
} 
//========================================
void ds1307_dat_GPG(byte gio,phut,giay) 
{ 
  giay &= 0x7F; 
  gio &= 0x3F; 

   i2c_start();  
   i2c_write(0xD0); // I2C write address 
   i2c_write(0x00); // Start at REG 0 - Seconds 
   i2c_write(bin2bcd(giay)); // REG 0 
   i2c_write(bin2bcd(phut)); // REG 1 
   i2c_write(bin2bcd(gio)); // REG 2 
   i2c_write(0x90);            // REG 7 - Enable squarewave output pin 
   i2c_stop(); 
} 
//========================================
void ds1307_set_date_time(BYTE gio, phut, giay, thu, ngay, thang, nam) 
{ 
  giay &= 0x7F; 
  gio &= 0x3F; 

  i2c_start();  
  i2c_write(0xD0);            // I2C write address 
  i2c_write(0x00);            // Start at REG 0 - Seconds 
  i2c_write(bin2bcd(giay));      // REG 0 
  i2c_write(bin2bcd(phut));      // REG 1 
  i2c_write(bin2bcd(gio));      // REG 2 
  i2c_write(bin2bcd(thu));      // REG 3 
  i2c_write(bin2bcd(ngay));      // REG 4 
  i2c_write(bin2bcd(thang));      // REG 5 
  i2c_write(bin2bcd(nam));      // REG 6 
  i2c_write(0x90);            // REG 7 - Enable squarewave output pin 
  i2c_stop(); 
} 
//========================================
void ds1307_doc_giay(BYTE &giay)
{
   i2c_start();
   i2c_write(0xD0);
   i2c_write(0x00); // Start at REG 0 - Seconds
   i2c_start();
   i2c_write(0xD1);
   giay = bcd2bin(i2c_read(0));
   i2c_stop();
}
//========================================
void ds1307_get_time(BYTE &gio,BYTE &phut,BYTE &giay)
{
   i2c_start();
   i2c_write(0xD0);
   i2c_write(0x00); // Start at REG 0 - Seconds
   i2c_start();
   i2c_write(0xD1);
   giay = bcd2bin(i2c_read(1) & 0x7F);
   phut = bcd2bin(i2c_read(1) & 0x7F);
   gio  = bcd2bin(i2c_read(0) & 0x3F);
   i2c_stop();
}
//========================================
void ds1307_get_date(BYTE &thu,BYTE &ngay,BYTE &thang,BYTE &nam)
{
   i2c_start();
   i2c_write(0xD0);
   i2c_write(3);            // Start at REG 3 - Day of week
   i2c_stop();
   i2c_start();
   i2c_write(0xD1);
   thu = bcd2bin(i2c_read(1));   // REG 3
   ngay  = bcd2bin(i2c_read(1));   // REG 4
   thang = bcd2bin(i2c_read(1));   // REG 5
   nam = bcd2bin(i2c_read(0));            // REG 6
   i2c_stop();
}
//========================================

BYTE bin2bcd(BYTE binary_value)//19 
{
   BYTE temp; 
   BYTE retval; 
   temp = binary_value; 
   retval = 0; 
   while(true) 
   { // Get the tens digit by doing multiple subtraction 
      if(temp >= 10) // of 10 from the binary value.
      { 
         temp -= 10; 
         retval += 0x10; 
      } 
      else // Get the ones digit by adding the remainder. 
      { 
         retval += temp; 
         break; 
      } 
   }
   return(retval); 
} 
//========================================

// Input range - 00 to 99. 
BYTE bcd2bin(BYTE bcd_value) //19
{
  byte tam; 
  tam = bcd_value; 
  tam >>= 1; // Shifting upper digit right by 1 is same as multiplying by 8.
  tam &= 0x78; // Isolate the bits for the upper digit. 
  return(tam + (tam >> 2) + (bcd_value & 0x0f)); // Now return: (Tens * 8) + (Tens * 2) + Ones 
  //return ( (bcd_val/16*10) + (bcd_val%16) );
}
//======================================== 
void doc_byte_1307(byte diachi,byte &dulieu)
{
   i2c_start();
   i2c_write(0xD0); // ghi vao ds1307 chieu ghi
   i2c_write(diachi); // ghi dia chi can doc
   i2c_start(); // start bit i2c 
   i2c_write(0xD1); // ghi vao ds1307 chieu doc
   dulieu = i2c_read(0);
   i2c_stop();
}
//======================================== 
void ghi_byte_1307(byte diachi,byte dulieu)
{
   i2c_start();
   i2c_write(0xD0); // ghi vao ds1307 chieu ghi
   i2c_write(diachi); // ghi dia chi can doc
   i2c_write(dulieu); // 
   i2c_stop();
}

/*#define DS1307_SDA  PIN_C4
#define DS1307_SCL  PIN_C3
#use i2c(master, sda=DS1307_SDA, scl=DS1307_SCL)
*/


/*// Convert normal decimal numbers to binary coded decimal
int8 decToBcd(int8 dec_val)
{
  return ( (dec_val/10*16) + (dec_val%10) );
}

// Convert binary coded decimal to normal decimal numbers
int8 bcdToDec(int8 bcd_val)
{
  return ( (bcd_val/16*10) + (bcd_val%16) );
}

//==========================
// initial DS1307
//==========================
void ds1307_init()
{
   output_float(DS1307_SCL);
   output_float(DS1307_SDA);
}
//==========================
// write data one byte to
// DS1307
//==========================
void write_DS1307(byte address, BYTE data)
{
   short int status;
   i2c_start();
   i2c_write(0xd0);
   i2c_write(address);
   i2c_write(data);
   i2c_stop();
   i2c_start();
   status=i2c_write(0xd0);
   while(status==1)
   {
      i2c_start();
      status=i2c_write(0xd0);
   }
}
//==========================
// read data one byte from DS1307
//==========================
BYTE read_DS1307(byte address)
{
   BYTE data;
   i2c_start();
   i2c_write(0xd0);
   i2c_write(address);
   i2c_start();
   i2c_write(0xd1);
   data=i2c_read(0);
   i2c_stop();
   return(data);
}

// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers

void ds1307_get_date(BYTE &thu,BYTE &ngay,BYTE &thang,BYTE &nam)
{
  i2c_start();
  i2c_write(0xD0);
  i2c_write(3);            // Start at REG 3 - Day of week
  i2c_stop();
  i2c_start();
  i2c_write(0xD1);
  thu = bcdToDec(i2c_read() & 0x7f);   // REG 3
  ngay  = bcdToDec(i2c_read() & 0x3f);   // REG 4
  thang = bcdToDec(i2c_read() & 0x1f);   // REG 5
  nam = bcdToDec(i2c_read(0));            // REG 6
  i2c_stop();
}

*/
