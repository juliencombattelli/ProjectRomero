#include "IT.h"
#include "stm32f10x.h"

void (* pFnc2) (void) ; 
void (* pFnc3) (void) ; 
void (* pFnc4) (void) ; 
void (* pFnc1) (void) ;

void TIM1_UP_IRQHandler ( void )
{
	if (pFnc1 != 0)
		(*pFnc1) (); /* appel indirect de la fonction */
	TIM1->SR &= 0xFE;
}


void TIM2_IRQHandler ( void )
{
	if (pFnc2 != 0)
		(*pFnc2) (); /* appel indirect de la fonction */
	TIM2->SR &= 0xFE;
}

void TIM3_IRQHandler ( void )
{
	if (pFnc3 != 0)
		(*pFnc3) (); /* appel indirect de la fonction */
	TIM3->SR &= 0xFE;
}

void TIM4_IRQHandler ( void )
{
	if (pFnc4 != 0)
		(*pFnc4) (); /* appel indirect de la fonction */
	TIM4->SR &= 0xFE;
}


void Timer_Active_IT(TIM_TypeDef *Timer, u8 Priority, void (*IT_function) (void))
{
	if (Timer ==TIM1) {
		NVIC->ISER[0] |= (0x01 << 25);
		NVIC->IP[25] = (Priority<<4);
		TIM1->DIER |= TIM_DIER_UIE;
		pFnc1 = IT_function;
	}		
  else if (Timer == TIM2) {
		NVIC->ISER[0] |= (0x01 << 28);
		NVIC->IP[28] = (Priority<<4);
		TIM2->DIER |= TIM_DIER_UIE;
		pFnc2 = IT_function;
	}
	else if (Timer == TIM3) {
		NVIC->ISER[0] |= (0x01 << 29);
		NVIC->IP[29] = (Priority<<4);
		TIM3->DIER |= TIM_DIER_UIE;
		pFnc3 = IT_function;
	}
	else if (Timer == TIM4) {
		NVIC->ISER[0] |= (0x01 << 30);
		NVIC->IP[30] = (Priority<<4);
		TIM4->DIER |= TIM_DIER_UIE;
		pFnc4 = IT_function;
	}
}
