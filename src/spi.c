#include "spi.h"

void _spi_start(uint8_t dev)
{
	if(0x00 == dev){
		GPIO_OCTL(SPI_NSS_PORT) &= ~SPI_NSS_0;
	}
	else if(0x01 == dev)
	{
		GPIO_OCTL(SPI_NSS_PORT) &= ~SPI_NSS_1;
	}
  GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
}
void _spi_stop(uint8_t dev)
{
	if(0x00 == dev)
	{
		GPIO_OCTL(SPI_NSS_PORT) |=SPI_NSS_0;
	}
	else if(0x01 == dev)
	{
		GPIO_OCTL(SPI_NSS_PORT) |=SPI_NSS_1;
	}
  GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
}
void _spi_sendbyte(unsigned char d)
{
  unsigned char i;
  for (i = 0; i < 2; i++)
  {
    GPIO_OCTL(SPI_PORT) &= ~SPI_SDI;
    GPIO_OCTL(SPI_PORT) |= SPI_SCK;
    GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
  }
  for (i = 0; i < 8; i++)
  {
    if (d & 0x80)
    {
      GPIO_OCTL(SPI_PORT) |= SPI_SDI;
    }
    else
    {
      GPIO_OCTL(SPI_PORT) &= ~SPI_SDI;
    }
    GPIO_OCTL(SPI_PORT) |= SPI_SCK;

    d <<= 1;
    GPIO_OCTL(SPI_PORT) &= ~SPI_SCK;
  }
}
void _AD8400_set(unsigned char voltage_step, uint8_t dev)
{
  _spi_start(dev);
  _spi_sendbyte((unsigned char)voltage_step);
  _spi_stop(dev);
}