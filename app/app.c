#include "config_system.h"
#include "display.h"
#include "fw_header.h"
#include "main.h"
#include "platform.h"
#include "ret_mem.h"
#include "usbd_device.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern ADC_HandleTypeDef hadc1;

uint32_t adc_val[2];

enum
{
	AXIS_OFF = 0x06, // OFF
	AXIS_x = 0x11,	 // X
	AXIS_y = 0x12,	 // Y
	AXIS_z = 0x13,	 // Z
	AXIS_a = 0x14,	 // A
	AXIS_b = 0x15,	 // B
	AXIS_c = 0x16,	 // C
};

enum
{
	FEED_2 = 0x0d,	 // 2% - 0.01mm
	FEED_5 = 0x0e,	 // 5%
	FEED_10 = 0x0f,	 // 10% - 0.1mm
	FEED_30 = 0x10,	 // 30%
	FEED_60 = 0x1a,	 // 60%
	FEED_100 = 0x1b, // 100%
};

enum
{
	BTN_reset = 0x01,		   // reset				macro-11
	BTN_stop = 0x02,		   // stop				macro-12
	BTN_start = 0x03,		   // start-pause		macro-13
	BTN_feed_plus = 0x04,	   // feed-plus			macro-1
	BTN_feed_minus = 0x05,	   // feed-minus		macro-2
	BTN_spindle_plus = 0x06,   // spindle-plus		macro-3
	BTN_spindle_minus = 0x07,  // spindle-minus		macro-4
	BTN_machine_home = 0x08,   // m-home			macro-5
	BTN_safe_z = 0x09,		   // safe-z			macro-6
	BTN_workpiece_home = 0x0a, // w-home			macro-7
	BTN_spindle_on_off = 0x0b, // s-on-off			macro-8
	BTN_function = 0x0c,	   // fn				<unused>
	BTN_probe_z = 0x0d,		   // probe-z			macro-9
	BTN_macro10 = 0x10,		   // macro-10			macro-14
	BTN_continuous = 0x0e,	   // mode-continuous	macro-15
	BTN_step = 0x0f,		   // mode-step			macro-16
};

enum
{
	STEP_MODE_CON = 0, // "CON: <xxx>%": feed rotary button: CON:2%, CON:5%, CON:10%, CON:30%, CON:60%, CON:100%
	STEP_MODE_STEP,	   // "STP: <x.xxxx>": feed rotary button: STP:0.001, STP:0.01, STP:0.1, STP:1.0 | On 60%, 100% or Lead still displays "STP: 1.0" (firmware bug).
	STEP_MODE_MPG,	   // "MPG: <xxx>%": feed rotary button:  MPG:2%, MPG:5%, MPG:10%, MPG:30%, MPG:60%, MPG:100%
	STEP_MODE_PERCENT, // <xxx%>: feed rotary button: 2%, 5%, 10%, 30%, 60%, 100%v
};

static struct
{
	uint8_t header;
	uint8_t randomByte;
	uint8_t keyCode;
	uint8_t modifierCode;
	uint8_t rotaryButtonFeedKeyCode;
	uint8_t rotaryButtonAxisKeyCode;
	int8_t stepCount;
	uint8_t crc;
} hid_report = {0x04, 0, 0, 0, FEED_2, AXIS_OFF, 0, 0xFF};

bool g_stay_in_boot = false;
uint32_t g_uid[3];

uint8_t test = 0;
config_entry_t g_device_config[] = {
	{"test", sizeof(test), 0, &test},
};
const uint32_t g_device_config_count = sizeof(g_device_config) / sizeof(g_device_config[0]);

static struct __PACKED
{
	uint16_t header;
	uint8_t seed;
	struct
	{
		uint8_t step_mode : 2;
		uint8_t unknown : 4;
		uint8_t is_reset : 1;	  // if flag set displays "RESET", step_mode otherwise
		uint8_t is_rel_coord : 1; // if flag set axis names are "X1" "X1" ... "C1", "X" "Y" ... "C" otherwise
	} disp_flags;
	struct
	{
		uint16_t integer_val;
		uint16_t fract_val : 15;
		uint16_t coord_sign : 1;
	} axis[3];
	uint16_t feedrate;		   // on feed +/- button pressed shown on display
	uint16_t spindle_feedrate; // on spindle +/- button pressed shown on display
	uint8_t padding;
} hid_req = {0}, hid_req_prev = {0};

void init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;

	platform_get_uid(g_uid);
	// platform_watchdog_init();

	fw_header_check_all();

	ret_mem_init();
	ret_mem_set_load_src(LOAD_SRC_APP); // let preboot know it was booted from bootloader

	TIM1->CR1 |= TIM_CR1_CEN;

	hid_req.seed = hid_req_prev.seed = 0xFE;

	HAL_ADC_Start_DMA(&hadc1, adc_val, 2);

	display_init();
	display_set_font(font3x5);
	display_line_h(0, 0, 128, 1);
	display_line_h(0, 63, 128, 1);
	display_line_v(0, 0, 64, 1);
	display_line_v(127, 0, 64, 1);
	display_set_update_pending();
}

uint32_t init_seq = 0;

void loop(void)
{
	static uint32_t tick_ms_prev = 0;
	uint32_t diff_ms = (HAL_GetTick() - tick_ms_prev);
	if(diff_ms > 0x7FFFFFFF) diff_ms = 0xFFFFFFFF - tick_ms_prev + HAL_GetTick();
	tick_ms_prev = HAL_GetTick();

	static const uint8_t rot_feed_lookup[] = {FEED_2, FEED_5, FEED_10, FEED_30, FEED_60, FEED_100};
	static const uint8_t rot_axis_lookup[] = {AXIS_OFF, AXIS_x, AXIS_y, AXIS_z, AXIS_a, AXIS_b, AXIS_c};

	static uint32_t prev_tim = 0;
	{ // report update
		hid_report.keyCode = 0;
		// if(init_seq)
		{
			hid_report.keyCode = BTN_step;
			init_seq--;
		}
		hid_report.stepCount = (int32_t)(TIM1->CNT - prev_tim);
		hid_report.rotaryButtonFeedKeyCode = rot_feed_lookup[(5 * adc_val[0] + 2048) >> 12];
		hid_report.rotaryButtonAxisKeyCode = rot_axis_lookup[(6 * adc_val[1] + 2048) >> 12];
		hid_report.crc = hid_report.keyCode
							 ? hid_report.randomByte - (hid_report.keyCode ^ (hid_report.randomByte & hid_req.seed))
							 : hid_report.randomByte & hid_req.seed;
	}
	if(usbd_pend_hid_send_report(&hUsbDeviceFS, (uint8_t *)&hid_report, sizeof(hid_report)) == 0) prev_tim = TIM1->CNT;
	if(display_is_update_pending()) display_update();
}

static void parse_req(bool updated)
{
	if(updated)
	{
		display_buffer_clear();

		display_set_font(font5x8);
		display_print(0, 0, false, "Z %c%d.%04d ", hid_req.axis[2].coord_sign ? '-' : '+', hid_req.axis[2].integer_val, hid_req.axis[2].fract_val);
		display_print(0, 12, false, "Y %c%d.%04d ", hid_req.axis[1].coord_sign ? '-' : '+', hid_req.axis[1].integer_val, hid_req.axis[1].fract_val);
		display_print(0, 24, false, "X %c%d.%04d ", hid_req.axis[0].coord_sign ? '-' : '+', hid_req.axis[0].integer_val, hid_req.axis[0].fract_val);
		display_print(0, 36, false, "FD: %d %d ", hid_req.feedrate, hid_req.spindle_feedrate);
		display_print(0, 48, false, "R %d REL %d STP %d ", hid_req.disp_flags.is_reset, hid_req.disp_flags.is_rel_coord, hid_req.disp_flags.step_mode);
		init_seq = 20;
	}
	static uint8_t upd_cnt = 0;
	display_set_font(font3x5);
	display_print(127 - 10, 63 - 4, false, "%03d", upd_cnt++);
	display_set_update_pending();
}

void pend_hid_parse_buf(const uint8_t *buf, uint32_t size)
{
	if(size != 8) return;
	static uint32_t msg_it = 0;
	if(buf[1] == 0xfe && buf[2] == 0xfd) msg_it = 0;
	if(msg_it < 3)
	{
		memcpy(&((uint8_t *)&hid_req)[msg_it * 7], buf + 1, 7);
		if(++msg_it == 3)
		{
			parse_req(memcmp(&hid_req, &hid_req_prev, sizeof(hid_req)) != 0);
			hid_req_prev = hid_req;
		}
	}
}

enum
{
	__check_hid_report_size = 1 / ((sizeof(hid_report) == USBD_PEND_HID_EPIN_SIZE) ? 1 : 0),
	__check_hid_req_size = 1 / ((sizeof(hid_req) == 7 * 3) ? 1 : 0),
};