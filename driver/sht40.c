/**
 * @author Jared Wolff (hello@jaredwolff.com)
 * @copyright Copyright Circuit Dojo LLC 2021
 */

#define DT_DRV_COMPAT sensirion_sht40

#include <math.h>
#include <device.h>
#include <drivers/i2c.h>
#include <drivers/sensor.h>

#include <sensirion_common.h>
#include "sht40.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(sht40, CONFIG_SENSOR_LOG_LEVEL);

struct sht40_data
{
    const struct device *i2c_dev;
    struct sensor_value temperature;
    struct sensor_value humidity;
    uint8_t raw_temp[3];
    uint8_t raw_humidity[3];
    bool use_raw_temp;
    bool use_raw_humidity;
};

static int sht40_sample_both(struct sht40_data *dat)
{

    int err = 0;

    uint8_t cmd[] = {SHT40_TEMP_HUM_H_PREC_CMD};
    uint8_t buf[6];

    /* Start the temperature/humidity measurement */
    err = i2c_write(dat->i2c_dev,
                    cmd, sizeof(cmd), DT_INST_REG_ADDR(0));
    if (err)
    {
        LOG_WRN("Unable to start temperature & humidity reading. Err: %i", err);
        return err;
    }

    /* Delay */
    k_sleep(K_MSEC(10));

    /* Read the data */
    err = i2c_read(dat->i2c_dev, buf, sizeof(buf), DT_INST_REG_ADDR(0));
    if (err)
    {
        LOG_WRN("Unable to read temperature & humidity. Err: %i", err);
        return err;
    }

    memcpy(dat->raw_temp, buf, sizeof(dat->raw_temp));

    /* Check crc for temp */
    uint8_t crc = sensirion_calc_crc(dat->raw_temp);
    if (crc != dat->raw_temp[2])
    {
        LOG_WRN("CRC error. CRC: %x", crc);
        return -EPROTO;
    }

    memcpy(dat->raw_humidity, buf + 3, sizeof(dat->raw_humidity));

    /* Check crc for humidity */
    crc = sensirion_calc_crc(dat->raw_humidity);
    if (crc != dat->raw_humidity[2])
    {
        LOG_WRN("CRC error. CRC: %x", crc);
        return -EPROTO;
    }

    /* Calculate the temperature */
    uint16_t temperature = (dat->raw_temp[0] << 8) + dat->raw_temp[1];
    double temperature_float = (temperature * 175) / pow(2, 16) - 45;

    /* Convert! */
    sensor_value_from_double(&dat->temperature, temperature_float);

    /* Calculate the humidity */
    uint16_t humidity = (dat->raw_humidity[0] << 8) + dat->raw_humidity[1];
    double humidity_float = humidity * 100 / pow(2, 16);

    /* Convert! */
    sensor_value_from_double(&dat->humidity, humidity_float);

    return 0;
}

static int sht40_sample_fetch(const struct device *dev,
                              enum sensor_channel chan)
{

    struct sht40_data *dat = (struct sht40_data *)dev->data;

    /* Get the data depending on which channel */
    switch (chan)
    {
    case SENSOR_CHAN_ALL:
    case SENSOR_CHAN_AMBIENT_TEMP:
    case SENSOR_CHAN_HUMIDITY:
        sht40_sample_both(dat);
        break;
    default:
        LOG_WRN("Invalid sensor_channel %i", chan);
        return -EINVAL;
    }

    return 0;
}

static int sht40_channel_get(const struct device *dev,
                             enum sensor_channel chan,
                             struct sensor_value *val)
{

    struct sht40_data *dat = (struct sht40_data *)dev->data;

    /* Clear value */
    memset(val, 0, sizeof(*val));

    /* Get the data depending on which channel */
    switch (chan)
    {
    case SENSOR_CHAN_AMBIENT_TEMP:

        if (dat->use_raw_temp)
        {
            memcpy(val, dat->raw_temp, sizeof(dat->raw_temp));
        }
        else
        {
            memcpy(val, &dat->temperature, sizeof(struct sensor_value));
        }

        break;
    case SENSOR_CHAN_HUMIDITY:

        if (dat->use_raw_humidity)
        {
            memcpy(val, dat->raw_humidity, sizeof(dat->raw_humidity));
        }
        else
        {
            memcpy(val, &dat->humidity, sizeof(struct sensor_value));
        }
        break;
    default:
        LOG_WRN("Invalid sensor_channel %i", chan);
        return -EINVAL;
    }

    return 0;
}

static int sht40_init(const struct device *dev)
{

    struct sht40_data *data = dev->data;

    data->i2c_dev = device_get_binding(DT_INST_BUS_LABEL(0));

    if (data->i2c_dev == NULL)
    {
        LOG_ERR("Unable to get I2C Master.");
        return -EINVAL;
    }

    /* Ensure use of normal values by default */
    data->use_raw_humidity = false;
    data->use_raw_temp = false;

    return 0;
}

static int sht40_attr_get(const struct device *dev,
                          enum sensor_channel chan,
                          enum sensor_attribute attr,
                          struct sensor_value *val)
{

    uint8_t *val_bytes = (uint8_t *)&val;
    struct sht40_data *data = dev->data;

    switch (chan)
    {
    case SENSOR_CHAN_AMBIENT_TEMP:
        val_bytes[0] = data->use_raw_temp ? 1 : 0;
        break;
    case SENSOR_CHAN_HUMIDITY:
        val_bytes[0] = data->use_raw_humidity ? 1 : 0;
        break;
    default:
        LOG_WRN("Unknown channel %i", chan);
        break;
    }

    return 0;
}

static int sht40_attr_set(const struct device *dev,
                          enum sensor_channel chan,
                          enum sensor_attribute attr,
                          const struct sensor_value *val)

{

    enum sht40_attribute sht40_attr = (enum sht40_attribute)attr;
    struct sht40_data *data = dev->data;

    switch (sht40_attr)
    {

    case SHT40_ATTR_USE_RAW:
    {

        switch (chan)
        {
        case SENSOR_CHAN_AMBIENT_TEMP:
            if (val->val1 > 0)
            {
                data->use_raw_temp = true;
            }
            else if (val->val1 == 0)
            {
                data->use_raw_temp = false;
            }
            break;
        case SENSOR_CHAN_HUMIDITY:
            if (val->val1 > 0)
            {
                data->use_raw_humidity = true;
            }
            else if (val->val1 == 0)
            {
                data->use_raw_humidity = false;
            }

            break;
        default:
            LOG_WRN("Unknown channel %i", chan);
            break;
        }
    }
    break;
    default:
        LOG_WRN("Unknown attr %i", sht40_attr);
        break;
    }

    return 0;
}

static const struct sensor_driver_api sht40_api = {
    .attr_set = &sht40_attr_set,
    .attr_get = &sht40_attr_get,
    .sample_fetch = &sht40_sample_fetch,
    .channel_get = &sht40_channel_get,
};

/* Main instantiation matcro */
#define SHT40_DEFINE(inst)                                       \
    static struct sht40_data sht40_data_##inst;                  \
    DEVICE_DT_INST_DEFINE(inst,                                  \
                          sht40_init, NULL,                      \
                          &sht40_data_##inst, NULL, POST_KERNEL, \
                          CONFIG_SENSOR_INIT_PRIORITY, &sht40_api);

/* Create the struct device for every status "okay"*/
DT_INST_FOREACH_STATUS_OKAY(SHT40_DEFINE)