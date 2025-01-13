#include "display.h"
#include "main.h"
#include "platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern SPI_HandleTypeDef hspi2;

#define Set_Lower_Column_Start_Address_CMD 0x00
#define Set_Higher_Column_Start_Address_CMD 0x10
#define Set_Memory_Addressing_Mode_CMD 0x20
#define Set_Column_Address_CMD 0x21
#define Set_Page_Address_CMD 0x22
#define Set_Display_Start_Line_CMD 0x40
#define Set_Contrast_Control_CMD 0x81
#define Set_Charge_Pump_CMD 0x8D
#define Set_Segment_Remap_CMD 0xA0
#define Set_Entire_Display_ON_CMD 0xA4
#define Set_Normal_or_Inverse_Display_CMD 0xA6
#define Set_Multiplex_Ratio_CMD 0xA8
#define Set_Display_ON_or_OFF_CMD 0xAE
#define Set_Page_Start_Address_CMD 0xB0
#define Set_COM_Output_Scan_Direction_CMD 0xC0
#define Set_Display_Offset_CMD 0xD3
#define Set_Display_Clock_CMD 0xD5
#define Set_Pre_charge_Period_CMD 0xD9
#define Set_Common_HW_Config_CMD 0xDA
#define Set_VCOMH_Level_CMD 0xDB
#define Set_NOP_CMD 0xE3

#define Horizontal_Addressing_Mode 0x00
#define Vertical_Addressing_Mode 0x01
#define Page_Addressing_Mode 0x02

#define Disable_Charge_Pump 0x00
#define Enable_Charge_Pump 0x04

#define Column_Address_0_Mapped_to_SEG0 0x00
#define Column_Address_0_Mapped_to_SEG127 0x01

#define Normal_Display 0x00
#define Entire_Display_ON 0x01

#define Non_Inverted_Display 0x00
#define Inverted_Display 0x01

#define Display_OFF 0x00
#define Display_ON 0x01

#define Scan_from_COM0_to_63 0x00
#define Scan_from_COM63_to_0 0x08

#define CS0 PIN_CLR(D_CS)
#define CS1 PIN_SET(D_CS)

#define DC0 PIN_CLR(D_DC)
#define DC1 PIN_SET(D_DC)

static char buf_print[256];
static uint8_t disp_buf[8 * 128] = {0};
static bool update_pending = true;
static volatile bool busy = false;

static struct
{
	const uint8_t *font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
	uint8_t inverse;
} cfont = {0};

static void Display_write(uint8_t value, uint8_t type)
{
	if(type)
		DC1;
	else
		DC0;
	CS1;
	CS0;
	HAL_SPI_Transmit(&hspi2, &value, 1, 100);
	CS1;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == &hspi2)
	{
		CS1;
		busy = false;
	}
}

void display_init(void)
{
	__HAL_SPI_ENABLE(&hspi2);
	CS1;

	Display_write(Set_Display_ON_or_OFF_CMD + Display_OFF, 0);

	Display_write(Set_Display_Clock_CMD, 0);
	Display_write(0xD0, 0); // x80

	Display_write(Set_Multiplex_Ratio_CMD, 0);
	Display_write(0x3F, 0);

	Display_write(Set_Display_Offset_CMD, 0);
	Display_write(0x00, 0);

	Display_write((Set_Display_Start_Line_CMD | 0x00), 0);

	Display_write(Set_Charge_Pump_CMD, 0);
	Display_write(Set_Higher_Column_Start_Address_CMD | Enable_Charge_Pump, 0);

	Display_write(Set_Memory_Addressing_Mode_CMD, 0);
	Display_write(Horizontal_Addressing_Mode, 0); // Page_Addressing_Mode

	Display_write(Set_Segment_Remap_CMD | Column_Address_0_Mapped_to_SEG127, 0);

#if 1
	Display_write(Set_COM_Output_Scan_Direction_CMD | Scan_from_COM0_to_63, 0);
#else
	Display_write(Set_COM_Output_Scan_Direction_CMD | Scan_from_COM63_to_0, 0); // inverts along the short size
#endif

	Display_write(Set_Common_HW_Config_CMD, 0);
	Display_write(0x12, 0);

	Display_write(Set_Contrast_Control_CMD, 0);
	Display_write(0x10, 0); // xCF

	Display_write(Set_Pre_charge_Period_CMD, 0);
	Display_write(0xF1, 0);

	Display_write(Set_VCOMH_Level_CMD, 0);
	Display_write(0x3C, 0); // x40

	Display_write(Set_Entire_Display_ON_CMD | Normal_Display, 0);
	Display_write(Set_Normal_or_Inverse_Display_CMD | Non_Inverted_Display, 0);
	Display_write(Set_Display_ON_or_OFF_CMD + Display_ON, 0);

	display_update();
}

void display_off(void)
{
	while(busy)
		asm("nop");
	Display_write(Set_Display_ON_or_OFF_CMD + Display_OFF, 0);
}

void display_update(void)
{
	if(busy) return;
	update_pending = false;
	busy = true;
	Display_write(Set_Page_Start_Address_CMD, 0);
	Display_write(Set_Lower_Column_Start_Address_CMD, 0);
	Display_write(Set_Higher_Column_Start_Address_CMD, 0);
	DC1;
	CS1;
	CS0;
	HAL_SPI_Transmit_DMA(&hspi2, disp_buf, sizeof(disp_buf));
}

bool display_spi_is_busy(void) { return busy; }
bool display_is_update_pending(void) { return update_pending && !busy; }
void display_set_update_pending(void) { update_pending = true; }
void display_buffer_clear(void) { memset(disp_buf, 0x0, sizeof(disp_buf)); }

void display_pixel(uint16_t x, uint16_t y, uint8_t is_filled)
{
	if(x >= 128 || y >= 64) return;
	if(is_filled)
		disp_buf[x + y / 8 * 128] |= (1 << (y % 8));
	else
		disp_buf[x + y / 8 * 128] &= ~(1 << (y % 8));
}

void display_invert_pixel(uint16_t x, uint16_t y)
{
	if(x >= 128 || y >= 64)
		return;
	x = 128 - 1 - x;

	if(disp_buf[x + y / 8 * 128] & (1 << (y % 8)))
		disp_buf[x + y / 8 * 128] &= ~(1 << (y % 8));
	else
		disp_buf[x + y / 8 * 128] |= (1 << (y % 8));
}

void display_invert(uint16_t x, uint16_t y, uint16_t length, uint16_t height)
{
	for(uint16_t i = 0; i < length; i++)
		for(uint16_t j = 0; j < height; j++)
			display_invert_pixel(x + i, y + j);
}

void display_line_h(uint16_t x, uint16_t y, uint16_t length, uint8_t is_filled)
{
	for(uint16_t i = 0; i < length; i++)
	{
		display_pixel(x + i, y, is_filled);
	}
}

void display_line_v(uint16_t x, uint16_t y, uint16_t length, uint8_t is_filled)
{
	for(uint16_t i = 0; i < length; i++)
	{
		display_pixel(x, y + i, is_filled);
	}
}

void display_set_font(const uint8_t *font)
{
	cfont.font = font;
	cfont.x_size = cfont.font[0];
	cfont.y_size = cfont.font[1];
	cfont.offset = cfont.font[2];
	cfont.numchars = cfont.font[3];
	cfont.inverse = cfont.font[4];
}

#define BitIsSet(x, y) ((x) & (1 << (y)))

void display_char(uint16_t x, uint16_t y, char s, uint8_t is_filled, uint8_t *act_width)
{
#define FONT_WIDTH cfont.x_size
#define FONT_HEIGHT cfont.y_size

	*act_width = 0;
	if(s >= cfont.offset + cfont.numchars) return;

	uint8_t scale = 1;
	uint8_t _transparent = 0;

	uint32_t temp = (uint32_t)((s - cfont.offset) * FONT_WIDTH * (FONT_HEIGHT < 8 ? 8 : FONT_HEIGHT) / 8) + 5;

	if(BitIsSet(cfont.inverse, 7))
	{ // STD FONTS
		for(int16_t j = FONT_HEIGHT * scale - 1; j > -1; j--)
		{
			for(int8_t k = 0; k < FONT_WIDTH / 8; k++)
			{
				char frame = cfont.font[temp++];
				for(uint8_t i = 0; i < 8 * scale; i++)
				{
					if(BitIsSet(frame, cfont.inverse & 0x01 ? (7 - i) : (i)))
					{
						display_pixel(x + k * 8 * scale + 8 * scale - 1 - i, y + j, is_filled);
						if(*act_width < k * 8 * scale + 8 * scale - 2 - i) *act_width = k * 8 * scale + 8 * scale - 2 - i;
					}
					else if(!_transparent)
					{
						display_pixel(x + k * 8 * scale + 8 * scale - 1 - i, y + j, !is_filled);
					}
				}
			}
			if(j % (scale) != 0) temp -= 2;
		}
	}
	else
	{
		for(uint16_t j = 0; j < FONT_WIDTH * (FONT_HEIGHT / 8 + FONT_HEIGHT % 8 ? 1 : 0) * scale; j++)
		{
			char frame = cfont.font[temp];

			for(uint8_t i = 0; i < FONT_HEIGHT * scale; i++)
			{
				if(BitIsSet(frame, cfont.inverse & 0x01 ? (7 - i / scale) : (i / scale)))
				{
					display_pixel(x + j, y + i, is_filled);
					if(*act_width < j + 1) *act_width = j + 1;
				}
				else if(!_transparent)
				{
					display_pixel(x + j, y + i, !is_filled);
				}
			}
			if(j % scale == (scale - 1)) temp++;
		}
	}
}

void display_print_str(uint16_t x, uint16_t y, bool monospace, const char *s)
{
	for(const char *it = s; *it; it++)
	{
		uint8_t act_width;
		display_char(x, y, *it, 1, &act_width);
		x += (monospace ? act_width : cfont.x_size) + 1;
	}
}

void display_print_str_n(uint16_t x, uint16_t y, bool monospace, const char *s, uint32_t len)
{
	for(uint32_t i = 0; i < len; i++)
	{
		uint8_t act_width;
		display_char(x, y, s[i], 1, &act_width);
		x += (monospace ? act_width : cfont.x_size) + 1;
	}
}

void display_print(uint16_t x, uint16_t y, bool monospace, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buf_print, sizeof(buf_print), format, args);
	va_end(args);
	display_print_str(x, y, monospace, buf_print);
}
