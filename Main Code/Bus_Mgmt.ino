//--------Supported Buses---------
//Teensy 3.2  3.5  3.6  4.0  4.1
//I2C0    Y    Y    Y    Y    Y
//I2C1    Y    Y    Y    Y    Y
//I2C2    N    Y    Y    Y    Y
//I2C3    N    N    Y    N    N    
//SPI0    Y    Y    Y    Y    Y
//SPI1    N    Y    Y    Y    Y
//SPI2    N    Y    Y    Y    N
//SDIO    N    Y    Y    Y    Y
//SER1    Y    Y    Y    Y    Y
//SER2    Y    Y    Y    Y    Y
//SER3    Y    Y    Y    Y    Y
//SER4    N    Y    Y    Y    Y
//SER5    N    Y    Y    Y    Y
//SER6    N    Y    Y    Y    Y
//SER7    N    N    N    Y    Y
//SER8    N    N    N    N    Y
//-----------Change Log------------
//03 Jan 22: Created to support Teensy 3.2, 4.0, and 4.1
//18 APR 22: Updated to support any device on any bus
//----------------------------
//LIST OF FUNCTIONS & ROUTINES
//----------------------------
//startI2C(): starts the I2C bus
//testSensor(): tests to see if there is a sensor at the given I2C address
//startSPI(): starts the SPI bus
//setActiveBus(): configures the primary bus for a subsequent read or write
//write8(): writes 8 bits to the register 
//write16(): writes 16 bits to the register
//read8(): reads 8 bits from the register
//burstRead(): reads a given number of bytes from the starting register
//setHWSERIAL(): sets the active UART bus for the GPS to the user specified bus
//------Code Start---------
typedef struct{
  TwoWire *accelWire;
  TwoWire *gyroWire;
  TwoWire *magWire;
  TwoWire *highGWire;
  TwoWire *baroWire;
  TwoWire *activeWire;
  uint32_t accelRate;
  uint32_t gyroRate;
  uint32_t magRate;
  uint32_t highGRate;
  uint32_t baroRate;
  uint32_t activeRate;
  SPIClass *accelSPI;
  SPIClass *gyroSPI;
  SPIClass *magSPI;
  SPIClass *highGSPI;
  SPIClass *baroSPI;
  SPIClass *activeSPI;
  SPISettings accelSet;
  SPISettings gyroSet;
  SPISettings magSet;
  SPISettings highGSet;
  SPISettings baroSet;
  SPISettings activeSet;
  byte activeCS;
  uint8_t activeAddress;
  char activeBusType;
} activeBus;
activeBus bus;
//***************************************************************************
//Generic UART Hardware Serial Functions
//***************************************************************************
void setHWSERIAL(){

  Serial.print(F("Setting HW Serial"));Serial.print(sensors.gpsBusNum);Serial.print(F("..."));

  switch(sensors.gpsBusNum){

    case 1: 
      HWSERIAL = &Serial1;
      break;
      
    case 2:
      HWSERIAL = &Serial2;
      break;
      
    case 3:
      HWSERIAL = &Serial3;
      break;
      
    //Teensy 3.5, 3.6, 4.0, 4.1
    #if defined (__MK64FX512__) || defined (__MK66FX1M0__) || defined (__IMXRT1062__)
      case 4:
        HWSERIAL = &Serial4;
        break;

      case 5:
        HWSERIAL = &Serial5;
        break;
        
      case 6:
        HWSERIAL = &Serial6;
        break;
      
    #endif

    //Teensy 4.0 and Teensy 4.1
    #if defined (__IMXRT1062__)

      case 7:
        HWSERIAL = &Serial7;
        break;

    #endif

    //Teensy 4.1
    #if defined (ARDUINO_TEENSY41)

      case 8:
        HWSERIAL = &Serial8;
        break;
    #endif
  }
  Serial.println(F("Done!"));}
//***************************************************************************
//Generic I2C Functions
//***************************************************************************
void startI2C(uint8_t busNum, uint32_t rate){
  
  if(settings.testMode){Serial.print("Starting i2c bus ");}
  
  bus.activeBusType = 'I';
  
  switch (busNum){

    //All Teensy 3.X versions
    #if defined (__MK64FX512__) || defined (__MK66FX1M0__) || defined (__MK20DX256__)
      case 0: Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire;
              break;
      case 4: Wire.begin(I2C_MASTER, 0x00, I2C_PINS_16_17, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire;
              busNum = 0;
              break;
    #endif

    //Teensy 3.5 and Teensy 3.6
    #if defined (__MK64FX512__) || defined (__MK66FX1M0__)
      case 1: Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_37_38, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire1;
              break;
      case 2: Wire2.begin(I2C_MASTER, 0x00, I2C_PINS_3_4, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire2;
              break;
      case 5: Wire.begin(I2C_MASTER, 0x00, I2C_PINS_7_8, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire;
              busNum = 0;
              break;
      case 6: Wire.begin(I2C_MASTER, 0x00, I2C_PINS_33_34, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire;
              busNum = 0;
              break;
      case 7: Wire.begin(I2C_MASTER, 0x00, I2C_PINS_47_48, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire;
              busNum = 0;
              break;
    #endif

    //Teensy 3.2
    #if defined (__MK20DX256__)
      case 1: Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_29_30, I2C_PULLUP_EXT, rate);
              bus.activeWire = &Wire1;
              busNum = 1;
              break;
    #endif

    //Teensy 3.6 extra bus
    #if defined (__MK66FX1M0__)
      case 3: Wire3.begin(I2C_MASTER, 0x00, I2C_PINS_56_57, I2C_PULLUP_EXT, rate);
               bus.activeWire = &Wire3;
               break;
    #endif

    //Teensy 4.0 and Teensy 4.1
    #if defined (__IMXRT1062__)
       case 0: Wire.begin();
               Wire.setClock(rate);
               bus.activeWire = &Wire;
               break;
      case 1:  Wire1.begin();
               Wire1.setClock(rate);
               bus.activeWire = &Wire1;
               break;
      case 2:  Wire2.begin();
               Wire2.setClock(rate);
               bus.activeWire = &Wire2;
               break;
      default: Wire.begin();
               Wire.setClock(400000);
               bus.activeWire = &Wire;
               break;
    #endif

    //All Teensy 3.X versions
    #if defined (__MK64FX512__) || defined (__MK66FX1M0__) || defined (__MK20DX256__)
      default: Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
              bus.activeWire = &Wire;
              break;
    #endif
  }
  if(settings.testMode){Serial.print(busNum);Serial.print(F("..."));}}

bool testSensor(byte address) {
  bus.activeWire->beginTransmission(address);
  if(bus.activeWire->endTransmission() == 0){return true;}
  else{return false;}}

//***************************************************************************
//Generic SPI Functions
//***************************************************************************
void startSPI(uint8_t busNum){
    
  if(settings.testMode){Serial.print("Starting SPI bus  "); Serial.print(busNum);Serial.print(F("..."));}

  bus.activeBusType = 'S';
  
  switch (busNum){

    //All boards
    case 0: SPI.begin();
            bus.activeSPI = &SPI;
            break;

    //Only for Teensy 3.5, 3.6, 4.0, and 4.1
    #if defined (__MK64FX512__) || defined (__MK66FX1M0__) || defined (__IMXRT1062__)
    
    case 1: SPI1.begin();
            bus.activeSPI = &SPI1;
            break;
    case 2: SPI2.begin();
            bus.activeSPI = &SPI2;
            break;
    #endif

    default: SPI.begin();
             bus.activeSPI = &SPI;
             break;}}//end startBus
//***************************************************************************
//Set Active Bus Function
//***************************************************************************
void setActiveBus(char busType, TwoWire *_wire, uint32_t Rate, byte addr, SPIClass *_spi, SPISettings spiSet, uint8_t CS){
  if(busType == 'I'){
    bus.activeBusType = 'I';
    bus.activeAddress = addr;
    bus.activeWire = _wire;
    if(bus.activeRate != Rate){
      bus.activeWire->setClock(Rate);
      bus.activeRate = Rate;}}
  else{
    bus.activeBusType = 'S';
    bus.activeSet = spiSet;
    bus.activeSPI = _spi;
    bus.activeCS = CS;}}
    
//***************************************************************************
//Common Read / Write Functions
//***************************************************************************
uint8_t read8(byte reg) {

  uint8_t val;

  //I2C Read
  if(bus.activeBusType == 'I'){
    bus.activeWire->beginTransmission(bus.activeAddress);
    bus.activeWire->write(reg);
    bus.activeWire->endTransmission(false);
    bus.activeWire->requestFrom(bus.activeAddress, (byte)1);
    while (bus.activeWire->available() < 1) {};
    val = bus.activeWire->read();}
    
  //SPI Read
  else{
    //begin SPI transaction
    bus.activeSPI->beginTransaction(bus.activeSet);
    digitalWriteFast(bus.activeCS, LOW);
    //send register
    bus.activeSPI->transfer(reg);
    //read data
    val = bus.activeSPI->transfer(0);
    //end SPI transaction
    digitalWriteFast(bus.activeCS, HIGH);
    bus.activeSPI->endTransaction();}
    
    return val;}

void burstRead(byte reg, byte bytes) {

  //I2C Burst Read
  if(bus.activeBusType == 'I'){
    bus.activeWire->beginTransmission(bus.activeAddress);
    bus.activeWire->write(reg);
    bus.activeWire->endTransmission(false);
    bus.activeWire->requestFrom(bus.activeAddress, bytes);
    while (bus.activeWire->available() < bytes) {};
    for (byte i = 0; i < bytes; i++) {rawData[i] = bus.activeWire->read();}}

  //SPI Burst Read
  else{
    //begin SPI transaction
    bus.activeSPI->beginTransaction(bus.activeSet);
    digitalWriteFast(bus.activeCS, LOW);
    
    //send register
    bus.activeSPI->transfer(reg);
  
    //read data
    for(byte i = 0; i < bytes; i++){rawData[i] = bus.activeSPI->transfer(0);}
  
    //end SPI transaction
    digitalWriteFast(bus.activeCS, HIGH);
    bus.activeSPI->endTransaction();}}

void write8(byte reg, uint8_t val) {

  //I2C Write
  if(bus.activeBusType == 'I'){
    bus.activeWire->beginTransmission(bus.activeAddress);
    bus.activeWire->write(reg);
    bus.activeWire->write(val);
    bus.activeWire->endTransmission(false);}

  //SPI Write
  else{
    //begin SPI transaction
    bus.activeSPI->beginTransaction(bus.activeSet);
    digitalWriteFast(bus.activeCS, LOW);
    
    //Send data
    bus.activeSPI->transfer(reg);
    bus.activeSPI->transfer(val);
  
    //end SPI transaction
    digitalWriteFast(bus.activeCS, HIGH);
    bus.activeSPI->endTransaction();}}

void write16(byte reg, uint16_t val) {

  //I2C write
  if(bus.activeBusType == 'I'){
    bus.activeWire->beginTransmission(bus.activeAddress);
    bus.activeWire->write((uint8_t)reg);
    bus.activeWire->write((uint8_t)(val >> 8));
    bus.activeWire->write((uint8_t)(val & 0xFF));
    bus.activeWire->endTransmission(false);}

  //SPI write
  else{
    
    //begin SPI transaction
    bus.activeSPI->beginTransaction(bus.activeSet);
    digitalWriteFast(bus.activeCS, LOW);
    
    //Send data
    bus.activeSPI->transfer(reg);
    bus.activeSPI->transfer(val >> 8);
    bus.activeSPI->transfer(val & 0xFF);
  
    //end SPI transaction
    digitalWriteFast(bus.activeCS, HIGH);
    bus.activeSPI->endTransaction();}}
