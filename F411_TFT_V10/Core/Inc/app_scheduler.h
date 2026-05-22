#ifndef __APP_SCHEDULER_H__
#define __APP_SCHEDULER_H__

#include "main.h"

/* 软件10ms节拍宏: 设为1使用HAL_GetTick软件节拍, 0使用硬件TIM */
#define APP_USE_SOFT_10MS_TICK  1

void App_Init(void);
void App_Run(void);

#endif
