/**
 * failsafe_controller.c
 *
 * Multi-cog firmware for the Drone Failsafe Parachute System.
 * Runs on Parallax Propeller (P8X32A) using SimpleIDE toolchain.
 *
 * Architecture:
 *   Cog 0 (Main)  — Supervisor loop: reads shared flags, actuates servos
 *   Cog 1          — Ultrasonic ranging: continuous ground-proximity measurement
 *   Cog 2          — IMU monitor: Z-axis jerk detection for freefall/failure events
 *
 * Hardware:
 *   P0/P1  — I2C bus (MPU6050)
 *   P2     — Parachute release servo (PWM)
 *   P3     — HC-SR04 ultrasonic trigger/echo (GPIO)
 *   P4     — Landing gear deploy servo (PWM)
 */

#include "simpletools.h"
#include "servo.h"
#include "simplei2c.h"

/* ── Pin Assignments ─────────────────────────────────────────────────── */
#define PIN_I2C_SDA          0
#define PIN_I2C_SCL          1
#define PIN_SERVO_PARACHUTE  2
#define PIN_ULTRASONIC       3
#define PIN_SERVO_GEAR       4

/* ── MPU6050 Registers ───────────────────────────────────────────────── */
#define MPU6050_ADDR         0x68
#define REG_ACCEL_ZOUT_H     0x3F
#define REG_PWR_MGMT_1       0x6B

/* ── Thresholds ──────────────────────────────────────────────────────── */
#define JERK_THRESHOLD       9000   // Raw ADC delta triggering parachute deploy
#define GEAR_DEPLOY_CM       35     // Ground proximity threshold for landing gear

/* ── Servo Positions ─────────────────────────────────────────────────── */
#define PARACHUTE_ARMED      90
#define PARACHUTE_DEPLOYED   180
#define GEAR_STOWED          0
#define GEAR_DEPLOYED        1100

/* ── Shared State (volatile: accessed across cogs) ───────────────────── */
volatile int ground_distance_cm = 0;
volatile int jerk_detected      = 0;

/* ── Private State ───────────────────────────────────────────────────── */
static i2c *imu_bus;
static int  prev_accel_z  = 0;
static int  gear_deployed = 0;

/* ── Forward Declarations ────────────────────────────────────────────── */
void ultrasonic_ranging_task(void);
void imu_monitor_task(void);

/* ── MPU6050 Driver ──────────────────────────────────────────────────── */

static void mpu6050_write_reg(unsigned char reg, unsigned char value) {
    i2c_out(imu_bus, MPU6050_ADDR, reg, 1, &value, 1);
}

static int mpu6050_read_word(unsigned char reg) {
    unsigned char buf[2];
    i2c_in(imu_bus, MPU6050_ADDR, reg, 1, buf, 2);
    return (int)((buf[0] << 8) | buf[1]);
}

static void mpu6050_init(void) {
    mpu6050_write_reg(REG_PWR_MGMT_1, 0x00);
    pause(100);
}

/* ── Main Supervisor (Cog 0) ─────────────────────────────────────────── */

int main(void) {
    servo_angle(PIN_SERVO_GEAR, GEAR_STOWED);
    servo_angle(PIN_SERVO_PARACHUTE, PARACHUTE_ARMED);
    pause(1000);

    cog_run(ultrasonic_ranging_task, 128);

    imu_bus = i2c_newbus(PIN_I2C_SDA, PIN_I2C_SCL, 0);
    mpu6050_init();
    prev_accel_z = -mpu6050_read_word(REG_ACCEL_ZOUT_H);

    cog_run(imu_monitor_task, 128);

    while (1) {
        if (ground_distance_cm > 0 && ground_distance_cm <= GEAR_DEPLOY_CM && !gear_deployed) {
            servo_angle(PIN_SERVO_GEAR, GEAR_DEPLOYED);
            gear_deployed = 1;
        }

        if (jerk_detected) {
            servo_angle(PIN_SERVO_PARACHUTE, PARACHUTE_DEPLOYED);
            jerk_detected = 0;
        }

        pause(100);
    }

    return 0;
}

/* ── Ultrasonic Ranging Task (Cog 1) ─────────────────────────────────── */

void ultrasonic_ranging_task(void) {
    while (1) {
        low(PIN_ULTRASONIC);
        pause(2);
        high(PIN_ULTRASONIC);
        pause(10);
        low(PIN_ULTRASONIC);

        long echo_us = pulse_in(PIN_ULTRASONIC, 1);
        ground_distance_cm = echo_us / 58;

        pause(100);
    }
}

/* ── IMU Monitor Task (Cog 2) ────────────────────────────────────────── */

void imu_monitor_task(void) {
    while (1) {
        int accel_z = -mpu6050_read_word(REG_ACCEL_ZOUT_H);
        int jerk = accel_z - prev_accel_z;

        if (jerk > JERK_THRESHOLD) {
            jerk_detected = 1;
        }

        prev_accel_z = accel_z;
        pause(50);
    }
}
