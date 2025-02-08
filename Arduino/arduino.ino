#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "SQM_TSL2591.h"
#include <Adafruit_MLX90614.h>
#include <FreqMeasure.h>
#include <Math.h>
#include <AccelStepper.h>
#include <Servo.h>

bool read_data = true;
double interval = 1000;

//############### SENSORS ###############

#define ONE_WIRE_BUS 2
#define GAS_LPG 0   
#define GAS_CO  1   
#define GAS_SMOKE 2 
#define HALFSTEP 8
#define motorPin1  9 
#define motorPin2  10 
#define motorPin3  11  
#define motorPin4  12

AccelStepper stepper(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
const int MQ_PIN=A3;
const int RELAY_PIN1 = 5;
const int RELAY_PIN2 = 6;
const int RELAY_5V_PIN = 7;
Adafruit_SHT31 sht31a = Adafruit_SHT31();
Adafruit_SHT31 sht31b = Adafruit_SHT31();
Adafruit_BMP280 bmp = Adafruit_BMP280();
SQM_TSL2591 sqm = SQM_TSL2591(2591);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);	
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
// Rain sensor minimum
const int rainSensorMin = 0;
// Rain sensor maximum
const int rainSensorMax = 1024; 
// Gas detection based on 
// https://www.instructables.com/How-to-Detect-Concentration-of-Gas-by-Using-MQ2-Se/
int RL_VALUE=5;                                  
float RO_CLEAN_AIR_FACTOR=9.83;                     
float LPGCurve[3] = {2.3,0.21,-0.47};
float COCurve[3] = {2.3,0.72,-0.34};
float SmokeCurve[3] = {2.3,0.53,-0.44};                                                    
float Ro = 10; 
int CALIBARAION_SAMPLE_TIMES=50; 
int CALIBRATION_SAMPLE_INTERVAL=500;
int READ_SAMPLE_INTERVAL=50; 
int READ_SAMPLE_TIMES=5;
//#######################################

// Reset Arduino
void(* resetFunc) (void) = 0; 


//################END resetFunc#######################

// Main setup function
// Init sensors or return an error
void setup() {
  Serial.begin(9600);
  String response = initSensors();
  if(response != "OK"){
    Serial.print( F("{"));
    Serial.print( F("'ok':'")); Serial.print(false); Serial.print( F("',"));
    Serial.print( F("'error':'")); Serial.print(response); Serial.print( F("'"));
    Serial.print( F("}"));
    Serial.println( F("###"));
  } else {
    Serial.print(F("{")); 
    Serial.print(F("'ok':'")); Serial.print(true); Serial.print(F("'")); 
    Serial.print(F("}"));
    Serial.println(F("###")); 
  }
}


//###############END setup()###################

// Main loop function -
// Serial read and execute
void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('#');
    if(command == "resetSensors"){
      String response = initSensors();
      if(response != "OK"){
        Serial.print(F("{")); 
        Serial.print(F("'ok':'")); Serial.print(false); Serial.print(F("',")); 
        Serial.print(F("'error':'")); Serial.print(response); Serial.print(F("'")); 
        Serial.print(F("}"));
        Serial.println(F("###")); 
      } else {
        Serial.print(F("{")); 
        Serial.print(F("'ok':'")); Serial.print(true); Serial.print(F("'")); 
        Serial.print(F("}")); 
        Serial.println(F("###")); 
      }
    }
    else if(command == "getSensors") {  
      getSensors();
    }
    else if(command == "cameraHeaterOn") {  
      heaterControl(RELAY_PIN1,"On");
    }
    else if(command == "cameraHeaterOff") {  
      heaterControl(RELAY_PIN1,"Off");
    } 
    else if(command == "sensorsHeaterOn") {  
      heaterControl(RELAY_PIN2,"On");
    }
    else if(command == "sensorsHeaterOff") {  
      heaterControl(RELAY_PIN2,"Off");
    }
    else if(command == "focuserOn") {  
      focuserControl(true);
    }
    else if(command == "focuserOff") {  
      focuserControl(false);
    }
    else if(command == "focusCWverysmall") {  
      rotateMotor(200,"CW");
    } 
    else if(command == "focusCCWverysmall") {  
      rotateMotor(200,"CCW");
    } 
    else if(command == "focusCWsmall") {  
      rotateMotor(400,"CW");
    } 
    else if(command == "focusCCWsmall") {  
      rotateMotor(400,"CCW");
    } 
    else if(command == "focusCWmed") {  
      rotateMotor(600,"CW");
    } 
    else if(command == "focusCCWmed") {  
      rotateMotor(600,"CCW");
    } 
    else if(command == "focusCWhigh") {  
      rotateMotor(800,"CW");
    } 
    else if(command == "focusCCWhigh") {  
      rotateMotor(800,"CCW");
    }
    else if(command == "focusCWmax") {  
      rotateMotor(1000,"CW");
    } 
    else if(command == "focusCCWmax") {  
      rotateMotor(1000,"CCW");
    }
    else if(command == "calibrateAirSensor") {  
      calibrateAirSensor();
    }
    else if(command == "reset") {
      delay(2000);
      resetFunc();
    }
  }
  delay(interval);
}


//###############END loop()####################


// Rotate focuser by <steps> in <direction>
void rotateMotor(int steps,String direction){
  if(direction == "CW"){
    stepper.moveTo(stepper.currentPosition()+steps); 
  } else {
    stepper.moveTo(stepper.currentPosition()-steps); 
  }
  stepper.setSpeed(1000); 
  while (stepper.currentPosition() != stepper.targetPosition()) { 
      stepper.runSpeedToPosition();
  }
  delay(1000);
}


//#################END rotateMotor#####################

// Activate/deactivate focuser relay
void focuserControl(bool onoff){
  if(onoff) {
    printStatus(F("Focuser On"));
    digitalWrite(RELAY_5V_PIN, LOW);
  } else {
    printStatus(F("Focuser Off"));
    digitalWrite(RELAY_5V_PIN, HIGH);
  }
  delay(1000);
  return;
}


//#################END focuserControl######################

// Activate/deactivate heaters relays
void heaterControl(int pin,String onoff){
  if(onoff == "On") {
    if(pin == 5) {
      printStatus(F("Heater On (main)"));
      digitalWrite(RELAY_PIN1, LOW);
    } else {
      printStatus(F("Heater On (rain sensor)")); 
      digitalWrite(RELAY_PIN2, LOW); 
    }
    
    
  } else {
    if(pin == 5) {
      printStatus(F("Heater Off (main)"));
      digitalWrite(RELAY_PIN1, HIGH);
    } else {
      printStatus(F("Heater Off (rain sensor)"));
      digitalWrite(RELAY_PIN2, HIGH);
    }
  }
  delay(1000);
  return;
}


//##################END heaterControl#####################

// Serial print Status
void printStatus(String message) {
  Serial.print(F("{")); 
  Serial.print(F("'ok':'")); Serial.print(true); Serial.print(F("',"));
  Serial.print(F("'status':'")); Serial.print(message); Serial.print(F("'")); 
  Serial.print(F("}"));
  Serial.println(F("###")); 
  return;
}


//#################END printStatus######################

// Print sensors information
void getSensors(){
  int rainSensorReading = analogRead(A0);
  int range = map(rainSensorReading, rainSensorMin, rainSensorMax, 0, 3);
  sensors.requestTemperatures(); 
  sqm.takeReading();
  
  Serial.print(F("{")); 
  Serial.print(F("'sht31a_temp':'")); Serial.print(sht31a.readTemperature()); Serial.print(F("',")); 
  Serial.print(F("'sht31a_humidity':'")); Serial.print(sht31a.readHumidity()); Serial.print(F("',")); 
  Serial.print(F("'sht31b_temp':'")); Serial.print(sht31b.readTemperature()); Serial.print(F("',")); 
  Serial.print(F("'sht31b_humidity':'")); Serial.print(sht31b.readHumidity()); Serial.print(F("',")); 
  Serial.print(F("'ds18b20_temp':'")); Serial.print(sensors.getTempCByIndex(0)); Serial.print(F("',")); 
  Serial.print(F("'bmp_temp':'")); Serial.print(bmp.readTemperature()); Serial.print(F("',")); 
  Serial.print(F("'bmp_pressure':'")); Serial.print(bmp.readPressure()/100); Serial.print(F("',")); 
  Serial.print(F("'bmp_altitude':'")); Serial.print(bmp.readAltitude(1012.66)); Serial.print(F("',")); 
  Serial.print(F("'mlx_ambient':'")); Serial.print(mlx.readAmbientTempC()); Serial.print(F("',")); 
  Serial.print(F("'mlx_object':'")); Serial.print(mlx.readObjectTempC()); Serial.print(F("',")); 
  Serial.print(F("'tsl237_sqm':'")); Serial.print(getFreqMeasure()); Serial.print(F("',")); 
  Serial.print(F("'tsl2591_full':'")); Serial.print(sqm.full); Serial.print(F("',")); 
  Serial.print(F("'tsl2591_ir':'")); Serial.print(sqm.ir); Serial.print(F("',")); 
  Serial.print(F("'tsl2591_visible':'")); Serial.print(sqm.vis); Serial.print(F("',")); 
  Serial.print(F("'tsl2591_sqm':'")); Serial.print(sqm.mpsas); Serial.print(F("',")); 
  Serial.print(F("'tsl2591_uncertainty':'")); Serial.print(sqm.dmpsas); Serial.print(F("',")); 
  Serial.print(F("'tsl2591_gain':'")); Serial.print(sqm.config.gain); Serial.print(F("',")); 
  Serial.print(F("'tsl2591_time':'")); Serial.print(sqm.config.time); Serial.print(F("',")); 
  Serial.print(F("'isRaning':'")); Serial.print(getRainSensor(range)); Serial.print(F("',")); 
  Serial.print(F("'Bortel':'")); Serial.print(calculateBortel(sqm.mpsas)); Serial.print(F("',")); 
  Serial.print(F("'airSensor':'")); Serial.print(analogRead(A3)); Serial.print(F("',")); 
  Serial.print(F("'lpg':'")); Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG)); Serial.print(F("',")); 
  Serial.print(F("'co':'")); Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO)); Serial.print(F("',")); 
  Serial.print(F("'smoke':'")); Serial.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE)); Serial.print(F("',")); 
  Serial.print(F("'camera_heater':'")); Serial.print(!digitalRead(RELAY_PIN2)); Serial.print(F("',")); 
  Serial.print(F("'sensors_heater':'")); Serial.print(!digitalRead(RELAY_PIN1)); Serial.print(F("',"));
  Serial.print(F("'focuser':'")); Serial.print(!digitalRead(RELAY_5V_PIN)); Serial.print(F("',"));
  Serial.print(F("'ok':'")); Serial.print(true); Serial.print(F("'"));
  Serial.print(F("}")); 
  Serial.println(F("###")); 
  return;  
}


//#################END getSensors######################

// Initialize sensors
String initSensors() {
  pinMode(A3, INPUT);
  
  // High Address Pin for 2nd SHT31 - 0x45
  pinMode(3, OUTPUT); 
  digitalWrite(3, HIGH);

  pinMode(RELAY_PIN1, OUTPUT);
  digitalWrite(RELAY_PIN1, HIGH);
  pinMode(RELAY_PIN2, OUTPUT);
  digitalWrite(RELAY_PIN2, HIGH);
  pinMode(RELAY_5V_PIN, OUTPUT);
  digitalWrite(RELAY_5V_PIN, HIGH);

  stepper.setMaxSpeed(1001);
  stepper.setCurrentPosition(0); // set position

  // SHT31 sensors
  if (!sht31a.begin(0x44)){  
    return "ERROR:Could not find a valid SHT31a sensor, check wiring!";
  }
  if (!sht31b.begin(0x45)){  
    return "ERROR:Could not find a valid SHT31b sensor, check wiring!";
  }
   
  // DS18B20
  sensors.begin();

  // MLX90614
  if (!mlx.begin()) {  
    return "ERROR:Could not find a valid MLX90614 sensor, check wiring!";
  }

  // BMP280 0x76
  if (!bmp.begin(0x76)) {
    return "ERROR:Could not find a valid BMP280 sensor, check wiring!";
  }

  // TSL2591
  if (sqm.begin()) {
    sqm.config.gain = TSL2591_GAIN_LOW;
    sqm.config.time = TSL2591_INTEGRATIONTIME_200MS;
    sqm.configSensor();
    sqm.setCalibrationOffset(0.0);
    sqm.verbose = false;
  } else {
    return "ERROR:Could not find a valid TSL2591 sensor, check wiring!";
  }

  return "OK";
}


//################END initSensors#######################

// Measure frequency
float getFreqMeasure(){
  FreqMeasure.begin();
  int reading = 1;
  double sum=0;
  int count=0;
  float Msqm;
  const float A = 22.5;
  while(reading) {
    if (FreqMeasure.available()) {
      sum = sum + FreqMeasure.read();
      count +=1;

      if (count > 30) {
        double frequency = F_CPU / (sum / count);
        sum = 0;
        count = 0;
        //Frequency to magnitudes/arcSecond2
        Msqm = A - 2.5*log10(frequency); 
        reading = 0; 
        FreqMeasure.end();
      }
    }
  }
  return Msqm;
}


//################END getFreqMeasure#######################

// Rain sensor reading
String getRainSensor(int range){
  switch (range) {
  case 0:    // Sensor getting very wet
      return "Raining Hard";
      break;
  case 1:    // Sensor getting wet
      return "Raining";
      break;
  case 2:    // Sensor dry
      return "Not Raining";
      break;
    }
}


//###############END getRainSensor########################

//Bortle calculation based on SQM
String calculateBortel(double sqm) {
  if(sqm<=18.38){
    return "B.8/9";
  } else if(sqm<=18.94 and  sqm>18.38){
    return "B.7";
  }
  else if(sqm<=19.50 and  sqm>18.94){
    return "B.6";
  }
  else if(sqm<=20.49 and  sqm>19.50){
    return "B.5";
  }
  else if(sqm<=21.69 and  sqm>20.49){
    return "B.4";
  }
  else if(sqm<=21.89 and  sqm>21.69){
    return "B.3";
  }
  else if(sqm<=21.99 and  sqm>21.89){
    return "B.2";
  }
  else if(sqm<=22.0 and  sqm>21.99){
    return "B.1";
  } else {
    return "Too Bright";
  }
}


//################END calculateBortel#######################

// Air sensor MQ-135 related functions 

void calibrateAirSensor(){
  Ro = MQCalibration(MQ_PIN);
  printStatus(F("Air sensor calibration complete"));
  return;
}


float MQCalibration(int mq_pin){
  int i;
  float val=0;
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {   
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;  
  val = val/RO_CLEAN_AIR_FACTOR;                                                           
  return val;                                             

}

float MQResistanceCalculation(int raw_adc) {
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}



float MQRead(int mq_pin) {
  int i;
  float rs=0;
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
  rs = rs/READ_SAMPLE_TIMES;
  return rs;  
}


long MQGetGasPercentage(float rs_ro_ratio, int gas_id){
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    
  return 0;
}

long  MQGetPercentage(float rs_ro_ratio, float *pcurve){
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
