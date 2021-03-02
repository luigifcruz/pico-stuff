#ifndef BMP180_H
#define BMP180_H

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define BMP_REG_CONTROL         0xF4
#define BMP_REG_RESULT          0xF6
#define BMP_COM_TEMP            0x2E
#define BMP_COM_PRES            0x34
#define BMP_CALIB_COEFF_LEN     0x16
#define BMP_CALIB_COEFF_REG     0xAA
#define BMP_CHIP_ID_REG         0xD0
#define BMP_CHIP_ID_VAL         0x55
#define BMP_MIN_TEMP_THRESHOLD	-40
#define BMP_MAX_TEMP_THRESHOLD	+85
#define BMP_TEMP_DELAY          5

#define ASSERT_OK(X) { if (X == false) return false; };

typedef struct {
	int16_t	 AC1;
	int16_t  AC2;
	int16_t  AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	int16_t  B1;
	int16_t  B2;
	int16_t  MB;
	int16_t  MC;
	int16_t  MD;
} bmp_calib_param_t;

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
    int32_t pressure;
    bmp_calib_param_t calib;
    int32_t B5;
} bmp_t;

const uint32_t oss_delay[] = {5, 8, 14, 26};

bool bmp_check_chip_id(bmp_t* bmp) {
    uint8_t chip_id_reg = BMP_CHIP_ID_REG;
    uint8_t chip_id_val[1];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &chip_id_reg, 1, true);
    i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, chip_id_val, 1, false);

    if (chip_id_val[0] != BMP_CHIP_ID_VAL) {
#ifdef DEBUG
        printf("Returned Chip ID: 0x%02x\n", chip_id_val[0]);
        printf("Check your I2C configuration and connection.\n");
#endif
        return false;
    }

    return true;
}

bool bmp_get_calib_coeffs(bmp_t* bmp) {
    uint8_t calib_coeffs_reg = BMP_CALIB_COEFF_REG;
    uint8_t calib_coeffs_val[BMP_CALIB_COEFF_LEN];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &calib_coeffs_reg, 1, true);
    i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, calib_coeffs_val, BMP_CALIB_COEFF_LEN, false);

    int16_t* data = (int16_t*)&bmp->calib;
    for (int i = 0, j = 1; i < BMP_CALIB_COEFF_LEN / 2; i++, j += 2) {
        data[i] = (calib_coeffs_val[i * 2] << 8) | calib_coeffs_val[j];

        if ((data[i] == 0) | (data[i] == -1)) {
#ifdef DEBUG
            printf("Calibation data invalid.\n");
#endif
            return false;
        }
    }

#ifdef DEBUG
    printf("==== CALIBRATION COEFFS ====\n");
    printf("AC1: %d\n", bmp->calib.AC1);
    printf("AC2: %d\n", bmp->calib.AC2);
    printf("AC3: %d\n", bmp->calib.AC3);
    printf("AC4: %d\n", bmp->calib.AC4);
    printf("AC5: %d\n", bmp->calib.AC5);
    printf("AC6: %d\n", bmp->calib.AC6);
    printf("B1: %d\n", bmp->calib.B1);
    printf("B2: %d\n", bmp->calib.B2);
    printf("MB: %d\n", bmp->calib.MB);
    printf("MC: %d\n", bmp->calib.MC);
    printf("MD: %d\n", bmp->calib.MD);
    printf("============================\n");
#endif

    return true;
}

uint32_t bmp_start_temperature(bmp_t* bmp) {
    uint8_t temp_reg[] = {
        BMP_REG_CONTROL,
        BMP_COM_TEMP
    };
    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, temp_reg, 2, false);
    return BMP_TEMP_DELAY;
}

bool bmp_read_temperature(bmp_t* bmp) {
    uint8_t temp_reg = BMP_REG_RESULT;
    uint8_t temp_val[2];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &temp_reg, 1, true);
    if (i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, temp_val, 2, false) != 2) {
#ifdef DEBUG
        printf("Wrong read length.\n");
#endif
        return false;
    }

    int16_t UT = (temp_val[0] << 8) + temp_val[1];

    if (UT == (int16_t)0x8000) {
#ifdef DEBUG
        printf("Non-initialized register detected.\n");
#endif
        return false;
    }

    int32_t X1 = (((int32_t)UT - bmp->calib.AC6) * bmp->calib.AC5) >> 15;
    int32_t X2 = (bmp->calib.MC << 11) / (X1 + bmp->calib.MD);
    bmp->B5 = X1 + X2;
    float temp = ((bmp->B5 + 8) >> 4) * 0.1f;

    if ((temp <= BMP_MIN_TEMP_THRESHOLD) || (temp >= BMP_MAX_TEMP_THRESHOLD)) {
#ifdef DEBUG
        printf("Temperature beyond threshold: %f\n", temp);
#endif
        return false;
	}

    bmp->temperature = temp;

    return true;
}

uint32_t bmp_start_pressure(bmp_t* bmp) {
    uint8_t pres_reg[] = {
        BMP_REG_CONTROL,
        BMP_COM_PRES + (bmp->oss << 6)
    };
    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, pres_reg, 2, false);
    return oss_delay[bmp->oss];
}

bool bmp_read_pressure(bmp_t* bmp) {
    uint8_t pres_reg = BMP_REG_RESULT;
    uint8_t pres_val[3];

    i2c_write_blocking(bmp->i2c.inst, bmp->i2c.addr, &pres_reg, 1, true);
    if (i2c_read_blocking(bmp->i2c.inst, bmp->i2c.addr, pres_val, 3, false) != 3) {
#ifdef DEBUG
        printf("Wrong read length.\n");
#endif
        return false;
    }

    int32_t UP = ((pres_val[0] << 16) + (pres_val[1] << 8) + pres_val[2]) >> (8 - bmp->oss);

    int32_t X1, X2, X3, B3, B6, p = 0;
	uint32_t B4, B7 = 0;

    B6 = bmp->B5 - 4000;
    X1 = (bmp->calib.B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = (bmp->calib.AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = (((bmp->calib.AC1 * 4 + X3) << bmp->oss) + 2) / 4;
    X1 = (bmp->calib.AC3 * B6) >> 13;
    X2 = (bmp->calib.B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = (bmp->calib.AC4 * (uint32_t)(X3 + 32768)) >> 15;
    B7 = ((uint32_t)(UP) - B3) * (50000 >> bmp->oss);

    if (B7 < 0x80000000) {
        p = (B7 * 2) / B4;
    } else {
        p = (B7 / B4) * 2;
    }

    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;

    bmp->pressure = p + ((X1 + X2 + 3791) >> 4);

    return true;
}

bool bmp_get_temperature(bmp_t* bmp) {
    sleep_ms(bmp_start_temperature(bmp));
    return bmp_read_temperature(bmp);
}

// User must call bmp_get_temperature() before calling this method.
// Or use the combo bmp_get_pressure_temperature().
bool bmp_get_pressure(bmp_t* bmp) {
    sleep_ms(bmp_start_pressure(bmp));
    return bmp_read_pressure(bmp);
}

bool bmp_get_pressure_temperature(bmp_t* bmp) {
    bool res = true;
    res &= bmp_get_temperature(bmp);
    res &= bmp_get_pressure(bmp);
    return res;
}

bool bmp_init(bmp_t* bmp) {
    i2c_init(bmp->i2c.inst, bmp->i2c.rate);

    if (bmp->oss < 0 || bmp->oss > 3) {
#ifdef DEBUG
        printf("Invalid over-sampling rate (%d). Valid 0 to 3.\n", bmp->oss);
#endif
        return false;
    }

    gpio_set_function(bmp->i2c.scl, GPIO_FUNC_I2C);
    gpio_set_function(bmp->i2c.sda, GPIO_FUNC_I2C);
    gpio_pull_up(bmp->i2c.scl);
    gpio_pull_up(bmp->i2c.sda);

    sleep_ms(100);

    ASSERT_OK(bmp_check_chip_id(bmp));
    ASSERT_OK(bmp_get_calib_coeffs(bmp));

    return true;
}

#endif