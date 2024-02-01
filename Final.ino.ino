#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DFRobot_PH.h"
#include <EEPROM.h>
#include "GravityTDS.h"

// Define Sensors and Display variables
#define tdsSensorPin A0
#define phSensorPin A1
#define VREF 5.0      // analog reference voltage (Volt) of the ADC
#define SCOUNT 30     // sum of sample point
#define NUTRIENT_A 4  // the Arduino pin, which connects to the IN1 pin of relay module
#define NUTRIENT_B 5  // the Arduino pin, which connects to the IN2 pin of relay module
#define PH_DOWN 6     // the Arduino pin, which connects to the IN3 pin of relay module
#define PH_UP 7       // the Arduino pin, which connects to the IN4 pin of relay module

// Declare Sensors and Display variables
GravityTDS gravityTds;
DFRobot_PH ph;
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Declare TDS and pH threshold variables
const int tdsLowThreshold = 600;     // TDS value below which Nutrient A and Nutrient B will be dosed
const int tdsHighThreshold = 800;    // TDS value above which no nutrients will be dosed
const float pHLowThreshold = 5.00;   // pH value below which pH down will be dosed
const float pHHighThreshold = 6.00;  // pH value above which pH up will be dosed

int analogBuffer[SCOUNT];  // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
int buf[10], temp;
float b;
unsigned long int avgValue;  //Store the average value of the sensor feedback

void setup() {
  // Initialize Sensors and Display
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  delay(1000);
  lcd.init();
  lcd.backlight();

  pinMode(NUTRIENT_A, OUTPUT);
  pinMode(NUTRIENT_B, OUTPUT);
  pinMode(PH_DOWN, OUTPUT);
  pinMode(PH_UP, OUTPUT);
}

void loop() {
  //TDS parameter
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(tdsSensorPin);  //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;                                                                                                   // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);                                                                                                                //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient;                                                                                                             //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5;  //convert voltage value to tds value
    Serial.print("TDS Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");

    //pH parameter
    for (int i = 0; i < 10; i++)  //Get 10 sample value from the sensor for smooth the value
    {
      buf[i] = analogRead(phSensorPin);
      delay(10);
    }
    for (int i = 0; i < 9; i++)  //sort the analog from small to large
    {
      for (int j = i + 1; j < 10; j++) {
        if (buf[i] > buf[j]) {
          temp = buf[i];
          buf[i] = buf[j];
          buf[j] = temp;
        }
      }
    }
    avgValue = 0;
    for (int i = 2; i < 8; i++)  // take the average value of 6 center sample
      avgValue += buf[i];
    float phValue = -5.70 * avgValue / 1024.0 / 6.5 + 7.5;  // convert ADC value to pH value
    Serial.print(" pH Value: ");
    Serial.println(phValue);

    // Display pH and TDS values on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TDS: ");
    lcd.print(tdsValue);
    lcd.print(" ppm");
    lcd.setCursor(0, 1);
    lcd.print(" pH: ");
    lcd.print(phValue);

    // Control nutrient pumps based on TDS value
    if (tdsValue < tdsLowThreshold) {
      digitalWrite(NUTRIENT_A, HIGH);
      digitalWrite(NUTRIENT_B, HIGH);
      delay(500);
      digitalWrite(NUTRIENT_A, LOW);
      digitalWrite(NUTRIENT_B, LOW);
    } else if (tdsValue > tdsHighThreshold) {
      digitalWrite(NUTRIENT_A, LOW);
      digitalWrite(NUTRIENT_B, LOW);
    }

    // Control pH pumps based on pH value
    if (phValue < pHLowThreshold) {
      digitalWrite(PH_UP, LOW);     // turn off pH up pump
      digitalWrite(PH_DOWN, HIGH);  // turn on pH down pump
      delay(500);
      digitalWrite(PH_DOWN, LOW);  // turn off pH down pump
    } else if (phValue > pHHighThreshold) {
      digitalWrite(PH_DOWN, LOW);  // turn off pH down pump
      digitalWrite(PH_UP, HIGH);   // turn on pH up pump
      delay(500);
      digitalWrite(PH_UP, LOW);  // turn off pH up pump
    } else {
      digitalWrite(PH_UP, LOW);    // turn off pH up pump
      digitalWrite(PH_DOWN, LOW);  // turn off pH down pump
    }
    delay(10000);
  }
}
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}
