#include "gpio.h"
#include <lpc2xxx.h>

void
initPin30()
{
  PINSEL1 &= 0xCFFFFFFF;
  IODIR0 = 0x40000000;
}

void
pin30High()
{
  IOSET0 = 0x40000000;
}

void
pin30Low()
{
  IOCLR0 = 0x40000000;
}
