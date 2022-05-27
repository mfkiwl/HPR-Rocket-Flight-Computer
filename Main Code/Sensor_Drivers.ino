//----------------------------
//Written by SparkyVT, TRA #12111, NAR #85720, L3
//Sensor Package 1: LSM303DLHC, L3GD20, BMP180, ADS1115, ADXL377
//Sensor Package 2: LSM9DS1, MPL3115A2, H3LIS331DL
//Sensor Package 3: LSM9DS1, MPL3115A2, ADXL377
//Sensor Package 4: LSM9DS1, MS5611, H3LIS331DL
//Sensor Package 5: LSM6DS33, LIS3MDL, MS5607, H3LIS331DL
//-----------Change Log------------
//26 Nov 17: Version 1 created to support Adafruit 10Dof board
//10 Nov 18: Version 2 created to support LSM9DS1, H3LIS331DL, BMP280, BMP388
//30 Apr 19: Version 3 created to support MPL3115A2 after EMI problems with BMP280 & BMP388
//31 Jan 20: Version 4 created to support all coded sensors and all orientations
//29 Oct 20: Added code for MS5611 sensor
//04 AUG 21: Added variable gain for LSM9DS1 accelerometer
//10 Aug 21: Removed variable gain after testing showed no expected improvement
//12 NOV 21: Corrected MS5611 bugs and added SPI support for H3LIS331DL
//28 NOV 21: Extended generic barometric pressure sensor offset calibration to all supported sensors
//30 DEC 21: Added support for LSM6DS33, LIS3MDL, MS5607
//03 JAN 22: Moved I2C and SPI generic functions to Bus_Mgmt tab to better support different processors
//10 MAY 22: Updated to have any device on any I2C or SPI bus
//--------Supported Sensors---------
//Accelerometers:LSM303, LSM9DS1, LSM6DS33
//Gyroscopes: L3GD20H, LSM9DS1, LSM6DS33
//Magnetometers: LSM303, LSM9DS1, LIS3MDL
//High-G Accelerometers: H3LIS331DL, ADS1115 & ADXL377 Combo, ADXL377 & Teensy3.5 ADC combo
//Barometric: BMP180, BMP280, BMP388, MPL3115A2, MS5611, MS5607
//WARNING: New generation Bosch sensors are sensitive to RF interference (BMP280,BMP388,etc)
//----------------------------
//LIST OF FUNCTIONS & ROUTINES
//----------------------------
//GENERIC SENSOR FUNCTIONS
//beginAccel(): starts accelerometer
//getAccel(): gets accelerometer data
//beginGyro(): starts gyro
//getGyro(): gets gyro data
//beginHighG(): starts high-g accelerometer
//getHighG(): gets high-g accelerometer data
//beginMag(): starts magnetometer if exernal to IMU
//resetMagGain(): sets the magnetometer gain to 8 Gauss for the magnetic switch function
//getMag(): gets magnetometer data
//beginBaro(): starts the barometer
//getBaro(): gets the barometer data

//SENSOR SPECIFIC FUNCTIONS
//beginLSM303_A(): starts accelerometer
//beginLSM303_M(): starts magnetometer
//getLSM303_A(): gets accelerometer data
//getLSM303_M(): gets magnetometer data

//beginL3GD20H(): starts sensor
//getL3GS20H(): gets gyro data

//beginLSM9DS1_AG(): starts accelerometer & gyro
//beginLSM9DS1_M(): starts magnetometer
//getLSM9DS1_A(): gets accelerometer data
//getLSM9DS1_G(): gets gyro data
//getLSM9DS1_AG(): gets accelerometer & gyro data
//getLSM9DS1_M(): gets magnetometer data

//beginLSM6DS33(): starts sensor
//getLSM6DS33_A(): gets accelerometer data
//getLSM6DS33_G(): gets gyro data
//getLSM6DS33_AG(): gets accelerometer & gyro data

//beginLIS3MDL(): starts sensor
//getLIS3MDL(): gets magnetometer data

//beginH3LIS331DL(): starts sensor
//getH3LIS331DL(): gets high-G accelerometer data

//beginADS1115(): starts ADC
//getADS1115(): gets acelerometer data from ADXL377
//writeRegister(): helper function for ADS1115

//beginADXL377(): starts Teensy ADC
//getADXL377(): gets accelerometer data from Teensy ADC

//beginBMP180(): starts sensor
//initiateTemp():
//initiatePressure():
//getPressure():

//beginBMP280(): starts sensor
//readBMP280coefficients(): gets sensor calibration data
//getBMP280(): gets sensor data

//beginBMP388(): starts sensor
//readBMP388coefficients(): gets sensor calibration data
//getBMP388(): gets sensor data

//beginMPL3115A2(): starts sensor
//getMPL3115A2(): reads sensor data
//pressureToAltitude(): converts pressure to altitude

//beginMS56XX(): starts sensor
//readPromMS56XX(): gets the calibration settings in PROM
//getMS56XX(): gets sensor data and manages the readings between temp and press
//readMS56XX(): manages the I2C bus speed before calling I2C helper functions
//CmdMS56XX1(): helper function to send commands
//ConvertTempMS56XX(): converts temperature
//ConvertPressMS56XX(): converts pressure

//***************************************************************************
//Generic Sensor Begin & Read Statements
//***************************************************************************
//***************************************************************************
//Generic Sensor Begin & Read Statements
//***************************************************************************
void beginAccel() {

  switch (sensors.accel) {

    case 2:
      sensors.status_LSM9DS1 = beginLSM9DS1_AG();
      break;

    case 3:
      sensors.status_LSM6DS33 = beginLSM6DS33();
      break;

    case 1:
      sensors.status_LSM303 = beginLSM303_A();
      break;
  }
}

void getAccel() {

  //get the sensor data
  switch (sensors.accel) {

    case 2:
      if (events.liftoff) {
        getLSM9DS1_AG();
      }
      else {
        getLSM9DS1_A();
      }
      break;

    case 3:
      if (events.liftoff) {
        getLSM6DS33_AG();
      }
      else {
        getLSM6DS33_A();
      }
      break;

    case 1:
      getLSM303_A();
      break;
  }

  //remove bias
  accel.rawX -= accel.biasX;
  accel.rawY -= accel.biasY;
  accel.rawZ -= accel.biasZ;

  //orient sensor data
  accel.x = *accel.ptrX * *accel.ptrXsign;
  accel.y = *accel.ptrY * *accel.ptrYsign;
  accel.z = *accel.ptrZ * *accel.ptrZsign;
}

void beginMag() {

  switch (sensors.mag) {

    case 1:
      beginLSM303_M();
      break;

    case 2:
      beginLSM9DS1_M();
      break;

    case 3:
      beginLIS3MDL();
      break;

    default:
      break;
  }
}

void resetMagGain() {

  //The highest sensitivity produces false positives when using the magnetometer as a switch, so we need to reduce the gain
#define LSM9DS1_ADDRESS_MAG           (0x1E)
#define LSM9DS1_REGISTER_CTRL_REG2_M  (0x21)
#define LSM303_ADDRESS_MAG            (0x3C >> 1)
#define LSM303_REGISTER_MAG_CRB_REG_M (0x01)
#define LSM303_MAGGAIN_1_3            (0x20)
#define LIS3MDL_ADDRESS_MAG           (0x3C)
#define LIS3MDL_REGISTER_CTRL_REG2    (0x21)

  switch (sensors.mag) {

    case 3: //LIS3MDL
      //setup the bus
      setActiveBus(sensors.magBusType, bus.magWire, bus.magRate, LIS3MDL_ADDRESS_MAG | 0x01, bus.magSPI, bus.magSet, pins.magCS);
      //Set gain to 8 Gauss
      write8(LIS3MDL_REGISTER_CTRL_REG2, 0b00100000);
      break;

    case 2: //LSM9DS1
      //setup the bus
      setActiveBus(sensors.magBusType, bus.magWire, bus.magRate, LSM9DS1_ADDRESS_MAG, bus.magSPI, bus.magSet, pins.magCS);
      //Set gain to 8 Gauss
      write8(LSM9DS1_REGISTER_CTRL_REG2_M, 0b00100000);
      break;

    case 1: //LSM303
      //setup the bus
      setActiveBus(sensors.magBusType, bus.magWire, bus.magRate, LSM303_ADDRESS_MAG, bus.magSPI, bus.magSet, pins.magCS);
      //Set gain to 8 Gauss
      write8(LSM303_REGISTER_MAG_CRB_REG_M, 0b11100000);
      break;
  }

  //Set mag trigger
  magTrigger = 3000;

  //correct the bias
  mag.biasX /= 2;
  mag.biasY /= 2;
  mag.biasZ /= 2;

  //clear the buffer
  getMag();
  delay(100);
  getMag();
}

void getMag() {

  //get the sensor data
  switch (sensors.accel) {

    case 3:
      getLIS3MDL();
      break;

    case 2:
      getLSM9DS1_M();
      break;

    case 1:
      getLSM303_M();
      break;
  }

  //remove bias
  mag.rawX -= mag.biasX;
  mag.rawY -= mag.biasY;
  mag.rawZ -= mag.biasZ;

  //translate sensor data
  mag.x = *mag.ptrX * *mag.ptrXsign;
  mag.y = *mag.ptrY * *mag.ptrYsign;
  mag.z = *mag.ptrZ * *mag.ptrZsign;
}

void beginGyro() {

  switch (sensors.gyro) {

    case 1:
      sensors.status_L3GD20H = beginL3GD20H();
      break;

    default:
      break;
  }
}

void getGyro() {

  //get sensor data
  switch (sensors.gyro) {

    case 2:
      if (!events.liftoff) {
        getLSM9DS1_G();
      }
      break;

    case 3:
      if (!events.liftoff) {
        getLSM6DS33_G();
      }
      break;

    case 1:
      getL3GD20H();
      break;
  }

  //remove bias
  gyro.rawX -= gyro.biasX;
  gyro.rawY -= gyro.biasY;
  gyro.rawZ -= gyro.biasZ;

  //orient sensor data
  gyro.x = *gyro.ptrX * *gyro.ptrXsign;
  gyro.y = *gyro.ptrY * *gyro.ptrYsign;
  gyro.z = *gyro.ptrZ * *gyro.ptrZsign;
}

void beginHighG() {

  switch (sensors.highG) {

    case 0:
      sensors.status_NoHighGAccel = true;
      break;

    case 1:
      sensors.status_ADS1115 = beginADS1115('S');
      break;

    case 2:
      sensors.status_H3LIS331DL = beginH3LIS331DL();
      break;

    case 3:
      if (!startADXL377) {
        sensors.status_ADXL377 = beginADXL377();
      }
      break;
  }
}

void getHighG(boolean fullScale) {

  switch (sensors.highG) {

    //get sensor data
    case 0:
      break;

    case 1:
      getADS1115();
      break;

    case 2:
      getH3LIS331DL();
      break;

    case 3:
      getADXL377(fullScale);
      break;
  }

  //remove bias
  highG.rawX -= highG.biasX;
  highG.rawY -= highG.biasY;
  highG.rawZ -= highG.biasZ;

  //orient sensor data
  highG.x = *highG.ptrX * *highG.ptrXsign;
  highG.y = *highG.ptrY * *highG.ptrYsign;
  highG.z = *highG.ptrZ * *highG.ptrZsign;
}

void beginBaro() {

  switch (sensors.baro) {

    case 1:
      sensors.status_BMP180 = beginBMP180();
      break;

    case 2:
      sensors.status_MPL3115A2 = beginMPL3115A2();
      break;

    case 3:
      sensors.status_BMP280 = beginBMP280();
      break;

    case 4:
      sensors.status_BMP388 = beginBMP388();
      break;

    case 5:
      sensors.status_MS5611 = beginMS56XX();
      break;

    case 6:
      sensors.status_MS5607 = beginMS56XX();
      break;
  }
}

void getBaro() {

  switch (sensors.baro) {

    case 1:
      getBMP180();

    case 2:
      getMPL3115A2();
      break;

    case 3:
      getBMP280();
      break;

    case 4:
      getBMP388();
      break;

    case 5:
      getMS56XX();
      break;

    case 6:
      getMS56XX();
      break;
  }
}

//***************************************************************************
//LSM303 Accelerometer
//***************************************************************************

bool beginLSM303_A() {

#define LSM303_ADDRESS_ACCEL               (0x32 >> 1)
#define LSM303_REGISTER_ACCEL_CTRL_REG4_A  (0x23)
#define LSM303_REGISTER_ACCEL_CTRL_REG1_A  (0x20)

  //Define bus settings and start bus- ONLY I2C for LSM303!!
  bus.accelRate = 400000;
  startI2C(sensors.accelBusNum, bus.accelRate);
  bus.accelWire = bus.activeWire;
  bus.activeAddress = LSM303_ADDRESS_ACCEL;

  //check if there is a device at this address
  if (!testSensor(LSM303_ADDRESS_ACCEL)) {
    Serial.println(F("LSM303 Accelerometer not found!"));
    return false;
  }

  //check whoami
  byte id = read8(LSM303_REGISTER_ACCEL_CTRL_REG1_A | 0x80);
  if (id != 0x07) {
    Serial.println(F("LSM303 Accelerometer not found!"));
    return false;
  }
  Serial.println(F("Accel: LSM303 OK!"));

  //----------------------
  //CONFIGURE ACCELEROMETER
  //----------------------
  //set max ADC value
  accel.ADCmax = (int)(0.98 * 2048);

  //Set accelerometer to 1300Hz ODR
  write8(LSM303_REGISTER_ACCEL_CTRL_REG1_A, 0b10010111);

  //Set 16G Range
  write8(LSM303_REGISTER_ACCEL_CTRL_REG4_A, 0b00111000);
  accel.gainX = accel.gainY = accel.gainZ = 0.012;

  //set G level and
  g = int16_t(1 / accel.gainX);
  accel.gainX *= 9.80665;
  accel.gainY *= 9.80665;
  accel.gainZ *= 9.80665;

  return true;
}//end beginLSM303_A

//***************************************************************************
//LSM303 Magnetometer
//***************************************************************************
boolean beginLSM303_M() {
#define LSM303_ADDRESS_MAG            (0x3C >> 1)
#define LSM303_REGISTER_MAG_MR_REG_M  (0x02)
#define LSM303_REGISTER_MAG_CRB_REG_M (0x01)
#define LSM303_REGISTER_MAG_CRA_REG_M (0x00)
#define LSM303_MAGGAIN_1_3            (0x20)
#define LSM303_MAGRATE_15             (0x10)
#define LSM303_IRA_REG_M              (0x0A)

  //Define bus settings and start bus - ONLY I2C for LSM303!!
  bus.magRate = 400000;
  startI2C(sensors.magBusNum, bus.magRate);
  bus.magWire = bus.activeWire;
  bus.activeAddress = LSM303_ADDRESS_MAG;

  //check if there is a device at this address
  if (!testSensor(LSM303_ADDRESS_MAG)) {
    Serial.println(F("LSM303 Magnetometer not found!"));
    return false;
  }

  //check whoami
  byte id = read8(LSM303_IRA_REG_M | 0x80);
  if (id != 0b01001000) {
    Serial.println(F("LSM303 Magnetometer not found!"));
    return false;
  }
  Serial.println(F("Mag: LSM303 OK!"));

  //enable magnetometer
  write8(LSM303_REGISTER_MAG_MR_REG_M, 0x00);

  //set gain
  //_lsm303Mag_Gauss_LSB_XY = 1100;
  //_lsm303Mag_Gauss_LSB_Z  = 980;
  write8(LSM303_REGISTER_MAG_CRB_REG_M, LSM303_MAGGAIN_1_3);
  float gainX = 1100.0;
  float gainY = 1100.0;
  float gainZ = 980.0;
  mag.ADCmax = 32768;

  //LSM303 has different gains for different axes so we must correct for orientation
  if (mag.orientX == 'X') {
    mag.gainX = gainX;
  }
  if (mag.orientX == 'Y') {
    mag.gainX = gainY;
  }
  if (mag.orientX == 'Z') {
    mag.gainX = gainZ;
  }
  if (mag.orientY == 'X') {
    mag.gainY = gainX;
  }
  if (mag.orientY == 'Y') {
    mag.gainY = gainY;
  }
  if (mag.orientY == 'Z') {
    mag.gainY = gainZ;
  }
  if (mag.orientZ == 'X') {
    mag.gainZ = gainX;
  }
  if (mag.orientZ == 'Y') {
    mag.gainZ = gainY;
  }
  if (mag.orientZ == 'Z') {
    mag.gainZ = gainZ;
  }

  //set data rate to 30Hz
  write8(LSM303_REGISTER_MAG_CRA_REG_M, 0b00010100);
  magTime = 33333UL;

  return true;
}//end beginLSM303_M

void getLSM303_A() {

#define LSM303_REGISTER_ACCEL_OUT_X_L_A  (0x28)
  bus.activeBusType = 'I';
  bus.activeWire = bus.accelWire;
  bus.activeWire->setClock(bus.accelRate);
  bus.activeAddress = LSM303_ADDRESS_ACCEL;
  burstRead((LSM303_REGISTER_ACCEL_OUT_X_L_A | 0x80), 6);

  //get the timestamp
  fltTime.tmClockPrev = fltTime.tmClock;
  fltTime.tmClock = micros();
  //assemble the data
  accel.rawX = (int16_t)(rawData[0] | (rawData[1] << 8)) >> 4;
  accel.rawY = (int16_t)(rawData[2] | (rawData[3] << 8)) >> 4;
  accel.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8)) >> 4;
}

void getLSM303_M() {
#define LSM303_REGISTER_MAG_OUT_X_H_M  (0x03)
  bus.activeBusType = 'I';
  bus.activeWire = bus.magWire;
  bus.activeWire->setClock(bus.magRate);
  bus.activeAddress = LSM303_ADDRESS_MAG;
  burstRead(LSM303_REGISTER_MAG_OUT_X_H_M, 6);
  mag.rawX = (int16_t)(rawData[1] | ((int16_t)rawData[0] << 8));
  mag.rawY = (int16_t)(rawData[3] | ((int16_t)rawData[2] << 8));
  mag.rawZ = (int16_t)(rawData[5] | ((int16_t)rawData[4] << 8));
}

//***************************************************************************
//L3GD20 Gyroscope
//***************************************************************************
bool beginL3GD20H() {

#define L3GD20_ADDRESS           0x6B
#define GYRO_REGISTER_WHOAMI     0x0F
#define GYRO_REGISTER_CTRL_REG1  0x20
#define GYRO_REGISTER_CTRL2      0x21
#define GYRO_REGISTER_CTRL_REG4  0x23

  //Define bus settings and start bus
  if (sensors.gyroBusType == 'I') {
    bus.activeAddress = L3GD20_ADDRESS;
    bus.gyroRate = 400000;
    startI2C(sensors.gyroBusNum, bus.gyroRate);
    bus.gyroWire = bus.activeWire;
  }
  else {
    bus.gyroSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.gyroBusNum, pins.gyroCS);
    bus.gyroSPI = bus.activeSPI;
    bus.activeSet = bus.gyroSet;
    bus.activeCS = pins.gyroCS;
  }

  //Check if there is a device at this address
  if (sensors.gyroBusType == 'I') {
    if (!testSensor(L3GD20_ADDRESS)) {
      Serial.println(F("L3GD20H not found!"));
      return false;
    }
  }

  //check whoami
  byte id = read8(GYRO_REGISTER_WHOAMI);
  if (id != 0xD4 && id != 0xD7) {
    Serial.println(F("L3GD20H not found!"));
    return false;
  }
  Serial.println(F("Gyro: L3GD20H OK!"));

  //reset then enable
  write8(GYRO_REGISTER_CTRL_REG1, 0x00);
  write8(GYRO_REGISTER_CTRL_REG1, 0x0F);

  //set gain
  write8(GYRO_REGISTER_CTRL_REG4, 0x20);
  gyro.gainX = gyro.gainY = gyro.gainZ = 0.07;
  gyro.ADCmax = 32768;

  //set data rate 760Hz: 11, 50Hz Bandwidth:01, normal mode, all axes enabled:0b11011111
  write8(GYRO_REGISTER_CTRL_REG1, 0xDF);

  return true;
}

void getL3GD20H() {

#define GYRO_REGISTER_OUT_X_L (0x28)

  //setup the bus
  setActiveBus(sensors.gyroBusType, bus.gyroWire, bus.gyroRate, L3GD20_ADDRESS, bus.gyroSPI, bus.gyroSet, pins.gyroCS);

  //read the data
  burstRead((GYRO_REGISTER_OUT_X_L | 0x80), 6);

  //Capture current timestamp
  fltTime.gyroClockPrev = fltTime.gyroClock;
  fltTime.gyroClock = micros();

  //Assemble the data
  gyro.rawX = (int16_t)(rawData[0] | (rawData[1] << 8));
  gyro.rawY = (int16_t)(rawData[2] | (rawData[3] << 8));
  gyro.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8));
}

//***************************************************************************
//LSM9DS1 Accelerometer & Gyroscope
//***************************************************************************
bool beginLSM9DS1_AG() {

  //Addresses for the registers
#define LSM9DS1_ADDRESS_ACCELGYRO          (0x6B)
#define LSM9DS1_XG_ID                      (0b01101000)
#define LSM9DS1_REGISTER_CTRL_REG1_G         (0x10)
#define LSM9DS1_REGISTER_CTRL_REG5_XL        (0x1F)
#define LSM9DS1_REGISTER_CTRL_REG6_XL        (0x20)
#define LSM9DS1_REGISTER_CTRL_REG3_G         (0x12)

  //Define bus settings and start bus
  if (sensors.accelBusType == 'I') {
    bus.activeAddress = LSM9DS1_ADDRESS_ACCELGYRO;
    bus.accelRate = bus.gyroRate = 400000;
    startI2C(sensors.accelBusNum, bus.accelRate);
    bus.accelWire = bus.gyroWire = bus.activeWire;
  }
  else {
    bus.accelSet = bus.gyroSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.accelBusNum, pins.accelCS);
    bus.accelSPI = bus.gyroSPI = bus.activeSPI;
    bus.activeSet = bus.accelSet;
    bus.activeCS = pins.accelCS;
  }

  //if I2C, check if there is a sensor at this address
  if (sensors.accelBusType == 'I') {
    if (!testSensor(LSM9DS1_ADDRESS_ACCELGYRO)) {
      Serial.println(F("LSM9DS1 not found!"));
      return false;
    }
  }

  //check whoami
  byte id = read8(0x0F);
  if (id != 0b01101000) {
    Serial.println(F("LSM9DS1 not found!"));
    return false;
  }
  Serial.println(F("LSM9DS1 OK!"));

  //Set Accelerometer 16G Range, 952 Hz ODR
  write8(LSM9DS1_REGISTER_CTRL_REG6_XL, 0b11001000);
  accel.ADCmax = (int16_t)(0.98 * 32768);
  accel.gainX = accel.gainY = accel.gainZ = 0.000735;

  //Set Gyroscope 2000dps Range, 952 Hz ODR
  write8(LSM9DS1_REGISTER_CTRL_REG1_G, 0b11011000);
  gyro.gainX = gyro.gainY = gyro.gainZ = 0.07;
  gyro.ADCmax = 32768;

  //set Accelerometer G level and add G to the gains
  g = int16_t(1 / accel.gainX);
  accel.gainX *= 9.80665;
  accel.gainY *= 9.80665;
  accel.gainZ *= 9.80665;

  return true;
}

//***************************************************************************
//LSM9DS1 Magnetometer
//***************************************************************************
bool beginLSM9DS1_M() {
#define LSM9DS1_ADDRESS_MAG                (0x1E)
#define LSM9DS1_MAG_ID                     (0b00111101)
#define LSM9DS1_REGISTER_CTRL_REG1_M         (0x20)
#define LSM9DS1_REGISTER_CTRL_REG2_M         (0x21)
#define LSM9DS1_REGISTER_CTRL_REG3_M         (0x22)
#define LSM9DS1_REGISTER_CTRL_REG4_M         (0x23)

  //Define bus settings and start bus
  if (sensors.magBusType == 'I') {
    bus.activeAddress = LSM9DS1_ADDRESS_MAG;
    bus.magRate = 400000;
    startI2C(sensors.magBusNum, bus.magRate);
    bus.magWire = bus.activeWire;
  }
  else {
    bus.magSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.magBusNum, pins.magCS);
    bus.magSPI = bus.activeSPI;
    bus.activeSet = bus.magSet;
    bus.activeCS = pins.magCS;
  }

  //If I2C, check if there is a sensor at this address
  if (sensors.magBusType == 'I') {
    if (!testSensor(LSM9DS1_ADDRESS_MAG)) {
      Serial.println(F("LSM9DS1 Magnetometer not found!"));
      return false;
    }
  }

  //check whoami
  byte id = read8(0x0F);
  if (id != LSM9DS1_MAG_ID) {
    Serial.println(F("LSM9DS1 Magnetometer not found!"));
    return false;
  }
  Serial.println(F("LSM9DS1 Magnetometer OK!"));


  //Mag Temp Compensation, UltraHigh Perf, 40Hz
  write8(LSM9DS1_REGISTER_CTRL_REG1_M, 0b11111000);
  //Set gain to 4 Gauss
  write8(LSM9DS1_REGISTER_CTRL_REG2_M, 0x00);
  //Mag Continuous Conversion
  write8(LSM9DS1_REGISTER_CTRL_REG3_M, 0x00);
  //Mag Reverse Magnetometer MSB / LSB Order, Z-Axis high-perf mode
  write8(LSM9DS1_REGISTER_CTRL_REG4_M, 0b00001100);

  //set gains
  mag.gainX = mag.gainY = mag.gainZ = (float)(0.14 / 1000);

  return true;
}//end begin

void getLSM9DS1_AG() {

  //this routine uses the LSM9DS1 burst read to rapidly read 12 bytes from the sensors
#define LSM9DS1_REGISTER_OUT_X_L_XL (0x28)
#define LSM9DS1_REGISTER_OUT_X_L_G  (0x18)

  //Capture current timestamp
  fltTime.gyroClockPrev = fltTime.gyroClock;
  fltTime.tmClockPrev = fltTime.tmClock;
  fltTime.gyroClock = fltTime.tmClock = micros();

  //setup the bus
  setActiveBus(sensors.accelBusType, bus.accelWire, bus.accelRate, LSM9DS1_ADDRESS_ACCELGYRO, bus.accelSPI, bus.accelSet, pins.accelCS);

  //read the data
  burstRead(LSM9DS1_REGISTER_OUT_X_L_G, 12);

  //assemble the data
  gyro.rawX = (int16_t)(rawData[0] | (rawData[1] << 8));
  gyro.rawY = (int16_t)(rawData[2] | (rawData[3] << 8));
  gyro.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8));
  accel.rawX  = (int16_t)(rawData[6] | (rawData[7] << 8));
  accel.rawY  = (int16_t)(rawData[8] | (rawData[9] << 8));
  accel.rawZ  = (int16_t)(rawData[10] | (rawData[11] << 8));
}

void getLSM9DS1_A() {

  //This routine reads 6 bytes only from the accelerometer
#define LSM9DS1_REGISTER_OUT_X_L_XL (0x28)

  //setup the bus
  setActiveBus(sensors.accelBusType, bus.accelWire, bus.accelRate, LSM9DS1_ADDRESS_ACCELGYRO, bus.accelSPI, bus.accelSet, pins.accelCS);

  //read the data
  burstRead(0x80 | LSM9DS1_REGISTER_OUT_X_L_XL, 6);

  //Capture current timestamp
  fltTime.gyroClockPrev = fltTime.gyroClock;
  fltTime.gyroClock = micros();
  fltTime.tmClockPrev = fltTime.tmClock;
  fltTime.tmClock = micros();

  accel.rawX = (int16_t)(rawData[0] | (rawData[1] << 8));
  accel.rawY = (int16_t)(rawData[2] | (rawData[3] << 8));
  accel.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8));
}

void getLSM9DS1_G() {

  //This routine reads 6 bytes only from the gyro
#define LSM9DS1_REGISTER_OUT_X_L_G  (0x18)

  //setup the bus
  setActiveBus(sensors.gyroBusType, bus.gyroWire, bus.gyroRate, LSM9DS1_ADDRESS_ACCELGYRO, bus.gyroSPI, bus.gyroSet, pins.gyroCS);

  //read the data
  burstRead(0x80 | LSM9DS1_REGISTER_OUT_X_L_G, 6);

  //assemble the data
  gyro.rawX  = (int16_t)(rawData[0] | (rawData[1] << 8));
  gyro.rawY  = (int16_t)(rawData[2] | (rawData[3] << 8));
  gyro.rawZ  = (int16_t)(rawData[4] | (rawData[5] << 8));
}

void getLSM9DS1_M() {

  //This routine reads 6 bytes from the magnetometer
#define LSM9DS1_REGISTER_OUT_X_L_M  (0x28)

  //setup the bus
  setActiveBus(sensors.magBusType, bus.magWire, bus.magRate, LSM9DS1_ADDRESS_MAG, bus.magSPI, bus.magSet, pins.magCS);

  //read the data
  burstRead(0x80 | LSM9DS1_REGISTER_OUT_X_L_M, 6);

  //assemble the data
  mag.rawX  = (int16_t)(rawData[0] | (rawData[1] << 8));
  mag.rawY  = (int16_t)(rawData[2] | (rawData[3] << 8));
  mag.rawZ  = (int16_t)(rawData[4] | (rawData[5] << 8));
}

//***************************************************************************
//LSM6DS33 Accelerometer & Gyroscope
//***************************************************************************
bool beginLSM6DS33() {

  //Addresses for the registers
#define LSM6DS33_ADDRESS_ACCELGYRO            (0xD6)
#define LSM6DS33_WHOAMI                       (0x0F)
#define LSM6DS33_REGISTER_CTRL1_XL            (0x10)
#define LSM6DS33_REGISTER_CTRL2_G             (0x11)

  //Define bus settings and start bus
  if (sensors.accelBusType == 'I') {
    bus.activeAddress = LSM6DS33_ADDRESS_ACCELGYRO;
    bus.accelRate = bus.gyroRate = 400000;
    startI2C(sensors.accelBusNum, bus.accelRate);
    bus.accelWire = bus.gyroWire = bus.activeWire;}
  else {
    bus.accelSet = bus.gyroSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.accelBusNum, pins.accelCS);
    bus.accelSPI = bus.gyroSPI = bus.activeSPI;
    bus.activeSet = bus.accelSet;
    bus.activeCS = pins.accelCS;}

  //if I2C, check if there is a sensor at this address
  if (sensors.accelBusType == 'I') {
    if (!testSensor(LSM6DS33_ADDRESS_ACCELGYRO)) {
      Serial.println(F("LSM9DS1 not found!"));
      return false;}}

  //check whoami
  byte id = read8(LSM6DS33_WHOAMI);
  if (id != 0b01101001) {
    Serial.println(F("LSM9DS1 not found!"));
    return false;}
  Serial.println(F("LSM9DS1 OK!"));

  //CTRL_REG1_ODR: 10000 = 1.66kHz, 1001 = 3.33kHz, 1010 = 6.66kHz
  //CTRL_REG1_FS: 00 = 2G, 01 = 16G, 10 = 4G, 11 = 8G
  //CTRL_REG1_BW: 00 = 400Hz, 01 = 200Hz, 10 = 100Hz, 11 = 50Hz
  //Accelerometer set 16G Range, 3.33kHz ODR, 400Hz BW
  write8(LSM6DS33_REGISTER_CTRL1_XL, 0b10010100);
  accel.gainX = accel.gainY = accel.gainZ = 0.000488;
  accel.ADCmax = (int16_t)(0.98 * 32768);

  //Gyroscope set 2000dps Range, 1.66kHz ODR
  write8(LSM6DS33_REGISTER_CTRL2_G, 0b10001100);
  gyro.gainX = gyro.gainY = gyro.gainZ = 0.07;
  gyro.ADCmax = 32768;

  //Set G level and add G to the gains
  g = int16_t(1 / accel.gainX);
  accel.gainX *= 9.80665;
  accel.gainY *= 9.80665;
  accel.gainZ *= 9.80665;

  return true;
}//end begin

void getLSM6DS33_AG() {

  //this routine uses the LSM9DS1 burst read to rapidly read 12 bytes from the sensors
#define LSM6DS33_REGISTER_OUTX_L_G (0x22)

  //Capture current timestamp
  fltTime.gyroClockPrev = fltTime.gyroClock;
  fltTime.tmClockPrev = fltTime.tmClock;
  fltTime.gyroClock = fltTime.tmClock = micros();

  //setup the bus
  setActiveBus(sensors.accelBusType, bus.accelWire, bus.accelRate, LSM6DS33_ADDRESS_ACCELGYRO | 0x01, bus.accelSPI, bus.accelSet, pins.accelCS);

  //read the data
  burstRead(LSM6DS33_REGISTER_OUTX_L_G, 12);

  //assemble the data
  gyro.rawX = (int16_t)(rawData[0] | (rawData[1] << 8));
  gyro.rawY = (int16_t)(rawData[2] | (rawData[3] << 8));
  gyro.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8));
  accel.rawX  = (int16_t)(rawData[6] | (rawData[7] << 8));
  accel.rawY  = (int16_t)(rawData[8] | (rawData[9] << 8));
  accel.rawZ  = (int16_t)(rawData[10] | (rawData[11] << 8));
}

void getLSM6DS33_A() {

  //This routine reads 6 bytes only from the accelerometer
#define LSM6DS33_REGISTER_OUTX_L_XL (0x28)

  //Capture current timestamp
  fltTime.gyroClockPrev = fltTime.gyroClock;
  fltTime.tmClockPrev = fltTime.tmClock;
  fltTime.tmClock = fltTime.gyroClock = micros();

  //setup the bus
  setActiveBus(sensors.accelBusType, bus.accelWire, bus.accelRate, LSM6DS33_ADDRESS_ACCELGYRO | 0x01, bus.accelSPI, bus.accelSet, pins.accelCS);

  //read the data
  burstRead(LSM6DS33_REGISTER_OUTX_L_G, 6);

  //assemble the data
  accel.rawX = (int16_t)(rawData[0] | (rawData[1] << 8));
  accel.rawY = (int16_t)(rawData[2] | (rawData[3] << 8));
  accel.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8));
}

void getLSM6DS33_G() {

  //routine is only used for calibration purposes so no timestamp is needed
#define LSM6DS33_REGISTER_OUTX_L_G (0x22)

  //setup the bus
  setActiveBus(sensors.gyroBusType, bus.gyroWire, bus.gyroRate, LSM6DS33_ADDRESS_ACCELGYRO | 0x01, bus.gyroSPI, bus.gyroSet, pins.gyroCS);

  //read the data
  burstRead(LSM6DS33_REGISTER_OUTX_L_G, 6);

  //assemble the data
  gyro.rawX  = (int16_t)(rawData[0] | (rawData[1] << 8));
  gyro.rawY  = (int16_t)(rawData[2] | (rawData[3] << 8));
  gyro.rawZ  = (int16_t)(rawData[4] | (rawData[5] << 8));
}

//***************************************************************************
//LIS3MDL Magnetometer
//***************************************************************************
bool beginLIS3MDL() {

  //Addresses for the registers
#define LIS3MDL_ADDRESS_MAG                  (0x3C)
#define LIS3MDL_WHOAMI                       (0x0F)
#define LIS3MDL_REGISTER_CTRL_REG1           (0x20)
#define LIS3MDL_REGISTER_CTRL_REG2           (0x21)
#define LIS3MDL_REGISTER_CTRL_REG3           (0x22)
#define LIS3MDL_REGISTER_CTRL_REG4           (0x23)

  //Define bus settings and start bus
  if (sensors.magBusType == 'I') {
    bus.activeAddress = LIS3MDL_ADDRESS_MAG;
    bus.magRate = 400000;
    startI2C(sensors.magBusNum, bus.magRate);
    bus.magWire = bus.activeWire;
  }
  else {
    bus.magSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.magBusNum, pins.magCS);
    bus.magSPI = bus.activeSPI;
    bus.activeSet = bus.magSet;
    bus.activeCS = pins.magCS;
  }

  //If I2C, check to see if there is a sensor at this address
  if (sensors.magBusType == 'I') {
    if (!testSensor(LIS3MDL_ADDRESS_MAG | 0x01)) {
      Serial.println(F("LIS3MDL not found!"));
      return false;
    }
  }

  //check whoami
  byte id = read8(LIS3MDL_WHOAMI);
  if (id != 0b00111101) {
    Serial.println(F("LIS3MDL not found!"));
    return false;
  }
  Serial.println(F("LIS3MDL OK!"));

  //Temp Sensor Off, UltraHigh Perf, 40Hz
  write8(LIS3MDL_REGISTER_CTRL_REG1, 0b01111000);

  //Set gain to 4 Gauss
  write8(LIS3MDL_REGISTER_CTRL_REG2, 0x00);

  //Mag Continuous Conversion
  write8(LIS3MDL_REGISTER_CTRL_REG3, 0x00);

  //Mag Reverse Magnetometer MSB / LSB Order, Z-Axis high-perf mode
  write8(LIS3MDL_REGISTER_CTRL_REG3, 0b00001100);

  return true;
}//end begin

void getLIS3MDL() {
#define LIS3MDL_REGISTER_OUT_X_L  (0x28)

  //setup the bus
  setActiveBus(sensors.magBusType, bus.magWire, bus.magRate, LIS3MDL_ADDRESS_MAG | 0x01, bus.magSPI, bus.magSet, pins.magCS);

  //read data
  burstRead(LIS3MDL_REGISTER_OUT_X_L, 6);

  //assemble data
  mag.rawX  = (int16_t)(rawData[0] | (rawData[1] << 8));
  mag.rawY  = (int16_t)(rawData[2] | (rawData[3] << 8));
  mag.rawZ  = (int16_t)(rawData[4] | (rawData[5] << 8));
}

//***************************************************************************
//AXL377 High-G Analog Accelerometer w/ Teensy3.5 ADC
//***************************************************************************
bool beginADXL377() {

  //set gain
  highG.gainX = highG.gainY = highG.gainZ = 9.80655 / 129;
  high1G = 129;
  sizeHighGfilter = 10;
  threeAxisMode = true;

  startADXL377 = true;

  Serial.println(F("ADXL377 OK!"));
  return true;
}

void getADXL377(boolean threeAxisMode) {

  //measured time is 45 micros per reading
  const uint16_t ADCmidValue = 32768;
  long highGsumX = 0L;
  long highGsumY = 0L;
  long highGsumZ = 0L;

  if (threeAxisMode) {

    highGsumX = 0;
    highGsumY = 0;
    highGsumZ = 0;

    for (byte i = 0; i < 5; i++) {
      highGsumX += analogRead(A2);
      highGsumY += analogRead(A3);
      highGsumZ += analogRead(A4);}

    highG.rawX = (int16_t)((highGsumX / 5) - ADCmidValue);
    highG.rawY = (int16_t)((highGsumY / 5) - ADCmidValue);
    highG.rawZ = (int16_t)((highGsumZ / 5) - ADCmidValue);}

  else {

    highGsumZ = 0;

    if (highG.orientX == 'Z') {
      for (byte i = 0; i < 5; i++) {highGsumZ += analogRead(A2);}
      highG.rawX = (int16_t)((highGsumZ / 5) - ADCmidValue);}

    else if (highG.orientY == 'Z') {
      for (byte i = 0; i < 5; i++) {highGsumZ += analogRead(A3);}
      highG.rawY = (int16_t)((highGsumZ / 5) - ADCmidValue);}

    else if (highG.orientZ == 'Z') {
      for (byte i = 0; i < 5; i++) {highGsumZ += analogRead(A4);}
      highG.rawZ = (int16_t)((highGsumZ / 5) - ADCmidValue);}}

}//endvoid

//***************************************************************************
//H3LIS331DL High-G Accelerometer
//***************************************************************************

bool beginH3LIS331DL() {

#define H3LIS331_ADDRESS            (0x19)
#define H3LIS331_REGISTER_CTRL_REG1 (0x20)
#define H3LIS331_REGISTER_CTRL_REG2 (0x21)
#define H3LIS331_REGISTER_CTRL_REG4 (0x23)

  //Define bus settings and start bus
  if (sensors.highGBusType == 'I') {
    bus.activeAddress = H3LIS331_ADDRESS;
    bus.highGRate = 400000;
    startI2C(sensors.highGBusNum, bus.highGRate);
    bus.highGWire = bus.activeWire;
    threeAxisMode = false;}
  else {
    bus.highGSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.highGBusNum, pins.highG_CS);
    bus.highGSPI = bus.activeSPI;
    bus.activeSet = bus.highGSet;
    bus.activeCS = pins.highG_CS;
    threeAxisMode = true;}

  //If I2C, check to see if there is a sensor at this address
  if (sensors.highGBusType == 'I') {
    if (!testSensor(H3LIS331_ADDRESS)) {
      Serial.println(F("H3LIS331 not found!"));
      return false;}}

  //check whoami
  byte id = 0x00;
  if (sensors.highGBusType == 'I') {id = read8(0x0F);}
  else {id = read8(0x8F);}
  if (id != 0x32) {
    Serial.println(F("H3LIS331 not found!"));
    return false;}
  Serial.println(F("H3LIS331 OK!"));

  //sensor setup
  //Normal Mode (001), 1000 Hz data rate (11), Enable axes (111)
  write8(H3LIS331_REGISTER_CTRL_REG1, 0b00111111);
  
  //Set 100G scale
  write8(H3LIS331_REGISTER_CTRL_REG4, 0x00);

  high1G = 21;
  highG.gainX = highG.gainY = highG.gainZ = 0.049 * 9.80655;
  sizeHighGfilter = 15;

  return true;}

void getH3LIS331DL() {

#define H3LIS331_REGISTER_OUT_X_L   0x28
#define H3LIS331_REGISTER_OUT_Y_L   0x2A
#define H3LIS331_REGISTER_OUT_Z_L   0x2C

  //setup the bus
  setActiveBus(sensors.highGBusType, bus.highGWire, bus.highGRate, H3LIS331_ADDRESS, bus.highGSPI, bus.highGSet, pins.highG_CS);

  //I2C Bus----------------------------------------------------------------
  if (sensors.highGBusType == 'I') {

    //set to zero
    highG.rawX = highG.biasX;
    highG.rawY = highG.biasY;
    highG.rawZ = highG.biasZ;

    //high-G over I2C is slow and getting all axes is redundant, so only grab the Z axis
    if (highG.orientX == 'Z') {
      burstRead(H3LIS331_REGISTER_OUT_X_L, 2);
      highG.rawX = (int16_t)(rawData[0] | (rawData[1] << 8)) >> 4;}
    else if (highG.orientY == 'Z') {
      burstRead(H3LIS331_REGISTER_OUT_Y_L, 2);
      highG.rawY = (int16_t)(rawData[0] | (rawData[1] << 8)) >> 4;}
    else if (highG.orientZ == 'Z') {
      burstRead(H3LIS331_REGISTER_OUT_Z_L, 2);
      highG.rawZ = (int16_t)(rawData[0] | (rawData[1] << 8)) >> 4;}
    else if (highG.orientX == 'Q') { //calibration
      burstRead(H3LIS331_REGISTER_OUT_X_L, 6);
      highG.rawX = (int16_t)(rawData[0] | (rawData[1] << 8)) >> 4;
      highG.rawY = (int16_t)(rawData[2] | (rawData[3] << 8)) >> 4;
      highG.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8)) >> 4;}
    return;}

  //SPI Bus----------------------------------------------------------------
  else {
    
    //get data from all axes and assemble
    burstRead(0xC0 | H3LIS331_REGISTER_OUT_X_L, 6);
    highG.rawX = (int16_t)(rawData[0] | (rawData[1] << 8)) >> 4;
    highG.rawY = (int16_t)(rawData[2] | (rawData[3] << 8)) >> 4;
    highG.rawZ = (int16_t)(rawData[4] | (rawData[5] << 8)) >> 4;}
}//end getH3LIS331DL

//***************************************************************************
//ADS1115 ADC interface to ADXL377
//***************************************************************************
bool beginADS1115(char dataRate) {

#define ADS1115_ADDRESS                 (0x48)

  //Define bus settings and start bus - ONLY I2C!!
  bus.activeAddress = ADS1115_ADDRESS;
  bus.highGRate = 400000;
  startI2C(sensors.highGBusNum, bus.highGRate);
  bus.highGWire = bus.activeWire;

  //Check to see if there is a sensor at this address, no whoami
  if (!testSensor(ADS1115_ADDRESS)) {
    Serial.println(F("ADS1115 not found!"));
    return false;
  }
  Serial.println(F("ADS1115 OK!"));

  //       OS: 1   -> begin conversion
  //      MUX: 100 -> read pin 0, compare to ground
  //      PGA: 001 -> +/- 4.096V Gain
  //     MODE: 0   -> Continuous Conversion
  //       DR: 111 -> 860 SPS, 110 -> 475 SPS, 010 -> 32 SPS
  //COMP_MODE: 0   -> Traditional Comparator
  // COMP_POL: 0   -> Active Low
  // COMP_LAT: 0   -> non-latching
  // COMP_QUE: 11  -> Disable Comparator

  uint16_t ADS1115settings;
  switch (dataRate) {
    case 'F': ADS1115settings = 0b1100001011100011;
      break;

    case 'M': ADS1115settings = 0b1100001011000011;
      break;

    case 'S': ADS1115settings = 0b1100001001000011;
      break;

    default: ADS1115settings = 0b1100001011100011;
      break;
  }

  // Write config register to the ADC
#define ADS1115_REG_CONFIG      (0x01)
  write16(ADS1115_REG_CONFIG, ADS1115settings);

  highG.gainX = highG.gainY = highG.gainZ = 0.0183 * 9.80665;
  high1G = 55;
  sizeHighGfilter = 10;

  return true;
}

void getADS1115() {

#define ADS1115_REG_CONVERT     (0x00)
  const int16_t zeroLevel = 13120;

  //set bus
  setActiveBus(sensors.highGBusType, bus.highGWire, bus.highGRate, H3LIS331_ADDRESS, bus.highGSPI, bus.highGSet, pins.highG_CS);

  //read data
  burstRead(ADS1115_REG_CONVERT, 2);

  //X-axis must be pointed towards the sky!
  highG.rawX = (int16_t)(rawData[0] | (rawData[1] << 8));
  highG.rawX -= zeroLevel;
}

//***************************************************************************
//MPL3115A2 Barometric Pressure Sensor
//***************************************************************************

boolean beginMPL3115A2() {
#define MPL3115A2_ADDRESS   0x60
#define MPL3115A2_I2C_READ  0xC1
#define MPL3115A2_I2C_WRITE 0xC0
#define MPL3115A2_WHOAMI    0x0C

  //Define bus settings and start bus - ONLY I2C!!
  bus.activeAddress = MPL3115A2_ADDRESS;
  bus.baroRate = 1000000;
  startI2C(sensors.baroBusNum, bus.baroRate);
  bus.baroWire = bus.activeWire;

  //Check to see if there is a sensor at this address, no whoami
  if (!testSensor(MPL3115A2_ADDRESS)) {
    Serial.println(F("MPL3115A2 I2C Fail!"));
    return false;
  }

  //check whoami
  byte id = read8(MPL3115A2_WHOAMI);
  if (id != 0xC4) {
    Serial.print(F("MPL3115A2 not found! ")); Serial.println(id);
    return false;
  }
  Serial.println(F("MPL3115A2 OK!"));

  //set the sampling frequency
  timeBtwnBaro = 35000UL;

  //SETUP Control Register
  //barometer mode: 0
  //RAW mode: 0
  //Oversampling: 18ms - 010, 34ms - 011, 66ms - 100
  //Reset: 0
  //One Shot Mode (OST): 1
  //Standby: 0
  //bus.activeAddress = MPL3115A2_I2C_WRITE;
  write8(0x26, 0b00011010);

  return true;
}

void getMPL3115A2() {

  //set bus
  setActiveBus(sensors.baroBusType, bus.baroWire, bus.baroRate, MPL3115A2_ADDRESS, bus.baroSPI, bus.baroSet, pins.baroCS);

  //get data
  burstRead(0x01, 5);

  uint32_t adc_P;
  adc_P = rawData[0];
  adc_P <<= 8;
  adc_P |= rawData[1];
  adc_P <<= 8;
  adc_P |= rawData[2];
  adc_P >>= 4;

  int16_t adc_T;
  adc_T = rawData[3];
  adc_T <<= 8;
  adc_T |= rawData[4];
  adc_T >>= 4;

  pressure = (float)(adc_P) * 0.0025 - baroPressOffset;

  if (adc_T & 0x800) {adc_T |= 0xF000;}
  temperature = (float)(adc_T) * 0.0625 - baroTempOffset;

  Alt = 44330.77 * (1.0 - pow(pressure / seaLevelPressure, 0.1902632));

  //initiate next reading
  write8(0x26, 0b00011010);

  newBaro = newTemp = true;
}

float pressureToAltitude(float seaLevel, float atmospheric) {
  return 44330.77 * (1.0 - pow(atmospheric / seaLevel, 0.1902632));
}

//***************************************************************************
//BMP180 Pressure Sensor
//***************************************************************************
/***************************************************************************
  This is a library for the BMP180 pressure sensor

  Designed specifically to work with the Adafruit BMP180 or BMP180 Breakout
  ----> http://www.adafruit.com/products/391
  ----> http://www.adafruit.com/products/1603

  These displays use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
***************************************************************************/
#define BMP180_ADDRESS  (0x77)
#define _BMP180Mode     (3)

struct BMP180_calib_data
{
  int16_t  ac1;
  int16_t  ac2;
  int16_t  ac3;
  uint16_t ac4;
  uint16_t ac5;
  uint16_t ac6;
  int16_t  b1;
  int16_t  b2;
  int32_t  b5;
  int16_t  mb;
  int16_t  mc;
  int16_t  md;
} ;
static BMP180_calib_data _BMP180_coeffs;

boolean beginBMP180() {

#define BMP180_REGISTER_CAL_AC1 0xAA
#define BMP180_REGISTER_CAL_AC2 0xAC
#define BMP180_REGISTER_CAL_AC3 0xAE
#define BMP180_REGISTER_CAL_AC4 0xB0
#define BMP180_REGISTER_CAL_AC5 0xB2
#define BMP180_REGISTER_CAL_AC6 0xB4
#define BMP180_REGISTER_CAL_B1  0xB6
#define BMP180_REGISTER_CAL_B2  0xB8
#define BMP180_REGISTER_CAL_MB  0xBA
#define BMP180_REGISTER_CAL_MC  0xBC
#define BMP180_REGISTER_CAL_MD  0xBE

  //Define bus settings and start bus - ONLY I2C!!
  bus.activeAddress = BMP180_ADDRESS;
  bus.baroRate = 1000000;
  startI2C(sensors.baroBusNum, bus.baroRate);
  bus.baroWire = bus.activeWire;

  //establish contact
  if (!testSensor(BMP180_ADDRESS)) {
    if (settings.testMode) {
      Serial.println(F("BMP180 not found!"));
    }
    return false;
  }

  //check whoami
  byte id = read8(0xD0);
  if (id != 0x58) {
    Serial.println(F("BMP180 not found!"));
    return false;
  }
  Serial.println(F("BMP180 OK!"));

  //Read calibration coefficients
  burstRead(BMP180_REGISTER_CAL_AC1, 2);
  _BMP180_coeffs.ac1 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_AC2, 2);
  _BMP180_coeffs.ac2 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_AC3, 2);
  _BMP180_coeffs.ac3 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_AC4, 2);
  _BMP180_coeffs.ac4 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_AC5, 2);
  _BMP180_coeffs.ac5 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_AC6, 2);
  _BMP180_coeffs.ac6 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_B1, 2);
  _BMP180_coeffs.b1 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_B2, 2);
  _BMP180_coeffs.b2 = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_MB, 2);
  _BMP180_coeffs.mb = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_MC, 2);
  _BMP180_coeffs.mc = (rawData[0] << 8) | rawData[1];
  burstRead(BMP180_REGISTER_CAL_MD, 2);
  _BMP180_coeffs.md = (rawData[0] << 8) | rawData[1];

  //since everthing happens in a timed sequence, we need to check it every cycle
  timeBtwnBaro = 0;

  return true;
}

void getBMP180() {

  //Get a BMP180 barometric event if needed
  //See if a new temp is needed
  const unsigned long tmpRdTime = 4500; //BMP180 needs 4.5ms to read temp
  const unsigned long bmpRdTime = 25500; //BMP180 needs 25.5ms to read pressure
  static boolean getTemp = true;
  static boolean readTemp = false;
  static boolean readPress = false;
  static unsigned long tempReadStart;
  static unsigned long pressReadStart;

  if (getTemp) {
    initiateTemp();
    tempReadStart = micros();
    getTemp = false;
    readTemp = true;
  }

  if (readTemp && micros() - tempReadStart > tmpRdTime) {
    initiatePressure(&temperature);
    pressReadStart = micros();
    newTemp = true;
    readTemp = false;
    readPress = true;
  }

  if (readPress && micros() - pressReadStart > bmpRdTime) {
    getPressure(&pressure);
    prevBaroAlt = Alt;
    Alt = pressureToAltitude(seaLevelPressure, pressure);
    readPress = false;
    getTemp = true;
    newBaro = true;
  }
}

void initiateTemp() {
#define BMP180_REGISTER_CONTROL         0xF4
#define BMP180_REGISTER_READTEMPCMD     0x2E
#define BMP180_REGISTER_READPRESSURECMD 0x34

  //set bus
  setActiveBus(sensors.baroBusType, bus.baroWire, bus.baroRate, BMP180_ADDRESS, bus.baroSPI, bus.baroSet, pins.baroCS);

  //send command
  write8(BMP180_REGISTER_CONTROL, BMP180_REGISTER_READTEMPCMD);
}

void initiatePressure(float *temp) {
  int32_t  ut = 0;
  int32_t X1, X2;     // following ds convention
  float t;

  //Read ucompensated temperature
#define BMP180_REGISTER_TEMPDATA 0xF6
  uint16_t rt;

  //set bus
  setActiveBus(sensors.baroBusType, bus.baroWire, bus.baroRate, BMP180_ADDRESS, bus.baroSPI, bus.baroSet, pins.baroCS);

  //read data
  burstRead(BMP180_REGISTER_TEMPDATA, 2);
  rt = rawData[0] | (rawData[1] >> 8);
  ut = rt;

  //Calculate true temperature
  X1 = (ut - (int32_t)_BMP180_coeffs.ac6) * ((int32_t)_BMP180_coeffs.ac5) >> 15;
  X2 = ((int32_t)_BMP180_coeffs.mc << 11) / (X1 + (int32_t)_BMP180_coeffs.md);
  _BMP180_coeffs.b5 = X1 + X2;
  t = (_BMP180_coeffs.b5 + 8) >> 4;
  t /= 10;
  *temp = t - baroTempOffset;

  //Initiate Pressure
  write8(BMP180_REGISTER_CONTROL, BMP180_REGISTER_READPRESSURECMD + (_BMP180Mode << 6));
}

void getPressure(float *pressure) {

  uint8_t  p8;
  uint16_t p16;
  int32_t  up = 0, compp = 0;
  int32_t  x1, x2, b6, x3, b3, p;
  uint32_t b4, b7;

  //Read uncompensated pressure
#define BMP180_REGISTER_PRESSUREDATA 0xF6

  //set bus
  setActiveBus(sensors.baroBusType, bus.baroWire, bus.baroRate, BMP180_ADDRESS, bus.baroSPI, bus.baroSet, pins.baroCS);

  //get data
  burstRead(BMP180_REGISTER_PRESSUREDATA, 3);

  //assemble data
  p16 = rawData[0] | (rawData[1] >> 8);
  up = (uint32_t)p16 << 8;
  p8 = rawData[2];
  up += p8;
  up >>= (8 - _BMP180Mode);

  //Calculate true pressure
  b6 = _BMP180_coeffs.b5 - 4000;
  x1 = (_BMP180_coeffs.b2 * ((b6 * b6) >> 12)) >> 11;
  x2 = (_BMP180_coeffs.ac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = (((((int32_t) _BMP180_coeffs.ac1) * 4 + x3) << _BMP180Mode) + 2) >> 2;
  x1 = (_BMP180_coeffs.ac3 * b6) >> 13;
  x2 = (_BMP180_coeffs.b1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (_BMP180_coeffs.ac4 * (uint32_t) (x3 + 32768)) >> 15;
  b7 = ((uint32_t) (up - b3) * (50000 >> _BMP180Mode));

  if (b7 < 0x80000000) {
    p = (b7 << 1) / b4;
  }
  else {
    p = (b7 / b4) << 1;
  }

  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  compp = p + ((x1 + x2 + 3791) >> 4);

  /* Assign compensated pressure value */
  *pressure = compp / 100.0F - baroPressOffset;
}

//***************************************************************************
//BMP280 Barometric Pressure Sensor
//***************************************************************************
/***************************************************************************
  This is a library for the BMP280 pressure sensor

  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#define BMP280_ADDRESS                (0x77)

enum
{
  BMP280_REGISTER_DIG_T1              = 0x88,
  BMP280_REGISTER_DIG_T2              = 0x8A,
  BMP280_REGISTER_DIG_T3              = 0x8C,

  BMP280_REGISTER_DIG_P1              = 0x8E,
  BMP280_REGISTER_DIG_P2              = 0x90,
  BMP280_REGISTER_DIG_P3              = 0x92,
  BMP280_REGISTER_DIG_P4              = 0x94,
  BMP280_REGISTER_DIG_P5              = 0x96,
  BMP280_REGISTER_DIG_P6              = 0x98,
  BMP280_REGISTER_DIG_P7              = 0x9A,
  BMP280_REGISTER_DIG_P8              = 0x9C,
  BMP280_REGISTER_DIG_P9              = 0x9E,

  BMP280_REGISTER_CHIPID             = 0xD0,
  BMP280_REGISTER_VERSION            = 0xD1,
  BMP280_REGISTER_SOFTRESET          = 0xE0,

  BMP280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

  BMP280_REGISTER_CONTROL            = 0xF4,
  BMP280_REGISTER_CONFIG             = 0xF5,
  BMP280_REGISTER_PRESSUREDATA       = 0xF7,
  BMP280_REGISTER_TEMPDATA           = 0xFA,
};

typedef struct
{
  uint16_t dig_T1;
  int16_t  dig_T2;
  int16_t  dig_T3;

  uint16_t dig_P1;
  int16_t  dig_P2;
  int16_t  dig_P3;
  int16_t  dig_P4;
  int16_t  dig_P5;
  int16_t  dig_P6;
  int16_t  dig_P7;
  int16_t  dig_P8;
  int16_t  dig_P9;

  uint8_t  dig_H1;
  int16_t  dig_H2;
  uint8_t  dig_H3;
  int16_t  dig_H4;
  int16_t  dig_H5;
  int8_t   dig_H6;
} bmp280_calib_data;

bmp280_calib_data _bmp280_calib;

boolean beginBMP280() {

  //Define bus settings and start bus
  if (sensors.baroBusType == 'I') {
    bus.activeAddress = BMP280_ADDRESS;
    bus.baroRate = 1000000;
    startI2C(sensors.baroBusNum, bus.baroRate);
    bus.baroWire = bus.activeWire;
  }
  else {
    bus.baroSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.baroBusNum, pins.baroCS);
    bus.baroSPI = bus.activeSPI;
    bus.activeSet = bus.baroSet;
    bus.activeCS = pins.baroCS;
  }

  //If I2C, check to see if there is a sensor at this address
  if (sensors.baroBusType == 'I') {
    if (!testSensor(BMP280_ADDRESS)) {
      Serial.println(F("BMP280 not found!"));
      return false;
    }
  }

  //check whoami
  byte id = read8(BMP280_REGISTER_CHIPID);
  if (id != 0x58) {
    Serial.println(F("BMP280 not found!"));
    return false;
  }
  Serial.println(F("BMP280 OK!"));

  //set the sampling control rate
  timeBtwnBaro = 38100UL;

  delay(100);

  //read coefficients
  burstRead(BMP280_REGISTER_DIG_T1, 2);
  _bmp280_calib.dig_T1 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_T2, 2);
  _bmp280_calib.dig_T2 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_T3, 2);
  _bmp280_calib.dig_T3 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P1, 2);
  _bmp280_calib.dig_P1 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P2, 2);
  _bmp280_calib.dig_P2 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P3, 2);
  _bmp280_calib.dig_P3 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P4, 2);
  _bmp280_calib.dig_P4 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P5, 2);
  _bmp280_calib.dig_P5 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P6, 2);
  _bmp280_calib.dig_P6 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P7, 2);
  _bmp280_calib.dig_P7 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P8, 2);
  _bmp280_calib.dig_P8 = (rawData[1] << 8) | rawData[0];
  burstRead(BMP280_REGISTER_DIG_P9, 2);
  _bmp280_calib.dig_P9 = (rawData[1] << 8) | rawData[0];

  //put the BMP280 into sleep mode
  write8(BMP280_REGISTER_CONTROL, 0x00);
  //Set the config register to 0.5ms standby, 16 IIR coeff,
  write8(BMP280_REGISTER_CONFIG, 0b00010100);
  //Set the control register to Ultra high resolution and sleep mode: osrr_t,osrs_p,mode
  write8(BMP280_REGISTER_CONTROL, 0b01010111);

  return true;
}

void getBMP280() {

  //set bus
  setActiveBus(sensors.baroBusType, bus.baroWire, bus.baroRate, BMP280_ADDRESS, bus.baroSPI, bus.baroSet, pins.baroCS);

  //read data
  burstRead(BMP280_REGISTER_PRESSUREDATA, 6);

  int32_t adc_P;
  adc_P = rawData[0];
  adc_P <<= 8;
  adc_P |= rawData[1];
  adc_P <<= 8;
  adc_P |= rawData[2];
  adc_P >>= 4;

  int32_t adc_T;
  adc_T = rawData[3];
  adc_T <<= 8;
  adc_T |= rawData[4];
  adc_T <<= 8;
  adc_T |= rawData[5];
  adc_T >>= 4;

  //put the BMP280 into forced mode
  write8(BMP280_REGISTER_CONTROL, 0b10101010);

  //---------------------------------------------------------------
  //compensate and compute temperature
  //---------------------------------------------------------------

  int32_t var1, var2, t_fine;

  var1  = ((((adc_T >> 3) - ((int32_t)_bmp280_calib.dig_T1 << 1))) *
           ((int32_t)_bmp280_calib.dig_T2)) >> 11;

  var2  = (((((adc_T >> 4) - ((int32_t)_bmp280_calib.dig_T1)) *
             ((adc_T >> 4) - ((int32_t)_bmp280_calib.dig_T1))) >> 12) *
           ((int32_t)_bmp280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;

  temperature  = (t_fine * 5 + 128) >> 8;
  temperature *= .01;
  temperature -= - baroTempOffset;
  //---------------------------------------------------------------
  //Read pressure and convert
  //---------------------------------------------------------------

  int64_t var3, var4, p;

  var3 = ((int64_t)t_fine) - 128000;
  var4 = var3 * var3 * (int64_t)_bmp280_calib.dig_P6;
  var4 = var4 + ((var3 * (int64_t)_bmp280_calib.dig_P5) << 17);
  var4 = var4 + (((int64_t)_bmp280_calib.dig_P4) << 35);
  var3 = ((var3 * var3 * (int64_t)_bmp280_calib.dig_P3) >> 8) +
         ((var3 * (int64_t)_bmp280_calib.dig_P2) << 12);
  var3 = (((((int64_t)1) << 47) + var3)) * ((int64_t)_bmp280_calib.dig_P1) >> 33;

  // avoid exception caused by division by zero
  if (var3 == 0) {
    pressure = 0;
  }
  else {
    p = 1048576 - adc_P;
    p = (((p << 31) - var4) * 3125) / var3;
    var3 = (((int64_t)_bmp280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var4 = (((int64_t)_bmp280_calib.dig_P8) * p) >> 19;

    p = ((p + var3 + var4) >> 8) + (((int64_t)_bmp280_calib.dig_P7) << 4);
    pressure = (float)p / 256;
  }

  //--------------------------------------------
  //Calculate Altitude
  //--------------------------------------------

  pressure *= 0.01;
  pressure -= baroPressOffset;

  Alt = 44330 * (1.0 - pow(pressure / (seaLevelPressure), 0.1903));

  newBaro = newTemp = true;
}

//***************************************************************************
//BMP388 Barometric Pressure Sensor
//***************************************************************************
#define BMP388_ADDRESS                (0x77)

enum {
  REGISTER_NVM_PAR_P11 = 0x45,
  REGISTER_NVM_PAR_P10 = 0x44,
  REGISTER_NVM_PAR_P09U = 0x43,
  REGISTER_NVM_PAR_P09L = 0x42,
  REGISTER_NVM_PAR_P08 = 0x41,
  REGISTER_NVM_PAR_P07 = 0x40,
  REGISTER_NVM_PAR_P06U = 0x3F,
  REGISTER_NVM_PAR_P06L = 0x3E,
  REGISTER_NVM_PAR_P05U = 0x3D,
  REGISTER_NVM_PAR_P05L = 0x3C,
  REGISTER_NVM_PAR_P04 = 0x3B,
  REGISTER_NVM_PAR_P03 = 0x3A,
  REGISTER_NVM_PAR_P02U = 0x39,
  REGISTER_NVM_PAR_P02L = 0x38,
  REGISTER_NVM_PAR_P01U = 0x37,
  REGISTER_NVM_PAR_P01L = 0x36,

  REGISTER_NVM_PAR_T3 = 0x35,
  REGISTER_NVM_PAR_T2U = 0x34,
  REGISTER_NVM_PAR_T2L = 0x33,
  REGISTER_NVM_PAR_T1U = 0x32,
  REGISTER_NVM_PAR_T1L = 0x31,

};

typedef struct
{
  int8_t    NVM_PAR_P11;
  int8_t    NVM_PAR_P10;
  int16_t   NVM_PAR_P09;
  int8_t    NVM_PAR_P08;
  int8_t    NVM_PAR_P07;
  uint16_t  NVM_PAR_P06;
  uint16_t  NVM_PAR_P05;
  int8_t    NVM_PAR_P04;
  int8_t    NVM_PAR_P03;
  int16_t   NVM_PAR_P02;
  int16_t   NVM_PAR_P01;

  int8_t    NVM_PAR_T3;
  uint16_t  NVM_PAR_T2;
  uint16_t  NVM_PAR_T1;

  double    PAR_P11 ;
  double    PAR_P10 ;
  double    PAR_P09 ;
  double    PAR_P08 ;
  double    PAR_P07 ;
  double    PAR_P06 ;
  double    PAR_P05 ;
  double    PAR_P04 ;
  double    PAR_P03 ;
  double    PAR_P02 ;
  double    PAR_P01 ;

  double    PAR_T3 ;
  double    PAR_T2 ;
  double    PAR_T1 ;

  float     t_lin ;

} bmp388_calib_data;

bmp388_calib_data bmp388cal;

boolean beginBMP388() {

#define BMP388_REGISTER_CHIPID 0x00
#define BMP388_REGISTER_PWR_CTRL  0x1B
#define BMP388_REGISTER_CONFIG    0x1F
#define BMP388_REGISTER_OSR       0x1C

  //Define bus settings and start bus
  if (sensors.baroBusType == 'I') {
    bus.activeAddress = BMP388_ADDRESS;
    bus.baroRate = 1000000;
    startI2C(sensors.baroBusNum, bus.baroRate);
    bus.baroWire = bus.activeWire;
  }
  else {
    bus.baroSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.baroBusNum, pins.baroCS);
    bus.baroSPI = bus.activeSPI;
    bus.activeSet = bus.baroSet;
    bus.activeCS = pins.baroCS;
  }

  //If I2C, check to see if there is a sensor at this address
  if (sensors.baroBusType == 'I') {
    if (!testSensor(BMP388_ADDRESS)) {
      Serial.println(F("BMP388 not found!"));
      return false;
    }
  }

  //check whoami
  byte id = read8(BMP388_REGISTER_CHIPID);
  if (id != 0x50) {
    Serial.println(F("BMP388 not found!"));
    return false;
  }
  Serial.println(F("BMP388 OK!"));

  delay(100);

  //read the calibration values
  //8bit values
  burstRead(REGISTER_NVM_PAR_P11, 1);
  bmp388cal.NVM_PAR_P11 = (int8_t)rawData[0];
  burstRead(REGISTER_NVM_PAR_P10, 1);
  bmp388cal.NVM_PAR_P10 = (int8_t)rawData[0];
  burstRead(REGISTER_NVM_PAR_P08, 1);
  bmp388cal.NVM_PAR_P08 = (int8_t)rawData[0];
  burstRead(REGISTER_NVM_PAR_P07, 1);
  bmp388cal.NVM_PAR_P07 = (int8_t)rawData[0];
  burstRead(REGISTER_NVM_PAR_P04, 1);
  bmp388cal.NVM_PAR_P04 = (int8_t)rawData[0];
  burstRead(REGISTER_NVM_PAR_P03, 1);
  bmp388cal.NVM_PAR_P03 = (int8_t)rawData[0];
  burstRead(REGISTER_NVM_PAR_T3 , 1);
  bmp388cal.NVM_PAR_T3  = (int8_t)rawData[0];

  //16bit values
  burstRead(REGISTER_NVM_PAR_P09L, 2);
  bmp388cal.NVM_PAR_P09 = (rawData[1] << 8) | rawData[0];
  burstRead(REGISTER_NVM_PAR_P06L, 2);
  bmp388cal.NVM_PAR_P06 = (rawData[1] << 8) | rawData[0];
  burstRead(REGISTER_NVM_PAR_P05L, 2);
  bmp388cal.NVM_PAR_P05 = (rawData[1] << 8) | rawData[0];
  burstRead(REGISTER_NVM_PAR_P02L, 2);
  bmp388cal.NVM_PAR_P02 = (rawData[1] << 8) | rawData[0];
  burstRead(REGISTER_NVM_PAR_P01L, 2);
  bmp388cal.NVM_PAR_P01 = (rawData[1] << 8) | rawData[0];
  burstRead(REGISTER_NVM_PAR_T2L,  2);
  bmp388cal.NVM_PAR_T2  = (rawData[1] << 8) | rawData[0];
  burstRead(REGISTER_NVM_PAR_T1L,  2);
  bmp388cal.NVM_PAR_T1  = (rawData[1] << 8) | rawData[0];

  //recalculate values
  double denominator;
  denominator = 0.00390625f;
  bmp388cal.PAR_T1 = (double)bmp388cal.NVM_PAR_T1 / denominator;

  denominator = 1073741824.0f;
  bmp388cal.PAR_T2 = (double)bmp388cal.NVM_PAR_T2 / denominator;

  denominator = 281474976710656.0f;
  bmp388cal.PAR_T3 = (double)bmp388cal.NVM_PAR_T3 / denominator;

  denominator = 36893488147419103232.0f;
  bmp388cal.PAR_P11 = (double)bmp388cal.NVM_PAR_P11 / denominator;

  denominator = 281474976710656.0f;
  bmp388cal.PAR_P10 = (double)bmp388cal.NVM_PAR_P10 / denominator;

  bmp388cal.PAR_P09 = (double)bmp388cal.NVM_PAR_P09 / denominator;

  denominator = 32768.0f;
  bmp388cal.PAR_P08 = (double)bmp388cal.NVM_PAR_P08 / denominator;

  denominator = 256.0f;
  bmp388cal.PAR_P07 = (double)bmp388cal.NVM_PAR_P07 / denominator;

  denominator = 64.0f;
  bmp388cal.PAR_P06 = (double)bmp388cal.NVM_PAR_P06 / denominator;

  denominator = 0.125f;
  bmp388cal.PAR_P05 = (double)bmp388cal.NVM_PAR_P05 / denominator;

  denominator = 137438953472.0f;
  bmp388cal.PAR_P04 = (double)bmp388cal.NVM_PAR_P04 / denominator;

  denominator = 4294967296.0f;
  bmp388cal.PAR_P03 = (double)bmp388cal.NVM_PAR_P03 / denominator;

  denominator = 536870912.0f;
  bmp388cal.PAR_P02 = ((double)bmp388cal.NVM_PAR_P02 - 16384.0f) / denominator;

  denominator = 1048576.0f;
  bmp388cal.PAR_P01 = ((double)bmp388cal.NVM_PAR_P01 - 16384.0f) / denominator;

  //put the BMP388 into sleep mode
  write8(BMP388_REGISTER_PWR_CTRL, 0x00);

  //Set the config register 4 IIR coeff: 010 = 4
  write8(BMP388_REGISTER_CONFIG, 0b00000100);

  //Set the control register to Ultra high resolution
  //osr_p: 100 = 16x (2:0)
  //osr_t: 001 = 2x  (5:3)
  write8(BMP388_REGISTER_OSR, 0b00001100);

  //set the sampling control rate
  timeBtwnBaro = 40000UL;

  return true;
}

void getBMP388() {

  //---------------------------------------------------------------
  //burst read from the registers
  //---------------------------------------------------------------

#define BMP388_REGISTER_PRESSUREDATA 0x04

  //set bus
  setActiveBus(sensors.baroBusType, bus.baroWire, bus.baroRate, BMP388_ADDRESS, bus.baroSPI, bus.baroSet, pins.baroCS);

  //get data
  burstRead(BMP388_REGISTER_PRESSUREDATA, 6);

  uint32_t uncomp_press;
  /*uncomp_press = Wire2.read();
    uncomp_press <<= 8;
    uncomp_press |= Wire2.read();
    uncomp_press <<= 8;
    uncomp_press |= Wire2.read();
    uncomp_press >>= 4;*/
  uncomp_press = rawData[0];
  uncomp_press += (rawData[1] << 8);
  uncomp_press += (rawData[2] << 16);

  uint32_t uncomp_temp;
  /*uncomp_temp = Wire2.read();
    uncomp_temp <<= 8;
    uncomp_temp |= Wire2.read();
    uncomp_temp <<= 8;
    uncomp_temp |= Wire2.read();
    uncomp_temp >>= 4;*/
  uncomp_temp = rawData[3];
  uncomp_temp += (rawData[4] << 8);
  uncomp_temp += (rawData[5] << 16);

  //put the BMP388 into forced mode
  //press_en: 1 (0)
  //temp_en:  1 (1)
  //mode:     01 (5:4)
  write8(BMP388_REGISTER_PWR_CTRL, 0b00010011);

  //---------------------------------------------------------------
  //compensate and compute temperature
  //---------------------------------------------------------------

  float partial_data1;
  float partial_data2;
  float partial_data3;
  float partial_data4;
  float partial_out1;
  float partial_out2;

  partial_data1 = (float)(uncomp_temp - bmp388cal.PAR_T1);

  partial_data2 = (float)(partial_data1 * bmp388cal.PAR_T2);

  bmp388cal.t_lin = partial_data2 + (partial_data1 * partial_data1) * bmp388cal.PAR_T3;

  temperature = bmp388cal.t_lin;
  temperature -= - baroTempOffset;

  //---------------------------------------------------------------
  //compensate and compute pressure
  //---------------------------------------------------------------

  partial_data1 = bmp388cal.PAR_P06 * bmp388cal.t_lin;
  partial_data2 = bmp388cal.PAR_P07 * (bmp388cal.t_lin * bmp388cal.t_lin);
  partial_data3 = bmp388cal.PAR_P08 * (bmp388cal.t_lin * bmp388cal.t_lin * bmp388cal.t_lin);
  partial_out1 = bmp388cal.PAR_P05 + partial_data1 + partial_data2 + partial_data3;

  partial_data1 = bmp388cal.PAR_P02 * bmp388cal.t_lin;
  partial_data2 = bmp388cal.PAR_P03 * (bmp388cal.t_lin * bmp388cal.t_lin);
  partial_data3 = bmp388cal.PAR_P04 * (bmp388cal.t_lin * bmp388cal.t_lin * bmp388cal.t_lin);
  partial_out2 = (float)uncomp_press * (bmp388cal.PAR_P01 + partial_data1 + partial_data2 + partial_data3);

  partial_data1 = (float)uncomp_press * (float)uncomp_press;
  partial_data2 = bmp388cal.PAR_P09 + bmp388cal.PAR_P10 * bmp388cal.t_lin;
  partial_data3 = partial_data1 * partial_data2;
  partial_data4 = partial_data3 + ((float)uncomp_press * (float)uncomp_press * (float)uncomp_press) * bmp388cal.PAR_P11;
  pressure  = partial_out1 + partial_out2 + partial_data4;

  //--------------------------------------------
  //Calculate Altitude
  //--------------------------------------------

  pressure *= 0.01;
  pressure -= baroPressOffset;

  Alt = 44330 * (1.0 - pow(pressure / (seaLevelPressure), 0.1903));
  newBaro = newTemp = true;
}

//***************************************************************************
//MS5611 and MS5607 Barometric Pressure Sensors
//***************************************************************************
#define MS56XX_ADDRESS (0x77)
int16_t MS56XX_PROM[6];
int32_t MS56XX_dT;
int32_t MS56XX_TEMP;

void cmdMS56XX(byte cmd) {

  //I2C
  if (sensors.baroBusType == 'I') {
    bus.activeWire->beginTransmission(bus.activeAddress);
    bus.activeWire->write(cmd);
    bus.activeWire->endTransmission();
  }

  //SPI
  else {
    //begin SPI transaction
    bus.activeSPI->beginTransaction(bus.activeSet);
    digitalWriteFast(bus.activeCS, LOW);

    //Send data
    bus.activeSPI->transfer(cmd);

    //end SPI transaction
    digitalWriteFast(bus.activeCS, HIGH);
    bus.activeSPI->endTransaction();
  }
}

bool beginMS56XX() {

#define prom1MS56XX 0xA2
#define prom2MS56XX 0xA4
#define prom3MS56XX 0xA6
#define prom4MS56XX 0xA8
#define prom5MS56XX 0xAA
#define prom6MS56XX 0xAC

  //Define bus settings and start bus
  if (sensors.baroBusType == 'I') {
    bus.activeAddress = MS56XX_ADDRESS;
    bus.baroRate = 1000000;
    startI2C(sensors.baroBusNum, bus.baroRate);
    bus.baroWire = bus.activeWire;
  }
  else {
    bus.baroSet = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    startSPI(sensors.baroBusNum, pins.baroCS);
    bus.baroSPI = bus.activeSPI;
    bus.activeSet = bus.baroSet;
    bus.activeCS = pins.baroCS;
  }

  //If I2C, check to see if there is a sensor at this address
  if (sensors.baroBusType == 'I') {
    if (!testSensor(MS56XX_ADDRESS)) {
      Serial.println(F("MS56XX not found!"));
      return false;
    }
  }
  Serial.println(F("MS56XX OK!"));

  //reset
  cmdMS56XX(0x1E);
  delay(100);

  //read PROM
  burstRead(prom1MS56XX, 2);
  MS56XX_PROM[0] = (rawData[0] << 8 | rawData[1]);
  burstRead(prom2MS56XX, 2);
  MS56XX_PROM[1] = (rawData[0] << 8 | rawData[1]);
  burstRead(prom3MS56XX, 2);
  MS56XX_PROM[2] = (rawData[0] << 8 | rawData[1]);
  burstRead(prom4MS56XX, 2);
  MS56XX_PROM[3] = (rawData[0] << 8 | rawData[1]);
  burstRead(prom5MS56XX, 2);
  MS56XX_PROM[4] = (rawData[0] << 8 | rawData[1]);
  burstRead(prom6MS56XX, 2);
  MS56XX_PROM[5] = (rawData[0] << 8 | rawData[1]);

  //get initial temperature
  cmdMS56XX(0x58);
  delayMicroseconds(9040);
  burstRead(0x00, 3);
  temperature = ConvertTempMS56XX();
  if (settings.testMode) {
    Serial.print("Intial Temp Complete: ");
    Serial.println(temperature, 1);
  }

  //get initial pressure
  cmdMS56XX(0x48);
  delayMicroseconds(9040);
  burstRead(0x00, 3);
  pressure = ConvertPressMS56XX();
  if (settings.testMode) {
    Serial.print("Intial Press Complete: ");
    Serial.println(pressure, 2);
  }

  //get another pressure
  cmdMS56XX(0x48);

  //set the sampling control rate to 110Hz
  timeBtwnBaro = 9090UL;

  return true;
}

void getMS56XX() {

  static byte counter = 0;

  //set bus
  setActiveBus(sensors.baroBusType, bus.baroWire, bus.baroRate, MS56XX_ADDRESS, bus.baroSPI, bus.baroSet, pins.baroCS);

  //Get pressure
  if (counter < 10) {

    //D2 at 4096
    //Read pressure
    burstRead(0x00, 3);
    pressure = ConvertPressMS56XX();

    //update the counter
    counter++;

    //if we have done 10 pressure readings, then initiate a temperature reading
    if (counter >= 10) {
      cmdMS56XX(0x58);
    }

    //else issue the next pressure command
    else {
      cmdMS56XX(0x48);
    }
  }

  //Read temp and issue command for pressure
  else if (counter >= 10) {
    //Read Temp
    burstRead(0x00, 3);
    temperature = ConvertTempMS56XX();
    //Pressure Command
    cmdMS56XX(0x48);
    counter = 0;
  }
}

float ConvertTempMS56XX() {

  uint16_t C6 = MS56XX_PROM[5];
  uint16_t C5 = MS56XX_PROM[4];

  uint32_t D2 = (rawData[0] << 16) | (rawData[1] << 8) | rawData[2];

  MS56XX_dT = D2 - (C5 * 256);

  MS56XX_TEMP = 2000 + ((MS56XX_dT * C6) / 8388608);

  int32_t T2 = 0L;
  if (MS56XX_TEMP < 2000) {
    T2 = (MS56XX_dT * MS56XX_dT) / 2147483648;
  }

  MS56XX_TEMP -= T2;

  float finalTemp = ((float)MS56XX_TEMP) * 0.01 - baroTempOffset;;

  newTemp = true;

  return finalTemp;
}

float ConvertPressMS56XX() {

  uint16_t C1 = MS56XX_PROM[0];
  uint16_t C2 = MS56XX_PROM[1];
  uint16_t C3 = MS56XX_PROM[2];
  uint16_t C4 = MS56XX_PROM[3];
  int64_t OFF1;
  int64_t SENS;
  int64_t OFF2 = 0;
  int64_t SENS2 = 0;
  uint32_t D1;
  int32_t p;

  //-------------------------------------------------------------
  //MS5611
  //-------------------------------------------------------------
  if (sensors.status_MS5611) {
    OFF1 = ((int64_t)C2 << 16) + (((int64_t)C4 * (int64_t)MS56XX_dT) >> 7);
    SENS = ((int64_t)C1 << 15) + (((int64_t)C3 * (int64_t)MS56XX_dT) >> 8);

    if (temperature < 20.0F) {
      OFF2  = 5 * ((int64_t)(MS56XX_TEMP - 2000) * (int64_t)(MS56XX_TEMP - 2000)) / 2;
      SENS2 = 5 * ((int64_t)(MS56XX_TEMP - 2000) * (int64_t)(MS56XX_TEMP - 2000)) / 4;
    }

    if (temperature < -15.0F) {
      OFF2 += 7 * (int64_t)(MS56XX_TEMP + 1500) * (int64_t)(MS56XX_TEMP + 1500);
      SENS2 += 11 * (int64_t)(MS56XX_TEMP + 1500) * (int64_t)(MS56XX_TEMP + 1500) / 2;
    }

    OFF1 -= OFF2;
    SENS -= SENS2;

    D1 = (uint32_t)(rawData[0] << 16 | (rawData[1] << 8) | rawData[2]);

    p = (((D1 * SENS) >> 21) - OFF1) >> 15;
  }

  //-------------------------------------------------------------
  //MS5607
  //-------------------------------------------------------------
  if (sensors.status_MS5607) {
    OFF1 = ((int64_t)C2 << 17) + (((int64_t)C4 * (int64_t)MS56XX_dT) >> 6);
    SENS = ((int64_t)C1 << 16) + (((int64_t)C3 * (int64_t)MS56XX_dT) >> 7);

    if (temperature < 20.0F) {
      OFF2  = 61 * ((int64_t)(MS56XX_TEMP - 2000) * (int64_t)(MS56XX_TEMP - 2000)) / 16;
      SENS2 = 2 * ((int64_t)(MS56XX_TEMP - 2000) * (int64_t)(MS56XX_TEMP - 2000));
    }

    if (temperature < -15.0F) {
      OFF2 += 15 * (int64_t)(MS56XX_TEMP + 1500) * (int64_t)(MS56XX_TEMP + 1500);
      SENS2 += 8 * (int64_t)(MS56XX_TEMP + 1500) * (int64_t)(MS56XX_TEMP + 1500);
    }

    OFF1 -= OFF2;
    SENS -= SENS2;

    D1 = (uint32_t)(rawData[0] << 16 | (rawData[1] << 8) | rawData[2]);

    p = (((D1 * SENS) >> 21) - OFF1) >> 15;
  }

  //---------------------------------------------
  //final conversion
  //---------------------------------------------

  pressure = ((float)p) * 0.01 - baroPressOffset;

  Alt = 44330 * (1.0 - pow(pressure / (seaLevelPressure), 0.1903));

  newBaro = true;

  return pressure;
}
