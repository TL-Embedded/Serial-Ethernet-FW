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

// GPIO config
//#define GPIO_USE_IRQS
//#define GPIO_IRQ0_ENABLE


// UART config
#define UART2_PINS			(PA2 | PA3)
#define UART2_AF		  	GPIO_AF1_USART2
#define UART_BFR_SIZE     	128

// SPI config
#define SPI1_PINS		    (PB3 | PB4 | PB5)
#define SPI1_AF				GPIO_AF0_SPI1


// W5500 config
#define W5500_CS_PIN		PB6
#define W5500_INT_PIN		PA15
#define W5500_RST_PIN		PA10 // PA12?
#define W5500_SPI			SPI_1

#define LED_TX_PIN			PA4
#define LED_RX_PIN			PA5

#define SERIAL				UART_2
#define SERIAL_BAUD			38400

#define DHCP_SOCKET			0
#define MDNS_SOCKET			1

#define COM_SOCKET			2
#define COM_PORT			5025


#endif /* BOARD_H */
