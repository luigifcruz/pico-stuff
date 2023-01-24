#ifndef BMP390_H
#define BMP390_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define BMP_RESET_REG           0x7E
#define BMP_RESET_VAL           0xB6
#define BMP_SENSOR_ID_REG       0x00
#define BMP_SENSOR_ID_VAL       0x60
#define BMP_OSR_REG             0x1C
#define BMP_CAL_REG             0x31
#define BMP_CAL_LEN             0x15
#define BMP_TEMPERATURE_REG     0x07
#define BMP_PRESSURE_REG        0x04
#define BMP_MODE_REG            0x1B
#define BMP_MODE_VAL            0x13  // Forced Mode, Temp Enable, Pressure Enable
#define BMP_STATUS_REG          0x03

#define ASSERT_OK(X) { if (X == false) return false; };

typedef struct {
    uint16_t    T1;
    uint16_t    T2;
    int8_t      T3;
    int16_t     P1;
    int16_t     P2;
    int8_t      P3;
    int8_t      P4; 
    uint16_t    P5;
    uint16_t    P6;
    int8_t      P7;
    int8_t      P8;
    int16_t     P9; 
    int8_t      P10; 
    int8_t      P11; 
} bmp_calib_param_t;

typedef struct {
    double      T1;
    double      T2;
    double      T3;
    double      P1;
    double      P2;
    double      P3;
    double      P4; 
    double      P5;
    double      P6;
    double      P7;
    double      P8;
    double      P9; 
    double      P10; 
    double      P11; 
} bmp_calib_part_param_t;

typedef struct {
    int addr;
    int rate;
    int scl;
    int sda;
    i2c_inst_t* inst;
} i2c_t;

typedef struct {
    i2c_t i2c;
    uint8_t oss;
    float temperature;
    float pressure;
    float altitude;
    bmp_calib_param_t calib;
    bmp_calib_part_param_t calib_part;
} bmp_t;

bool bmp_reset(bmp_t* bmp) {
    uint8_t data_buffer[] = {
        BMP_RESET_REG, 
        BMP_RESET_VAL
    };

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, data_buffer, 2, false);

#ifdef DEBUG
    printf("INFO: Successfully reset sensor.\n");
#endif

    sleep_ms(10);
    
    return true;
}

bool bmp_check_chip_id(bmp_t* bmp) {
    uint8_t chip_id_reg = BMP_SENSOR_ID_REG;
    uint8_t chip_id_val[1];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &chip_id_reg, 1, true);
    i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, chip_id_val, 1, false);

    if (chip_id_val[0] != BMP_SENSOR_ID_VAL) {
#ifdef DEBUG
        printf("Returned Chip ID: 0x%02x\n", chip_id_val[0]);
        printf("Check your I2C configuration and connection.\n");
#endif
        return false;
    }

#ifdef DEBUG
    printf("INFO: Successfully checked the Chip ID.\n");
#endif

    return true;
}

bool bmp_set_oversampling_rate(bmp_t* bmp) {
    uint8_t data_buffer[] = {
        BMP_OSR_REG, 
        (bmp->oss << 3) | (bmp->oss << 0)
    };

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, data_buffer, 2, false);

#ifdef DEBUG
    printf("INFO: Successfully configured oversampling rate.\n");
#endif
    
    return true;
}
bool bmp_get_calib_coeffs(bmp_t* bmp) {
    uint8_t calib_coeffs_reg = BMP_CAL_REG;
    uint8_t calib_coeffs_val[BMP_CAL_LEN];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &calib_coeffs_reg, 1, true);
    i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, calib_coeffs_val, BMP_CAL_LEN, false);

    bmp->calib.T1 = (calib_coeffs_val[1] << 8) | calib_coeffs_val[0];
    bmp->calib.T2 = (calib_coeffs_val[3] << 8) | calib_coeffs_val[2];
    bmp->calib.T3 = calib_coeffs_val[4];
    bmp->calib.P1 = (calib_coeffs_val[6] << 8) | calib_coeffs_val[5];
    bmp->calib.P2 = (calib_coeffs_val[8] << 8) | calib_coeffs_val[7];
    bmp->calib.P3 = calib_coeffs_val[9];
    bmp->calib.P4 = calib_coeffs_val[10];
    bmp->calib.P5 = (calib_coeffs_val[12] << 8) | calib_coeffs_val[11];
    bmp->calib.P6 = (calib_coeffs_val[14] << 8) | calib_coeffs_val[13];
    bmp->calib.P7 = calib_coeffs_val[15];
    bmp->calib.P8 = calib_coeffs_val[16];
    bmp->calib.P9 = (calib_coeffs_val[18] << 8) | calib_coeffs_val[17];
    bmp->calib.P10 = calib_coeffs_val[19];
    bmp->calib.P11 = calib_coeffs_val[20];

    bmp->calib_part.T1 = (double)bmp->calib.T1 / pow(2, -8.0);
    bmp->calib_part.T2 = (double)bmp->calib.T2 / pow(2, 30.0);
    bmp->calib_part.T3 = (double)bmp->calib.T3 / pow(2, 48.0);

    bmp->calib_part.P1 = ((double)bmp->calib.P1 - pow(2, 14.0)) / pow(2, 20.0);
    bmp->calib_part.P2 = ((double)bmp->calib.P2 - pow(2, 14.0)) / pow(2, 29.0);
    bmp->calib_part.P3 = (double)bmp->calib.P3 / pow(2, 32.0);
    bmp->calib_part.P4 = (double)bmp->calib.P4 / pow(2, 37.0);
    bmp->calib_part.P5 = (double)bmp->calib.P5 / pow(2, -3.0);
    bmp->calib_part.P6 = (double)bmp->calib.P6 / pow(2, 6.0);
    bmp->calib_part.P7 = (double)bmp->calib.P7 / pow(2, 8.0);
    bmp->calib_part.P8 = (double)bmp->calib.P8 / pow(2, 15.0);
    bmp->calib_part.P9 = (double)bmp->calib.P9 / pow(2, 48.0);
    bmp->calib_part.P10 = (double)bmp->calib.P10 / pow(2, 48.0);
    bmp->calib_part.P11 = (double)bmp->calib.P11 / pow(2.0, 65.0);

#ifdef DEBUG
    printf("==== CALIBRATION COEFFS ====\n");
    printf("T1: %lf\n", bmp->calib_part.T1);
    printf("T2: %lf\n", bmp->calib_part.T2);
    printf("T3: %lf\n", bmp->calib_part.T3);
    printf("P1: %lf\n", bmp->calib_part.P1);
    printf("P2: %lf\n", bmp->calib_part.P2);
    printf("P3: %lf\n", bmp->calib_part.P3);
    printf("P4: %lf\n", bmp->calib_part.P4);
    printf("P5: %lf\n", bmp->calib_part.P5);
    printf("P6: %lf\n", bmp->calib_part.P6);
    printf("P7: %lf\n", bmp->calib_part.P7);
    printf("P8: %lf\n", bmp->calib_part.P8);
    printf("P9: %lf\n", bmp->calib_part.P9);
    printf("P10: %lf\n", bmp->calib_part.P10);
    printf("P11: %lf\n", bmp->calib_part.P11);
    printf("============================\n");
#endif

    return true;
}

bool bmp_read_uncalibrated_temperature(bmp_t* bmp) {
    uint8_t temp_reg = BMP_TEMPERATURE_REG;
    uint8_t temp_val[3];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &temp_reg, 1, true);
    i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, temp_val, 3, false);

    bmp->temperature = (temp_val[2] << 16) | (temp_val[1] << 8) | temp_val[0];

    return true;
}

bool bmp_read_uncalibrated_pressure(bmp_t* bmp) {
    uint8_t pres_reg = BMP_PRESSURE_REG;
    uint8_t pres_val[3];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &pres_reg, 1, true);
    i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, pres_val, 3, false);

    bmp->pressure = (pres_val[2] << 16) | (pres_val[1] << 8) | pres_val[0];

    return true;

}

bool bmp_calibrate_temperature(bmp_t* bmp) {
    double par1 = bmp->temperature - bmp->calib_part.T1;
    double par2 = par1 * bmp->calib_part.T2;
    bmp->temperature = par2 + (par1 * par1) * bmp->calib_part.T3;

    return true;
}

bool bmp_calibrate_pressure(bmp_t* bmp) {
    double out1, out2, out3;

    {
        double par1 = bmp->calib_part.P6 * bmp->temperature;
        double par2 = bmp->calib_part.P7 * pow(bmp->temperature, 2.0);
        double par3 = bmp->calib_part.P8 * pow(bmp->temperature, 3.0);
        out1 = bmp->calib_part.P5 + par1 + par2 + par3;
    }

    {
        double par1 = bmp->calib_part.P2 * bmp->temperature;
        double par2 = bmp->calib_part.P3 * pow(bmp->temperature, 2.0);
        double par3 = bmp->calib_part.P4 * pow(bmp->temperature, 3.0);
        out2 = bmp->pressure * (bmp->calib_part.P1 + par1 + par2 + par3);
    }

    {
        double par1 = pow(bmp->pressure, 2.0);
        double par2 = bmp->calib_part.P9 + bmp->calib_part.P10 * bmp->temperature;
        double par3 = par1 * par2;
        out3 = par3 + bmp->calib_part.P11 * pow(bmp->pressure, 3);
    }

    bmp->pressure = (out1 + out2 + out3) / 100.0; 

    return true;
}

bool bmp_calculate_altitude(bmp_t* bmp) {
    bmp->altitude = ((pow((1013.25 / bmp->pressure), (1/5.257)) - 1) * (bmp->temperature + 273.15)) / 0.0065;

    return true;
}

bool bmp_get_pressure_temperature(bmp_t* bmp) {
    bool res = true;
    
    {
        uint8_t data_buffer[] = {
            BMP_MODE_REG, 
            BMP_MODE_VAL
        };

        i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, data_buffer, 2, false);
    }
    
    {
        uint8_t status = 0x00;
        while ((status & 0x60) != 0x60) {
            uint8_t status_reg = BMP_STATUS_REG;
            i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &status_reg, 1, true);
            i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, &status, 1, false);
            sleep_ms(1);
        }
    }

    res &= bmp_read_uncalibrated_temperature(bmp);
    res &= bmp_read_uncalibrated_pressure(bmp);
    res &= bmp_calibrate_temperature(bmp);
    res &= bmp_calibrate_pressure(bmp);
    res &= bmp_calculate_altitude(bmp);

    return res;
}

bool bmp_init(bmp_t* bmp) {
    i2c_init(bmp->i2c.inst, bmp->i2c.rate);

    if (bmp->oss < 0 || bmp->oss > 5) {
#ifdef DEBUG
        printf("Invalid over-sampling rate (%d). Valid 0 to 5.\n", bmp->oss);
#endif
        return false;
    }

    gpio_set_function(bmp->i2c.scl, GPIO_FUNC_I2C);
    gpio_set_function(bmp->i2c.sda, GPIO_FUNC_I2C);
    gpio_pull_up(bmp->i2c.scl);
    gpio_pull_up(bmp->i2c.sda);

    sleep_ms(100);

    ASSERT_OK(bmp_reset(bmp));
    ASSERT_OK(bmp_check_chip_id(bmp));
    ASSERT_OK(bmp_set_oversampling_rate(bmp));
    ASSERT_OK(bmp_get_calib_coeffs(bmp));

    return true;
}

bool bmp_get_temperature(bmp_t* bmp) {
    return false;
}

bool bmp_get_pressure(bmp_t* bmp) {
    return false;
}

#endif
