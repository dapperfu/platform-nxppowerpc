
#ifndef _SPI_H
#define _SPI_H

#include "project.h"

void init_dspi_2 (void);
void init_dspi_1 (void);
void init_dspi_ports(void);
void read_data_DSPI_2(void);
void read_data_DSPI_1(void);

#if 0
void init_dspi_3 (void);
void init_spi_1 (void);
void init_spi_ports(void);
void read_data_DSPI_3(void);
void read_data_SPI_1(void);
#endif

#endif /* _SPI_H */
