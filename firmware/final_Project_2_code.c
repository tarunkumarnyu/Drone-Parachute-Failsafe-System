#include "simpletools.h"
#include "servo.h"
#include "simplei2c.h"

// **Ultrasonic Sensor Definitions**
#define ULTRASONIC_PIN  3   
#define SERVO1_PIN      4   

volatile int distance_cm = 0;  
int cog_ultrasonic;            
int servo1_triggered = 0;       

void measure_distance(); // Function prototype

// **MPU6050 Sensor Definitions**
#define MPU6050_ADDR 0x68
#define ACCEL_ZOUT_H 0x3F
#define PWR_MGMT_1   0x6B

#define SERVO2_PIN 2  
#define JERK_THRESHOLD 9000  

i2c *imu;
int prev_az = 0;  
int cog_imu;
volatile int jerk_detected = 0;  

void mpu6050_write(unsigned char reg, unsigned char data) {
    i2c_out(imu, MPU6050_ADDR, reg, 1, &data, 1);
}

int read_word(unsigned char reg) {
    unsigned char data[2];
    i2c_in(imu, MPU6050_ADDR, reg, 1, data, 2);
    return (int)((data[0] << 8) | data[1]);  
}

void mpu6050_init() {
    mpu6050_write(PWR_MGMT_1, 0x00);  
    pause(100);
}

void read_imu();  // Function prototype for second cog

int main() {
    // **Initialize Servos**
    servo_angle(SERVO1_PIN, 0);
    servo_angle(SERVO2_PIN, 90);
    print("Servos initialized.\n");
    pause(1000);

    // **Start Ultrasonic Measurement in a Separate Cog**
    cog_ultrasonic = cog_run(measure_distance, 128);

    // **Initialize MPU6050**
    imu = i2c_newbus(0, 1, 0);
    mpu6050_init();
    prev_az = -read_word(ACCEL_ZOUT_H);

    // **Start IMU Reading in Another Cog**
    cog_imu = cog_run(read_imu, 128);

    while(1) {
        // **Ultrasonic Distance-Based Movement**
        print("Distance: %d cm\n", distance_cm);
        if (distance_cm > 0 && distance_cm <= 35 && !servo1_triggered) {
            servo_angle(SERVO1_PIN, 1100);  
            print("Servo 1 moved.\n");
            servo1_triggered = 1;
        }

        // **MPU6050-Based Jerk Detection (Handled in Another Cog)**
        if (jerk_detected) {
            servo_angle(SERVO2_PIN, 180);
            print("Servo 2 moved due to jerk.\n");
            jerk_detected = 0;  // Reset the flag
        }

        pause(100);
    }

    return 0;
}

// **Function to Measure Distance Using Ultrasonic Sensor (Runs in Cog 1)**
void measure_distance() {
    while(1) {
        low(ULTRASONIC_PIN);
        pause(2);
        high(ULTRASONIC_PIN);
        pause(10);
        low(ULTRASONIC_PIN);

        long t = pulse_in(ULTRASONIC_PIN, 1);
        distance_cm = t / 58;

        pause(100);
    }
}

// **Function to Read MPU6050 and Detect Jerk (Runs in Cog 2)**
void read_imu() {
    while (1) {
        int az = -read_word(ACCEL_ZOUT_H);
        int jerk = az - prev_az;

        if (jerk > JERK_THRESHOLD) {
            jerk_detected = 1;  // Set flag for main loop to act
        }

        prev_az = az;
        pause(50);
    }
}
