#include "spi.h"

void _spi_start(void){
        GPIO_OCTL(SPI_PORT)&=~SPI_SCK;
				GPIO_OCTL(SPI_PORT)&=~SPI_NSS;
 }
 void _spi_stop(void){
				GPIO_OCTL(SPI_PORT)&=~SPI_SCK;
				GPIO_OCTL(SPI_PORT)|=SPI_NSS;
 }
void _spi_sendbyte(unsigned char d)
  {
     unsigned char i;
     for(i=0; i<2; i++)
     {
        //GPIO_WriteLow(SPI_PORT, SPI_SDI);
				GPIO_OCTL(SPI_PORT)&=~SPI_SDI;
        //GPIO_WriteHigh(SPI_PORT, SPI_SCK);
				GPIO_OCTL(SPI_PORT)|=SPI_SCK;
			  GPIO_OCTL(SPI_PORT)&=~SPI_SCK;
				//GPIO_WriteLow(SPI_PORT, SPI_SCK);
        //LL_GPIO_ResetOutputPin(SPI_PORT, SPI_SDI);
        //LL_GPIO_SetOutputPin(SPI_PORT, SPI_SCK);
        //LL_GPIO_ResetOutputPin(SPI_PORT, SPI_SCK);
     }
     for(i=0; i<8; i++)
     {
        if (d & 0x80)
        {
          //GPIO_WriteHigh(SPI_PORT, SPI_SDI);
      	  GPIO_OCTL(SPI_PORT)|=SPI_SDI;
					//LL_GPIO_SetOutputPin(SPI_PORT, SPI_SDI);
        }
        else
        {
      	  //LL_GPIO_ResetOutputPin(SPI_PORT, SPI_SDI);
          GPIO_OCTL(SPI_PORT)&=~SPI_SDI;
					//GPIO_WriteLow(SPI_PORT, SPI_SDI);
        }
        //GPIO_WriteHigh(SPI_PORT, SPI_SCK);
        GPIO_OCTL(SPI_PORT)|=SPI_SCK;
				//LL_GPIO_SetOutputPin(SPI_PORT, SPI_SCK);
        
        d <<= 1;
        //LL_GPIO_ResetOutputPin(SPI_PORT, SPI_SCK);
        GPIO_OCTL(SPI_PORT)&=~SPI_SCK;
				//GPIO_WriteLow(SPI_PORT, SPI_SCK);
     }
  }
void _AD8400_set(unsigned char voltage_step){
	_spi_start();
	_spi_sendbyte((unsigned char) voltage_step);
	_spi_stop();
}