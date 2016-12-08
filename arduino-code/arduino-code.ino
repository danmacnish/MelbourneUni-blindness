//this code uses a stepper to control the voltage output by the variac.
//Hall effect sensor is used to find the home position on startup.
//stepper motor is stepped with an exponentially decreasing delay between steps, to compensate for non-linear reponse of switchglass.

 const int stepDirection = 7;
 const int stepPin = 6;
 const int hallSensor = 3;
 const float MAX_VOLTAGE = 108; //55 //max voltage we want to go to
 const int NUM_STEPS = 703; //358 //number of steps from 0 to MAX_VOLTAGE
 const bool CW = false; //clockwise motor direction
 const bool CCW = true;
 const bool debugEnable = false;
 const int GLASS_CLEAR_DELAY = 10000;
 const int GLASS_OPAQUE_DELAY = 2000;
 const float DT_CONSTANT = 0.1;
 const int DT_SCALAR = 10;
 
 
 void setup()
 {
   Serial.begin(9600); //for debug
   Serial.setTimeout(1); //set time to wait for serial input to 1ms
   //while(!Serial) {
   // ;; //wait for serial connection to open (delete after finished debugging)
   //}
   pinMode(stepDirection,OUTPUT);
   pinMode(stepPin,OUTPUT);
   pinMode(hallSensor, INPUT); //for homing/initialisation

   //initialise outputs
   digitalWrite(stepDirection,CCW);
   digitalWrite(stepPin,HIGH);

 }
void loop()
 {
   
   static int* stepCount = 0; //counts number of steps
   static int scalar = DT_SCALAR; //scales delay between steps
   static int dT[NUM_STEPS]; //array to store the delay between steps
   
   Serial.println("initialising dT array...");
   //initialise array of dT, the delay between steps
   fillArray(dT);
   
   //find home position
   Serial.println("finding home position...");
   findHome(stepCount);


   //cycle from opaque to clear and back again, delaying dT[x] between steps. dT is scaled by value read from serial, so that we can adjust how long it takes to go from clear to opaque.
   while(1) {
    //cycle from opaque to clear
    scalar = 90;
    Serial.println("increasing voltage...");
    digitalWrite(stepDirection,CW); //change motor direction
    for(int i = 0; i < NUM_STEPS; ++i) {
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        if(debugEnable) {
            int val = Serial.parseInt(); //read int from serial port
            if(val > 0) {
              scalar = val; //set scalar to val if val > 0
            }
        }
    }

    delay(GLASS_CLEAR_DELAY); //wait for one second
    digitalWrite(stepDirection,CCW); //change motor direction
    
    //cycle from clear to opaque
    scalar = 15;
    Serial.println("decreasing voltage...");
    for(int i = NUM_STEPS - 1; i >= 0; --i) {
        if(!digitalRead(hallSensor)) { //if hall effect sensor triggered
          i = -1; //exit for loop step the other way
        }
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        digitalWrite(stepPin, LOW); //Step motor
        delay(dT[i]/scalar);
        digitalWrite(stepPin, HIGH);
        delay(1);
        if(debugEnable) {
            int val = Serial.parseInt(); //read int from serial port
            if(val > 0) {
              scalar = val; //set scalar to val if val > 0
            }
        }
    }
    delay(GLASS_OPAQUE_DELAY); //wait for one second
   }
 
 }

void findHome(int *count) {

    digitalWrite(stepDirection,CCW); //set motor to go towards home position
    bool homeFound = false;
    while(!homeFound) {
      if(digitalRead(hallSensor)) { //if hall effect sensor not triggered
        digitalWrite(stepPin, LOW); //Step motor
        delayMicroseconds(1500);
        digitalWrite(stepPin, HIGH);
        delay(1);
      } else { //else if home position found, reset step counter
        *count = 0;
        homeFound = true;
        Serial.println("home position found.");
      }
    }
}

void fillArray(int *dT) {
  //fill array of size NUM_STEPS, with the delay between steps. delay is exponentially decaying.
  float dV = MAX_VOLTAGE/static_cast<float>(NUM_STEPS); //find voltage increment per step
  Serial.println(dV);
  for(int i = 0; i < 33; ++i) // set initial part of array to low value, so that stepper covers first 5 volts quickly
  {
    dT[i] = 200;
  }
  for(int i = 33; i < NUM_STEPS; ++i) {
    float voltage = i*dV; //calculate voltage
    float opacity = ((log(voltage)-log(6.1))/0.0263) - 25;//calculate opacity @ voltage
    //Serial.println(opacity);
    if(opacity < 0) { //threshold opacity to > 0
      opacity = 0;
    }
    dT[i] = 1000*dV/(0.16043*pow(2.71828,(DT_CONSTANT*opacity)));  //calculate delay between steps
    Serial.println(dT[i]);  
  }
  Serial.println("finished filling dT array.");
 }

