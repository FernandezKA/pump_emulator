#ifndef _spi_h_
#define _spi_h_
#include "main.h"

#define SPI_PORT GPIOB
#define SPI_SCK (1 << 10)
#define SPI_NSS (1 << 9)
#define SPI_SDI (1 << 8)

void _spi_start(void);
void _spi_stop(void);
void _spi_sendbyte(unsigned char d);
void _AD8400_set(unsigned char voltage_step);

#endif