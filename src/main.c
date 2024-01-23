#include "../system/include/cmsis/stm32f4xx.h"

#define PIN 12  // Номер пина

void System_Clock_Config (void) {
	// Prescaler Configrations
	RCC->CFGR |= (5 << 10);			// APB1 Prescaler = 4
	RCC->CFGR |= (4 << 13);			// APB2 Prescaler = 2

	RCC->CR |= (1 << 16);			// HSE Clock Enable - HSEON
	while(!(RCC->CR & 0x00020000));		// Wait until HSE Clock Ready - HSERDY

	// PLL Configrations
	RCC->PLLCFGR = 0;					// Clear all PLLCFGR register
	RCC->PLLCFGR |=  (8		<<  0);		// PLLM = 8;
	RCC->PLLCFGR |=  (336 	<<  6);		// PLLN = 336;
	RCC->PLLCFGR &= ~(3 	<< 16);		// PLLP = 2; // For 2, Write 0
	RCC->PLLCFGR |=  (1 	<< 22);		// HSE Oscillator clock select as PLL
	RCC->PLLCFGR |=  (7 	<< 24);		// PLLQ = 7;

	RCC->CR 		|=  (1 		<< 24); // PLL Clock Enable - PLLON
	while(!(RCC->CR & 0x02000000)); 	// Wait until PLL Clock Ready - PLLRDY

	// Flash Configrations
	FLASH->ACR = 0;						// Clear all ACR register (Access Control Register)
	FLASH->ACR |= (5 <<  0); 		// Latency - 5 Wait State
	FLASH->ACR |= (1 <<  9);		// Instruction Cache Enable
	FLASH->ACR |= (1 << 10);		// Data Cache Enable

	RCC->CFGR |= (2 <<  0);		// PLL Selected as System Clock
	while((RCC->CFGR & 0x0F) != 0x0A); 	// Wait PLL On
}

/*  Прерывание надо для плавного увеличения ШИМ, но в целом можно и без него,
 *  а рализовать отдельными функциями. В данном случае, светодиод не будет мигать,
 *  а будет плавно загоратся.
 * */

uint32_t cnt = 0;
uint32_t cnt_stop = 0xFFFFFFFF;

void TIM4_IRQHandler(void) {
    
    GPIOD->ODR |= GPIO_ODR_OD13;
    ++cnt;
    if (cnt == cnt_stop)
    	TIM4->CR1 &= ~TIM_CR1_CEN;
    GPIOD->ODR &= ~GPIO_ODR_OD13;
	TIM4->SR &= ~TIM_SR_UIF;

//    TIM4->CR1 |= TIM_CR1_CEN;
}

void make_n_pulse(const uint32_t n) {
    cnt_stop = n;
    cnt = 0;
    TIM4->CR1 |= TIM_CR1_CEN;
}

int main(void) {
    //System_Clock_Config();
    SystemCoreClockUpdate();
    uint16_t Prescaler = (uint16_t) ((SystemCoreClock ) / 1000000 - 1);
    uint16_t Period = (uint16_t) (1000000 / 105000);

    //// Настраиваем порт светодиода
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER |= ((0x2 << (2 * PIN)) | GPIO_MODER_MODER13_0);
    GPIOD->AFR[1] |= (0x2 << 16);

    //// Настравиваем ШИМ на таймере
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    TIM4->PSC = Prescaler;
    TIM4->ARR = Period;
    TIM4->CCMR1 |= 0x60;
    TIM4->CCR1 = 4;
    TIM4->CCER |= 0x1;
    TIM4->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM4_IRQn);
    NVIC_SetPriority(TIM4_IRQn, 2);
    TIM4->CR1 |= TIM_CR1_CEN;
    TIM4->CR1 &= ~TIM_CR1_CEN;



    while(1) {
        make_n_pulse(5);
        //if (cnt < L) {
        //    GPIOD->ODR |= GPIO_ODR_OD13;
        //} else {
        //    GPIOD->ODR &= ~GPIO_ODR_OD13;
        //}
        //if (cnt == R)
        //   cnt = 0;
    }
}


