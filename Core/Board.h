#ifndef BOARD_H
#define BOARD_H

#define STM32G0

// Core config
//#define CORE_USE_TICK_IRQ

// CLK config
//#define CLK_USE_HSE
//#define CLK_USE_LSE
//#define CLK_LSE_BYPASS
//#define CLK_LSE_FREQ		32768
//#define CLK_SYSCLK_FREQ	32000000

// RTC config
//#define RTC_USE_IRQS

// US config
//#define US_TIM			TIM_22
//#define US_RES			1

// ADC config
//#define ADC_VREF	        3300

// GPIO config
//#define GPIO_USE_IRQS
//#define GPIO_IRQ0_ENABLE

// TIM config
//#define TIM_USE_IRQS
//#define TIM2_ENABLE

// UART config
#define UART2_PINS			(PA2 | PA3)
#define UART2_AF		  	GPIO_AF1_USART2
#define UART_BFR_SIZE     	128

// SPI config
#define SPI1_PINS		    (PB3 | PB4 | PB5)
#define SPI1_AF				GPIO_AF0_SPI1

// I2C config
//#define I2C1_GPIO			GPIOB
//#define I2C1_PINS			(GPIO_PIN_6 | GPIO_PIN_7)
//#define I2C1_AF			GPIO_AF1_I2C1
//#define I2C_USE_FASTMODEPLUS

// CAN config
//#define CAN_GPIO			GPIOB
//#define CAN_PINS			(GPIO_PIN_8 | GPIO_PIN_9)
//#define CAN_AF			GPIO_AF4_CAN
//#define CAN_DUAL_FIFO

// USB config
//#define USB_ENABLE
//#define USB_CLASS_CDC
//#define USB_CDC_BFR_SIZE	512

#define W5500_CS_PIN		PB6
#define W5500_INT_PIN		PA15
#define W5500_RST_PIN		PA10 // PA12?
#define W5500_SPI			SPI_1

#define LED_TX_PIN			PA4
#define LED_RX_PIN			PA5

#define SERIAL				UART_2
#define SERIAL_BAUD			38400

#define DHCP_SOCKET			0

#define DETECT_SOCKET		1
#define DETECT_PORT			18191
#define DETECT_STRING		"find_it6300"

#define COM_SOCKET			2
#define COM_PORT			5025


#endif /* BOARD_H */
