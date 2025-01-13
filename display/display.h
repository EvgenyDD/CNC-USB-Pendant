#ifndef DISPLAY_H__
#define DISPLAY_H__

#include <stdbool.h>
#include <stdint.h>

#define DT_FMT_S "%ldd %02ld:%02ld:%02ld"
#define DT_FMT_MS "%ldd %02ld:%02ld:%02ld.%03ld"
#define DT_FMT_US "%ldd %02ld:%02ld:%02ld.%03ld.%03ld"

#define DT_FMT_S_SIMPLE "%02ld:%02ld:%02ld"
#define DT_FMT_MS_SIMPLE "%02ld:%02ld:%02ld.%03ld"

#define DT_DATA_S(x) (uint32_t)(x / (86400ULL)),           \
					 (uint32_t)((x % 86400ULL) / 3600ULL), \
					 (uint32_t)((x % 3600ULL) / 60ULL),    \
					 (uint32_t)((x % 60ULL))

#define DT_DATA_MS(x) (uint32_t)(x / (86400000ULL)),              \
					  (uint32_t)((x % 86400000ULL) / 3600000ULL), \
					  (uint32_t)((x % 3600000ULL) / 60000ULL),    \
					  (uint32_t)((x % 60000ULL) / 1000ULL),       \
					  (uint32_t)((x % 1000ULL))
#define DT_DATA_US(x) (uint32_t)(x / (86400000000ULL)),                 \
					  (uint32_t)((x % 86400000000ULL) / 3600000000ULL), \
					  (uint32_t)((x % 3600000000ULL) / 60000000ULL),    \
					  (uint32_t)((x % 60000000ULL) / 1000000ULL),       \
					  (uint32_t)((x % 1000000ULL) / 1000ULL),           \
					  (uint32_t)((x % 1000ULL))

#define DT_DATA_S_SIMPLE(x) (uint32_t)((x % 86400ULL) / 3600ULL), \
							(uint32_t)((x % 3600ULL) / 60ULL),    \
							(uint32_t)((x % 60ULL))

#define DT_DATA_MS_SIMPLE(x) (uint32_t)((x % 86400000ULL) / 3600000ULL), \
							 (uint32_t)((x % 3600000ULL) / 60000ULL),    \
							 (uint32_t)((x % 60000ULL) / 1000ULL),       \
							 (uint32_t)((x % 1000ULL))

void display_init(void);
void display_off(void);
void display_buffer_clear(void);
bool display_is_update_pending(void);
void display_set_update_pending(void);
void display_update(void);

void display_pixel(uint16_t x, uint16_t y, uint8_t is_filled);
void display_invert_pixel(uint16_t x, uint16_t y);
void display_invert(uint16_t x, uint16_t y, uint16_t length, uint16_t height);

void display_line_h(uint16_t x, uint16_t y, uint16_t length, uint8_t is_filled);
void display_line_v(uint16_t x, uint16_t y, uint16_t length, uint8_t is_filled);

void display_set_font(const uint8_t *font);
void display_char(uint16_t x, uint16_t y, char s, uint8_t is_filled, uint8_t *act_width);
void display_print_str(uint16_t x, uint16_t y, bool monospace, const char *s);
void display_print_str_n(uint16_t x, uint16_t y, bool monospace, const char *s, uint32_t len);
void display_print(uint16_t x, uint16_t y, bool monospace, const char *format, ...) __attribute__((format(printf, 4, 5)));

bool display_spi_is_busy(void);

extern const unsigned char font3x5[];
extern const unsigned char font5x8[];
extern const unsigned char font16x24[];
extern const unsigned char fontSTD_swiss721_outline[];
extern const unsigned char font8x12[];
extern const unsigned char font16x16[];
extern const unsigned char font8x16[];

#endif // DISPLAY_H__