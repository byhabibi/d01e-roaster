#pragma once

// D01E — GPIO map v3.3 (D01E_Electrical_System_Design.md)
#define FW_VERSION_BCD    0x0101   // v1.01 — HMI dedup + auto-cool UI fix

// Nextion UART2 @ 115200 — GPIO33 TX, GPIO32 RX
#define NX_SERIAL         Serial2
#define NX_TX_PIN         33
#define NX_RX_PIN         32
#define NX_BAUD           115200

// MAX31856 SPI
#define TC_SPI_SCK        18
#define TC_SPI_MISO       19
#define TC_SPI_MOSI       23
#define TC_CS_BEAN        5
#define TC_CS_ENV         15

// Roast screen object names — page2 + page16..27 (layout identik)
#define NX_PAGE_ROAST         2
#define NX_PAGE_ROAST_FIRST   16
#define NX_PAGE_ROAST_LAST    27

#define NX_OBJ_TIMER      "t18"  // MM:SS
#define NX_OBJ_BT         "t19"  // Bean temp
#define NX_OBJ_ET         "t20"  // Environment temp
#define NX_OBJ_BT_ROR     "t21"  // Delta bean RoR
#define NX_OBJ_ET_ROR     "t22"  // Delta ET RoR
#define NX_OBJ_COOL_FAN   "bt8"  // Cooling fan dual-state
#define NX_OBJ_COLOR      "bt7"  // Color sensor dual-state

// Auto roast status bar (page16–27) — satu aktif; banner via p3.pic
#define NX_OBJ_STATUS_COOL    "bt2"
#define NX_OBJ_STATUS_WARM    "bt3"
#define NX_OBJ_STATUS_CHARGE  "bt4"
#define NX_OBJ_STATUS_ROAST   "bt5"
#define NX_OBJ_STATUS_DONE    "bt6"
#define NX_OBJ_STATUS_BANNER  "p3"
#define NX_PIC_BANNER_IDLE    165
#define NX_PIC_BANNER_COOL     210
#define NX_PIC_BANNER_WARM     214
#define NX_PIC_BANNER_CHARGE   213
#define NX_PIC_BANNER_ROAST    212
#define NX_PIC_BANNER_DONE     211
#define NX_OBJ_TM_BLINK_CHARGE "tm0"  // kedip banner INPUT BEAN (pic 213)
#define NX_OBJ_TM_BLINK_DONE   "tm1"  // kedip banner ROASTING DONE (pic 211)

// Panel latching buttons — J_H1 → opto U7/U8/U9 (D01E_Electrical §9)
#define PIN_BTN_DRUM          25
#define PIN_BTN_HEATER        26
#define PIN_BTN_AIR           27
#define PIN_DRUM_EN           12
#define PIN_ESTOP_SENSE       14
#define PANEL_BTN_ACTIVE_LOW  1      // opto ON → GPIO LOW
#define PANEL_ESTOP_ENABLED   1
#define PANEL_ESTOP_UNSAFE_HIGH 1    // E-Stop NC open → GPIO HIGH

// Pot Air ADC (GPIO34) + exhaust PWM (GPIO2 → IDC pin 8)
#define PIN_POT_AIR           34
#define PIN_EXHAUST_PWM       2
#define AIR_LEDC_CHANNEL      0
#define AIR_LEDC_FREQ_HZ      25000UL
#define AIR_LEDC_RES_BITS     8
#define AIR_POLL_MS           100UL
#define AIR_ADC_SAMPLES       8
// Kalibrasi ujung pot 10kΩ @ 3.3V — sesuaikan setelah uji hardware
#define POT_ADC_MIN           150
#define POT_ADC_MAX           3950

// Waveform s0 — pakai nama objek (id numerik beda tiap page)
#define NX_WAVE_OBJ       "s0"
#define WAVE_CH_BT        0
#define WAVE_CH_ET        1
#define WAVE_CH_ROR_BT    2
#define PLOT_TEMP_MAX_C   250    // Y axis 0..250°C (t0..t5)
#define PLOT_TEMP_MAX_F   482    // 250°C → °F
#define PLOT_ROR_MAX      30     // RoR channel 0..30°C/min
#define PLOT_ROR_MAX_F    54     // 30°C/min → °F/min

// Touch buttons page 2 — HMI Anda: b1=START, b2=STOP, b3=RESET
// (firmware terima perintah via printh A5 01/02/03, bukan nama tombol)
#define NX_BTN_START      "b1"
#define NX_BTN_STOP       "b2"
#define NX_BTN_RESET      "b3"

// Roast loop
#define ROAST_TICK_MS     1000UL   // 1 Hz UI + timer
#define ROR_WINDOW_MS     15000UL  // RoR over 15 s window (Artisan-like)

// Custom serial bytes from Nextion Touch Release (printh A5 xx FF FF FF)
#define NX_CMD_MAGIC      0xA5
#define NX_CMD_START      0x01
#define NX_CMD_STOP       0x02
#define NX_CMD_RESET      0x03
#define NX_CMD_HEAT_SET   0x04   // byte2 = heatPct 0..100
#define NX_CMD_COOL_SET   0x05   // byte2 = bt8.val 0=OFF 1=ON
#define NX_CMD_COLOR_SET  0x06   // byte2 = bt7.val 0=OFF 1=ON
#define NX_CMD_PAGE_LEAVE 0x07   // keluar roast page → semua output OFF
#define NX_CMD_PAGE_ENTER 0x08   // masuk roast page → kontrol diizinkan
#define NX_CMD_WARMUP_ENTER 0x09 // masuk page4 → warming up otomatis
#define NX_CMD_WARMUP_LEAVE 0x0A // keluar page4 → heater/air OFF
#define NX_CMD_TEMP_LIMIT_SAVE 0x0B // byte2-3 = limit °C uint16 BE
#define NX_CMD_LIMIT_PAGE_ENTER 0x0C // masuk page5 → ESP32 kirim t0 dari NVS
#define NX_CMD_UNIT_SET       0x0D   // byte2 = 0 Celsius, 1 Fahrenheit
#define NX_CMD_SETTINGS_ENTER 0x0E   // masuk page6 → ESP32 kirim batch + unit
#define NX_CMD_AUTO_ENTER     0x0F   // byte2 = page 16..27 (auto roast profil)

// Auto roast — charge BT & akhir profil (user spec 2026-07-12)
#define AUTO_CHARGE_WARN_C        5      // °C sebelum charge → audio
#define AUTO_DROP_WARN_SEC        30UL   // detik sebelum selesai → audio + cooling
#define AUTO_AUDIO_INTERVAL_MS    3000UL
#define DF_TRACK_PREPARE_CHARGE   2      // 0002.mp3
#define DF_TRACK_PREPARE_DROP     3      // 0003.mp3

// Page 6 — settings (global)
#define NX_PAGE_SETTINGS      6
#define NX_OBJ_BATCH          "t0"
#define NX_OBJ_UNIT_C         "c0"
#define NX_OBJ_UNIT_F         "c1"
#define SETTINGS_NVS_NS       "d01e"
#define SETTINGS_NVS_KEY_UNIT_F "useF"
#define SETTINGS_NVS_KEY_BATCH  "batch"
#define BATCH_MIN_SEC         300UL    // 5 menit roast = 1 batch

// Y-axis labels t0..t5 (°C grid → convert to °F when unit=F)
#define NX_OBJ_Y0             "t0"
#define NX_OBJ_Y1             "t1"
#define NX_OBJ_Y2             "t2"
#define NX_OBJ_Y3             "t3"
#define NX_OBJ_Y4             "t4"
#define NX_OBJ_Y5             "t5"

#define NX_PAGE_LIMIT         5
#define NX_OBJ_LIMIT          "t0"
#define TEMP_LIMIT_MIN_C      150
#define TEMP_LIMIT_MAX_C      320
#define TEMP_LIMIT_DEFAULT_C  300
#define TEMP_LIMIT_STEP_C     10
#define TEMP_LIMIT_NVS_NS     "d01e"
#define TEMP_LIMIT_NVS_KEY    "tLimit"

#define NX_PAGE_WARMUP        4
#define NX_OBJ_WU_TIMER       "t0"
#define NX_OBJ_WU_BT          "t1"
#define WARMUP_TARGET_C       180
#define WARMUP_NEAR_C         175
#define WARMUP_HEATER_PCT     100
#define WARMUP_AIR_PCT        60
#define WARMUP_AUDIO_INTERVAL_MS 3000UL

// DFPlayer Mini — one-wire TX GPIO4 @ 9600 (U13) — SoftwareSerial (UART1 = RS485)
#define DF_TX_PIN             4
#define DF_BAUD               9600
#define DF_TRACK_WARMUP_NEAR  1    // 0001.mp3 — voice: en-US-JennyNeural (see D01E_HMI_DEVNOTE §7.4)
#define DF_VOLUME             25   // 0..30
#define DF_VOICE_STANDARD     "en-US-JennyNeural"

// RS485 Modbus RTU — UART1 GPIO16 RX / GPIO17 TX @ 9600 (U6 MAX3485)
#define MB_SERIAL             Serial1
#define MB_RX_PIN             16
#define MB_TX_PIN             17
#define MB_BAUD               9600
#define MB_SLAVE_ID           19
#define MODBUS_FRAME_GAP_MS   5

// PCF8574 @ 0x20 — P2 RS485 DE, P3 cooling → IDC pin 7 → Q7 → Q3 (P3 LOW = ON)
#define PIN_I2C_SDA           21
#define PIN_I2C_SCL           22
#define PCF8574_ADDR          0x20
#define PCF8574_PIN_RS485_DE  2
#define PCF8574_PIN_COOL      3
#define TCS34725_I2C_ADDR     0x29   // J_COLOR header (optional module)

// --- Heater output (see D01E_Heater_Control_v3.6.md) ---
// v3.5: GPIO13 drives Q_K1 (K1 coil + SSR together) — burst NOT recommended
// v3.6: GPIO13 = SSR_BURST only; PCF8574 P0 = K1_HOLD
#define PIN_HEATER_EN     13     // v3.5 name HEATER_EN / v3.6 SSR_BURST
#define HEAT_CYCLE_MS     10000UL
#define HEAT_MIN_ON_MS    1000UL
// #define PCF8574_PIN_K1_HOLD  0   // v3.6 heater only
