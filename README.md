New personal project to make use of a few sensors and a screen I bought a long time ago. The idea would be to build some sort of flight logger.

## Hardware
* ST-7735 compatible breakout board + SD Card reader from Vetco
* Adafruit Ultimate GPS breakout board
* Adafruit 10DoF IMU
* Parallax HMC5883 Magnetometer sensor
* Parallax MS5607 Barometer/Altimeter sensor

## Current state
* most sensors are easy to wire over SPI or I2C but calibration will be an issue.
* the SD card reader doesn't seem to work well even with basic sample code, it's very unreliable. probably a hardware issue. considering replacing the Vetco part by the Adafruit equivalent.