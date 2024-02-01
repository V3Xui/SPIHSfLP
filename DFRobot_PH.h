#ifndef _DFROBOT_PH_H_
#define _DFROBOT_PH_H_

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define ReceivedBufferLength 10  //length of the Serial CMD buffer

class DFRobot_PH {
public:
  DFRobot_PH();
  ~DFRobot_PH();
  /**
   * @fn calibration
   * @brief Calibrate the calibration data
   *
   * @param voltage     : Voltage value
   * @param temperature : Ambient temperature
   * @param cmd         : enterph -> enter the PH calibration mode
   * @n                   calph   -> calibrate with the standard buffer solution, two buffer solutions(4.0 and 7.0) will be automaticlly recognized
   * @n                   exitph  -> save the calibrated parameters and exit from PH calibration mode
   */
  void calibration(float voltage, float temperature, char* cmd);  //calibration by Serial CMD
  void calibration(float voltage, float temperature);
  /**
   * @fn readPH
   * @brief Convert voltage to PH with temperature compensation
   * @note voltage to pH value, with temperature compensation
   *
   * @param voltage     : Voltage value
   * @param temperature : Ambient temperature
   * @return The PH value
   */
  float readPH(float voltage, float temperature);
  /**
   * @fn begin
   * @brief Initialization The Analog pH Sensor
   */
  void begin();

private:
  float _phValue;
  float _acidVoltage;
  float _neutralVoltage;
  float _voltage;
  float _temperature;

  char _cmdReceivedBuffer[ReceivedBufferLength];  //store the Serial CMD
  byte _cmdReceivedBufferIndex;

private:
  boolean cmdSerialDataAvailable();
  void phCalibration(byte mode);  // calibration process, wirte key parameters to EEPROM
  byte cmdParse(const char* cmd);
  byte cmdParse();
};

#endif