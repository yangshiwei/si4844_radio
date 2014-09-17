#include "delay.h"
/*****************************************************************************/
void DelayUs(unsigned int time)
{//48M C8051
#if DEBUG_DELAY == 1
#endif
	for(;time>0;time--)
	{
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_(); //10
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//20
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//30
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//40
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//48
	}
#if DEBUG_DELAY == 1
#endif
}

void DelayMs(unsigned int time)
{
  unsigned int i,j;
  for(i=0;i<time;i++)
    for(j=0;j<100;j++);
}

void DelayS(unsigned int time)
{
	unsigned int i;
	for(i=0;i<time;i++)
		DelayMs(1021);
}
