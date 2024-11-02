#include <Arduino.h>

#include <math.h>
#include <Wire.h>

int16_t mpu6050_gyro_x = 0, mpu6050_gyro_y = 0, mpu6050_gyro_z = 0, mpu6050_temp_raw = 0, mpu6050_acc_x = 0, mpu6050_acc_y = 0, mpu6050_acc_z = 0;
int mpu6050_gyro_offset_x = 0, mpu6050_gyro_offset_y = 0, mpu6050_gyro_offset_z = 0, mpu6050_acc_init_x = 0, mpu6050_acc_init_y = 0, mpu6050_acc_init_z = 0;
float mpu6050_acc_init_pitch_angle = 0, mpu6050_acc_init_roll_angle = 0, mpu6050_acc_pitch_angle = 0, mpu6050_acc_roll_angle = 0, mpu6050_gyro_pitch_angle = 0, mpu6050_gyro_roll_angle = 0, mpu6050_gyro_yaw_angle = 0;
float mpu6050_GYRO_SCALE = 131.0;
unsigned long mpu6050_start_time = 0;
float acc_pitch = 0;
float acc_roll = 0;
float gyro_pitch = 0;
float gyro_roll = 0;
float gyro_yaw = 0;

void MPU6050_setup(){
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1B);
  Wire.write(0b00000);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0b00000);
  Wire.endTransmission();
}

void MPU6050_measure(){
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68,14);
  while(Wire.available() < 14);
  mpu6050_acc_x = Wire.read()<<8|Wire.read();
  mpu6050_acc_y = Wire.read()<<8|Wire.read();
  mpu6050_acc_z = Wire.read()<<8|Wire.read();
  mpu6050_temp_raw = Wire.read()<<8|Wire.read();
  mpu6050_gyro_x = Wire.read()<<8|Wire.read();
  mpu6050_gyro_y = Wire.read()<<8|Wire.read();
  mpu6050_gyro_z = Wire.read()<<8|Wire.read();

  mpu6050_gyro_x -= mpu6050_gyro_offset_x;
  mpu6050_gyro_y -= mpu6050_gyro_offset_y;
  mpu6050_gyro_z -= mpu6050_gyro_offset_z;

  (abs(mpu6050_gyro_x) < mpu6050_GYRO_SCALE) ? mpu6050_gyro_x = 0 : true;
  (abs(mpu6050_gyro_y) < mpu6050_GYRO_SCALE) ? mpu6050_gyro_y = 0 : true;
  (abs(mpu6050_gyro_z) < mpu6050_GYRO_SCALE) ? mpu6050_gyro_z = 0 : true;
}

void MPU6050_calibrate(){
  long sum_mpu6050_gyro_x = 0, sum_mpu6050_gyro_y = 0, sum_mpu6050_gyro_z = 0, sum_mpu6050_acc_x = 0, sum_mpu6050_acc_y = 0, sum_mpu6050_acc_z = 0;

  long calibration = 0;
  Serial.println("Starting MPU6050 calibration in 5 seconds. Don't move during calibration.");
  delay(5000);
  Serial.println("Starting MPU6050 calibration now...");
  for (calibration = 0; calibration < 2000; calibration++){
    MPU6050_measure();
    sum_mpu6050_gyro_x += mpu6050_gyro_x;
    sum_mpu6050_gyro_y += mpu6050_gyro_y;
    sum_mpu6050_gyro_z += mpu6050_gyro_z;
    sum_mpu6050_acc_x += mpu6050_acc_x;
    sum_mpu6050_acc_y += mpu6050_acc_y;
    sum_mpu6050_acc_z += mpu6050_acc_z;
  }
  Serial.println("Finished MPU6050 calibration!");

  mpu6050_gyro_offset_x = int(sum_mpu6050_gyro_x / calibration);
  mpu6050_gyro_offset_y = int(sum_mpu6050_gyro_y / calibration);
  mpu6050_gyro_offset_z = int(sum_mpu6050_gyro_z / calibration);
  mpu6050_acc_init_x = int(sum_mpu6050_acc_x / calibration);
  mpu6050_acc_init_y = int(sum_mpu6050_acc_y / calibration);
  mpu6050_acc_init_z = int(sum_mpu6050_acc_z / calibration);

  mpu6050_acc_init_pitch_angle = atan2(mpu6050_acc_init_x, mpu6050_acc_init_z)*360/(2*PI);
  mpu6050_acc_init_roll_angle = atan2(mpu6050_acc_init_y, mpu6050_acc_init_z)*360/(2*PI);
  mpu6050_gyro_pitch_angle += mpu6050_acc_init_pitch_angle;
  mpu6050_gyro_roll_angle += mpu6050_acc_init_roll_angle;

  mpu6050_start_time = millis();
}

void MPU6050_calc_angles(float mpu6050_GYRO_TRUST){
  float elapsed_sec = 0;
  mpu6050_GYRO_TRUST /= 100;
  float mpu6050_ACC_TRUST = 1 - mpu6050_GYRO_TRUST;

  mpu6050_acc_pitch_angle = atan2(mpu6050_acc_x, mpu6050_acc_z)*360/(2*PI);
  mpu6050_acc_roll_angle = atan2(mpu6050_acc_y, mpu6050_acc_z)*360/(2*PI);

  elapsed_sec = float(millis() - mpu6050_start_time)/1000;
  mpu6050_start_time = millis();
  mpu6050_gyro_pitch_angle -= mpu6050_gyro_y * elapsed_sec / mpu6050_GYRO_SCALE;
  mpu6050_gyro_roll_angle += mpu6050_gyro_x * elapsed_sec / mpu6050_GYRO_SCALE;
  mpu6050_gyro_yaw_angle += mpu6050_gyro_z * elapsed_sec / mpu6050_GYRO_SCALE;

  mpu6050_gyro_pitch_angle += mpu6050_gyro_roll_angle * sin(mpu6050_gyro_z * elapsed_sec / mpu6050_GYRO_SCALE * (2*PI) / 360);
  mpu6050_gyro_roll_angle -= mpu6050_gyro_pitch_angle * sin(mpu6050_gyro_z * elapsed_sec / mpu6050_GYRO_SCALE * (2*PI) / 360);
  mpu6050_gyro_pitch_angle = (mpu6050_gyro_pitch_angle * mpu6050_GYRO_TRUST) + (mpu6050_acc_pitch_angle * mpu6050_ACC_TRUST);
  mpu6050_gyro_roll_angle = (mpu6050_gyro_roll_angle * mpu6050_GYRO_TRUST) + (mpu6050_acc_roll_angle * mpu6050_ACC_TRUST);

  //swapping pitch and roll angle + negate roll angle for blockcoding.click Gyro Board compatibility
  acc_pitch = mpu6050_acc_roll_angle;
  acc_roll = -mpu6050_acc_pitch_angle;
  gyro_pitch = mpu6050_gyro_roll_angle;
  gyro_roll = -mpu6050_gyro_pitch_angle;
  gyro_yaw = mpu6050_gyro_yaw_angle;
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  MPU6050_setup();
  MPU6050_calibrate();
}

void loop() {
  MPU6050_measure();
  MPU6050_calc_angles(99.8);
  Serial.print((String("Acc Pitch: ") + String((String)(acc_pitch)) + String("°  |  ")));
  Serial.print((String("Acc Roll:  ") + String((String)(acc_roll)) + String("°  |  ")));
  Serial.print((String("Gyro Pitch:  ") + String((String)(gyro_pitch)) + String("°  |  ")));
  Serial.print((String("Gyro Roll:  ") + String((String)(gyro_roll)) + String("°  |  ")));
  Serial.println((String("Gyro Yaw:  ") + String((String)(gyro_yaw)) + String("°")));
  delay(250);

}