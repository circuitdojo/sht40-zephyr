/**
 * @author Jared Wolff (hello@jaredwolff.com)
 * @copyright Copyright Circuit Dojo LLC 2021
 */

#ifndef _SHT40_H
#define _SHT40_H

#include <zephyr.h>
#include <drivers/sensor.h>

#define SHT40_TEMP_HUM_H_PREC_CMD 0xFD
#define SHT40_TEMP_HUM_M_PREC_CMD 0xF6
#define SHT40_TEMP_HUM_L_PREC_CMD 0xE0

#define SHT40_READ 0x89
#define SHT40_RESET 0x94

#define SHT40_TEMP_HUM_HP_1S_CMD 0x39
#define SHT40_TEMP_HUM_HP_0_1S_CMD 0x32

#define SHT40_TEMP_HUM_MP_1S_CMD 0x2F
#define SHT40_TEMP_HUM_MP_0_1S_CMD 0x24

#define SHT40_TEMP_HUM_LP_1S_CMD 0x1E
#define SHT40_TEMP_HUM_LP_0_1S_CMD 0x15

/* Additional custom attributes */
enum sht40_attribute
{
    SHT40_ATTR_USE_RAW = SENSOR_ATTR_PRIV_START,
};

#endif /*_SHT40_H*/