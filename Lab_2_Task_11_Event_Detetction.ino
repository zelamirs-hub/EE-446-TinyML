#include <Arduino_HS300x.h>
#include <Arduino_BMI270_BMM150.h>
#include <Arduino_APDS9960.h>
#include <math.h>
//Detects increases in humidity, temp, magnetic field, light/color

float temperature(){
  float temp = HS300x.readTemperature();
  return temp;
}

float humidity(){
  float hum = HS300x.readHumidity();
  return hum;
}

float magnet(){
  float mx, my, mz;
  float magnitude = 0;

  while(!IMU.magneticFieldAvailable()){
    delay(1);
  }
  IMU.readMagneticField(mx, my, mz);
  magnitude = sqrt((mx*mx) + (my*my) + (mz*mz));
  return magnitude;
}

struct quad{
  int r;
  int g;
  int b;
  int c;
};

quad light(){
  quad rgbc;
  int r, g, b, c;

  while(!APDS.colorAvailable()){
    delay(1);
  }
  APDS.readColor(r, g, b, c);
  rgbc.r = r;
  rgbc.g = g;
  rgbc.b = b;
  rgbc.c = c;

  return rgbc;
}

float base_temp;
float base_hum;
float base_mag;
int base_red;
int base_green;
int base_blue;
int base_clear;
bool started = false;

void calibrateTemp(){
  if(started == true){
    Serial.println("Recalibrating temp...");
  }
  float temp_sum = 0;
  for(int i = 0; i < 100; i++){
    temp_sum += temperature();
  }
  base_temp = temp_sum/100;
  Serial.print("Baseline Temperature: ");
  Serial.println(base_temp);
}

void calibrateHumidity(){
  if(started == true){
    Serial.println("Recalibrating Humidity...");
  }
  float hum_sum = 0;
  for(int i = 0; i < 100; i++){
    hum_sum += humidity();
  }
  base_hum = hum_sum/100;
  Serial.print("Baseline Humidity: ");
  Serial.println(base_hum);
}

void calibrateMagField(){
  if(started == true){
    Serial.println("Recalibrating Magnetic Field...");
  }
    float mag_sum = 0;
  for(int i = 0; i < 50; i++){
    mag_sum += magnet();
  }
  base_mag = mag_sum/50;
  Serial.print("Baseline Magnetic Field Magnitude: ");
  Serial.println(base_mag);
}

void calibrateLight(){
  if(started == true){
    Serial.println("Recalibrating RGBC...");
  }
  quad rgbc = light();
  int red_sum = 0;
  int green_sum = 0;
  int blue_sum = 0;
  int clear_sum = 0;

  for(int i = 0; i < 50; i++){
    red_sum += rgbc.r;
    green_sum += rgbc.g;
    blue_sum += rgbc.b;
    clear_sum += rgbc.c;
  }

  base_red = red_sum/50;
  base_green = green_sum/50;
  base_blue = blue_sum/50;
  base_clear = clear_sum/50;

  Serial.print("Baseline Red: ");
  Serial.println(base_red);
  Serial.print("Baseline green: ");
  Serial.println(base_green);
  Serial.print("Baseline Blue: ");
  Serial.println(base_blue);
  Serial.print("Baseline Clear: ");
  Serial.println(base_clear);
}

// void recalibate(String sensor){
//   calibrateTemp();
//   calibrateHumidity();
//   calibrateMagneticField();
//   calibrate
// }

void setup() {

  Serial.begin(115200);
  delay(1500);

  if(!IMU.begin()){
    Serial.println("Failed to initialize IMU");
    while(1);
  }
  if(!HS300x.begin()){
    Serial.println("Failed to initialize humidity/temp");
    while(1);
  }
  if(!APDS.begin()){
    Serial.println("Failed to initialize APDS");
    while(1);
  }

  Serial.println("Initialized Succesfully");
  Serial.println("Calibrating Baseline Normal... hold your breath!");

  // float temp_sum = 0;
  // for(int i = 0; i < 100; i++){
  //   temp_sum += temperature();
  // }
  // base_temp = temp_sum/100;
  // Serial.print("Baseline Temperature: ");
  // Serial.println(base_temp);

  // float hum_sum = 0;
  // for(int i = 0; i < 100; i++){
  //   hum_sum += humidity();
  // }
  // base_hum = hum_sum/100;
  // Serial.print("Baseline Humidity: ");
  // Serial.println(base_hum);

  // float mag_sum = 0;
  // for(int i = 0; i < 50; i++){
  //   mag_sum += magnet();
  // }
  // base_mag = mag_sum/50;
  // Serial.print("Baseline Magnetic Field Magnitude: ");
  // Serial.println(base_mag);

  // quad rgbc = light();
  // int red_sum = 0;
  // int green_sum = 0;
  // int blue_sum = 0;
  // int clear_sum = 0;
  // for(int i = 0; i < 50; i++){
  //   red_sum += rgbc.r;
  //   green_sum += rgbc.g;
  //   blue_sum += rgbc.b;
  //   clear_sum += rgbc.c;
  // }
  // base_red = red_sum/50;
  // base_green = green_sum/50;
  // base_blue = blue_sum/50;
  // base_clear = clear_sum/50;

  // Serial.print("Baseline Red: ");
  // Serial.println(base_red);
  // Serial.print("Baseline green: ");
  // Serial.println(base_green);
  // Serial.print("Baseline Blue: ");
  // Serial.println(base_blue);
  // Serial.print("Baseline Clear: ");
  // Serial.println(base_clear);

  calibrateTemp();
  calibrateHumidity();
  calibrateMagField();
  calibrateLight();

  Serial.println("Calibration Done!");
  Serial.print("Starting in 3...");
  delay(1000);
  Serial.print("2...");
  delay(1000);
  Serial.print("1...");
  delay(1000);
}

void loop() {
  started = true;

  String final_label = "BASELINE_NORMAL";
  bool normal_temp = true;
  float temp = temperature();
  bool normal_hum = true;
  float hum = humidity();
  if(temp >  base_temp +1 || hum > base_hum +10){
    normal_temp = false;
    normal_hum = false;
    final_label = "BREATH_OR_WARM_AIR_EVENT";
  }

  bool normal_mag = true;
  float magField = magnet();
  if(magField >  base_mag +20 || magField < base_mag -20){
    normal_mag = false;
    final_label = "MAGNETIC_DISTURBANCE_EVENT";
  }

  bool normal_light = true;
  quad rgbc = light();
  int red = rgbc.r;
  int green = rgbc.g;
  int blue = rgbc.b;
  int clear = rgbc.c;

  if(red > base_red +20 || green > base_green +20 || blue > base_blue +20 || clear > base_clear +50){
    normal_light = false;
    final_label = "LIGHT_OR_COLOR_CHANGE_EVENT";
  }
  else if(red < base_red -20 || green < base_green -20 || blue < base_blue -20 || clear < base_clear -50){
    normal_light = false;
    final_label = "LIGHT_OR_COLOR_CHANGE_EVENT";
  }

  Serial.println(String("Raw Values: rh=") + hum + ", temp=" + temp + ", mag=" + magField + ", r=" + red + ", g=" + green + ", b=" + blue + ", c=" + clear);
  Serial.println(String("Flags: humid_jump=") + !normal_hum + ", temp_rise=" + !normal_temp + ", mag_shift=" + !normal_mag + ", light_or_color_change=" + !normal_light);
  Serial.print("Event: ");
  Serial.println(final_label);  

  if(normal_temp == false){
    calibrateTemp();
  }
  if(normal_hum == false){
    calibrateHumidity();
  }
  if(normal_mag == false){
    calibrateMagField();
  }
  if(normal_light == false){
    calibrateLight();
  }

  delay(500);
}
