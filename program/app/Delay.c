#include "delay.h"
#include "INTRINS.H"
/*****************************************************************************/
void DelayUs(unsigned int time)
{//48M C8051
	for(;time>0;time--)
	{
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_(); //10
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//20
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//30
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//40
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();//48
	}
}

void DelayMs(unsigned int time)
{
  unsigned int i,j;
  for(i=0;i<time;i++)
    for(j=0;j< CONFIG_DELAY_MS_PARA;j++);
}

void DelayS(unsigned int time)
{
	unsigned int i;
	for(i=0;i<time;i++)
		DelayMs( CONFIG_DELAY_S_PARA );
}

