#ifndef _spi_h_
#define _spi_h_
#include "main.h"

#define SPI_PORT GPIOA
#define SPI_NSS_PORT GPIOB
#define SPI_SCK (1 << 5)
#define SPI_NSS_0 (1 << 12)
#define SPI_NSS_1 (1 << 10)
#define SPI_SDI (1 << 7)

void _spi_start(uint8_t dev);
void _spi_stop(uint8_t dev);
void _spi_sendbyte(unsigned char d);
void _AD8400_set(unsigned char voltage_step, uint8_t dev);

#endif