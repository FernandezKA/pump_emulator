#include "spi.h"

void _spi_start(uint8_t dev)
{
  //select device on NSS line
  if (0x00 == dev)
  {
    GPIO_OCTL(SPI_NSS_PORT) &= ~SPI_NSS_0;
  }
  else if (0x01 == dev)
  {
    GPIO_OCTL(SPI_NSS_PORT) &= ~SPI_NSS_1;
  }
  //Reset clk 
  GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
	for(uint8_t i = 0; i < 0xFF; ++i) {__NOP();}
}
void _spi_stop(uint8_t dev)
{
  if (0x00 == dev)
  {
    GPIO_OCTL(SPI_NSS_PORT) |= SPI_NSS_0;
  }
  else if (0x01 == dev)
  {
    GPIO_OCTL(SPI_NSS_PORT) |= SPI_NSS_1;
  }
  //Reset clk
  GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
}
void _spi_sendbyte(unsigned char d)
{
  unsigned char i;
  for (i = 0; i < 2; i++)
  {
    GPIO_OCTL(SPI_PORT) &= ~SPI_SDI;
		for(uint8_t i = 0; i < 0xFF; ++i) {__NOP();}
    GPIO_OCTL(SPI_PORT) |= SPI_SCK;
		for(uint8_t i = 0; i < 0xFF; ++i) {__NOP();}
    GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
		for(uint8_t i = 0; i < 0xFF; ++i) {__NOP();}
  }
  for (i = 0; i < 8; i++)
  {
    //Get MSB bit to out
    if (d & 0x80)
    {
      GPIO_OCTL(SPI_PORT) |= SPI_SDI;
    }
    else
    {
      GPIO_OCTL(SPI_PORT) &= ~SPI_SDI;
    }
		for(uint8_t i = 0; i < 0xFF; ++i) {__NOP();}
		//Reset clk signal 
    GPIO_OCTL(SPI_PORT) |= SPI_SCK;
    //Shift value for next using
    d <<= 1;  
    //Lath value at rising edge clk
		for(uint8_t i = 0; i < 0xFF; ++i) {__NOP();}
    GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
    //Delay can be removed
    for(uint8_t i = 0; i < 0xFF; ++i) {__NOP();}
		GPIO_OCTL(SPI_PORT) &= ~SPI_SDI;
  }
}
void _AD8400_set(unsigned char voltage_step, uint8_t dev)
{
  _spi_start(dev);
  _spi_sendbyte((unsigned char)voltage_step);
  _spi_stop(dev);
}