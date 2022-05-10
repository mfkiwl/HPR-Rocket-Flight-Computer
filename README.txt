*NOTE: If you are planning to use this code or board files, please send me a Private Message (PM) on Rocketry Forum 
to ensure I have posted the latest updates.  https://www.rocketryforum.com/members/sparkyvtflyer.810/  

HPR Rocket Flight Computer
Original sketch by SparkyVT
NAR #85720, L3
TRA #12111, L3

--------DESCRIPTION----------
This flight computer is designed for rockets 38mm in diameter or greater, and will fit inside a 38mm tube coupler.
Dimensions are 4.0in x 1.25in x 0.5in, not including antenna
It has similar capability as a TeleMega, but without a smart-phone app or advanced interfaces  
It has flown on multiple M-powered supersonic flights to over 24K feet and Mach 2.0.  
For large or high-power projects, an independent backup computer such as the PerfectFlite StratoLogger, MissileWorks RRC2, or EggTimer Quark is strongly recommended.  

--------FEATURES----------
Full-featured dual deploy/multi-stage/airstart rocket flight computer capable to 100,000ft or more
Multitple programmable events to include apogee droge deployment, dual-deploy main chute deployment, 2-stage & airstart capable
Live telemetry over 433MHz or 915MHz LoRa (433MHz: USA amateur 70cm band, EUR licencse free) (915MHz: USA FHSS licence free or USA amateur license use non-FHSS) 
4 high-current pyro outputs with continuity checks
High Data-Capture rate: approximately 50,000 samples per second recorded to SD card
--1400Hz 3-axis digital 24G and 100G accelerometer data logging
--1400Hz 3-axis digital 2000dps gyroscope data logging
--1400Hz of flight events & continuity data logging
--1400Hz of sensor-fuzed speed & altitude
--100Hz of pitch, yaw, roll rotation
--40Hz of of magnetic data logging and magnetic roll
--30Hz-100Hz of digital barometric data logging (Altitude, pressure, temperature)
--30Hz of main battery voltage (1400Hz during pyro events)
--20Hz of LoRa telemetry output (time, event, acceleration, speed, altitude, rotation, GNSS altitude, GNSS position, signal strength, packet number)
--5Hz-25Hz of GNSS data logging (chip-dependent data rates & constellations)
--Separate data file for each flight up to 100 flights
Simple, easy-to-use configuration interface through the SD card
--User Selectable Flight Mode: Single-Stage, Two-Stage, Airstart, or Booster
--Configurable Apogee delay
--Optional Audible Battery Voltage report at startup
--Optional Magnetic Switch Startup & Shut-down
--Preflight audible reporting options: Perfectflight or Marsa
--User selectable telemetry frequency & power settings
--8 configurable servo outputs (4 powered, 4 un-powered)
--User selectable inflight brownout recovery
--User selectable active stabilization for roll, pitch, and yaw correction
--User selectable return-to-pad controlled recovery
Tilt-sensing lockout for ignition of second stages and/or airstarts
Mach immune, sensor-fusion based apogee event
Barometric based main deploy event
Audible pre-flight continuity report
Audible Post-flight max altitude & speed report
Mount in any orientation, automatic orientation detection during calibration
Bench-test mode activated w/ tactile button; user configurable status messages over USB Serial
Built-in self-calibration mode 
Report in SI or Metric units
Compatible with Teensy 3.2, 3.5, 3.6, 4.0, 4.1
--Base code is compatible with many different sensors over I2C or SPI
--Base code configurable to use different pin input/outputs and hardware I2C/SPI bus options
--GPS can be wired to any available HW Serial port

--------FLIGHT COMPUTER COMPONENTS----------
    Microprocessor: Teensy 3.2, 3.5, 3.6, 4.0, 4.1 (PCB file compatible with Teensy 3.5/3.6)
               IMU: LSM9DS1, LSM6DS33, or LSM303 & L3GD20H combo
      Mangetometer: LSM9DS1, LSM303, or LIS3MDL
100G Accelerometer: H3LIS331DL, ADXL377, or ADXL377 & ADS1115 combo
   Pressure Sensor: MS5611, MS5607, MPL3115A2, BMP180, BMP280, or BMP388 (Note: BMP280 & BMP388 incompatible with telemetry due to EMI)
              GNSS: UBLOX M6, M7, M8, M9 (NEO footprint supported in PCB file)
    LoRa Telemetry: RFM96W 433MHz (USA: Amateur License Required, EUR: License Free) or RFM95W 915MHz (USA: License Free, optional software FHSS)
   Tactile Buttons: 0.2 in spacing
               PCB: Create your own or use the provided design file
           Battery: 2-cell 400mAh 20C LiPo recommended (DO NOT EXCEED 10V with provided PCB)

--------GROUND STATION COMPONENTS----------
                  MCU: Teensy 3.2
            Telemetry: RFM96W 433MHz (USA: Ham License Required, EUR: License Free) or RFM95W 915MHz (USA: License Free)
                  GPS: Ublox SAM-M8Q
                  IMU: Sparkfun LSM9DS1 breakout
                  LCD: 20X4 Sparkfun LCD
              SD Card: Adafruit SD Card breakout board
              Antenna: 433Mhz 5-element Yagi (Ham Only) or 900Mhz (License Free)
                 Case: File provided for 3D printed case
           BNC to SMA: 6-inch cable
            SMA cable: 6-inch cable
              Battery: Recommended 7.4V 2-cell LiPo 450MAh
      1K Ohm Resistor: 1/8 Watt
     10K Ohm Resistor: 1/8 Watt

--------ADDITIONAL LIBRARIES REQUIRED----------
TinyGPS++

--------INTENDED FUTURE REVISIONS----------
1) Upgrade ground station with GPS, digital compass, & Bluetooth
2) Develop Android App for ground station
3) Optional remote start & shutdown command

--------NOTES----------
Note: All of the above components were purchased through Digikey.  Other
sources for the same components should work, but are untested and may not
fit the provided PCB

Note: 1.25in x 4.0in size board.  Will fit in a 38mm coupler tube.
Components mount on both sides.  See provided pictures for examples.

Note: Estimated Flight Computer Cost = $150USD
Note: Estimated Ground Station Cost = $140USD (does not include antenna)
Note: Estimated build time = 3hrs
Note: Estimated programming, setup, calibration, & debugging time = 4hrs
