#include <PDM.h>
#include <Arduino_BMI270_BMM150.h>
#include <Arduino_APDS9960.h>
#include <math.h>

short sampleBuffer[256];
volatile int samplesRead = 0;

void onPDMdata(){
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable/2;
}

int sound(){
  if (samplesRead == 0){
    return 0;
  }

  int level;
  if(samplesRead){
    long sum = 0;
    int n = samplesRead;

    for(int i = 0; i < n; i++){
      sum +=abs(sampleBuffer[i]);
    }
    level = sum/n;
    samplesRead = 0;
    return level;
  }
} 

float motion(){
  float ax, ay, az;
  float magnitude = 0;

  if(IMU.accelerationAvailable()){
    IMU.readAcceleration(ax, ay, az);
    magnitude = sqrt((ax*ax) + (ay*ay) + ((az-1)*(az-1)));
    return magnitude;
  }
}

int proximity(){
  while(!APDS.proximityAvailable()){
    delay(1);
  }
  int dist = APDS.readProximity();
  return dist;
}

int light(){
  int r, g, b, c;
  
  while(!APDS.colorAvailable()){
    delay(1);
  }
  APDS.readColor(r, g, b, c);
  return c;
}

void setup() {

  Serial.begin(115200);
  delay(1500);

  PDM.onReceive(onPDMdata);
  if(!PDM.begin(1, 16000)){
    Serial.println("Failed to start PDM mic");
    while(1);
  }

  if(!IMU.begin()){
    Serial.println("Failed IMU");
    while(1);
  }

  if(!APDS.begin()){
    Serial.println("Failed to initialize APDS");
    while(1);
  }
  Serial.println("Initialized Succesfully");
}

void loop() {

  bool steady = true;
  String str_mov = "STEADY";
  float movement = motion();
  if(movement > 0.5){
    steady = false;
    str_mov = "MOVING";
  }

  bool quiet = true;
  String str_lev = "QUIET"; 
  int level = sound();
  if(level > 50){
    quiet = false;
    str_lev = "NOISY";
  }

  bool near = true;
  String str_dist = "NEAR";
  int distance = proximity();
  if(distance > 150){
    near = false;
    str_dist = "FAR";
  }

  bool dark = true;
  String str_lum = "DARK";
  int lumosity = light();
  if(lumosity > 100){
    dark = false;
    str_lum = "BRIGHT";
  }

  Serial.println(String("Raw Values: mic=") + level + ", clear=" + lumosity + ", motion=" + movement + ", prox=" + distance);
  Serial.println(String("Flags: sound=") + !quiet + ", dark=" + dark + ", moving=" + !steady + ", near=" + near);
  String final_label = str_lev +"_"+ str_lum +"_"+ str_mov +"_"+ str_dist;
  Serial.println(final_label);  

  delay(200);
}
