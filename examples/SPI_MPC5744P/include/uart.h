#ifndef uart_h_included
#define uart_h_included

void LINFlexD_0_Init(void);
void LINFlexD_1_Init(void);
void TransmitData(const char *pBuf, uint32_t cnt);
void ReceiveData(char *pBuf);

#endif /* uart_h_included */

