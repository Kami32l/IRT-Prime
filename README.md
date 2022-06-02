# IRT-Prime

Intelligent Robot (IRT) Prime

This is my first robot. IRT Prime is based on a d1 mini and an Arduino Pro Micro. It's controlled over the internet using the Blynk app. It uses an ultrasonic sensor to prevent hitting objects when going forward. IRT Prime has four wheels and a left/right drive split that allows to rotate basically in place. It is powered by three lithium ion cells which make it possible to play with the robot for a long time.

# Circuit
![obraz](https://user-images.githubusercontent.com/79849248/120123243-03f83300-c1ae-11eb-8bcc-b6e7d204d58f.png)

# Code
D1 mini - esp8266
'''
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

/*ESP8266 PINOUT
  D0  16 --high at boot
  D1  5
  D2  4
  D3  0 --boot fails if low, flash
  D4  2 high at boot, boot fails if low, led
  D5  14
  D6  12
  D7  13
  D8  15 --boot fails if high
  TX  1
  RX  3
*/

char auth[] = "blynk-token";                        // Blynk token
char ssid[] = "wifi-name";                          // WiFi name
char pass[] = "wifi-password";                      // WiFi password

#define in1 16              // =======================================
#define in2 12              // output pins to l298n with informations
#define in3 0               // about direction of motors
#define in4 2               // =======================================
#define pwm_pin 13          // output pin to pro micro with information about speed
#define relay 14            // output pin to relay turns on or off motors
#define too_close 15        // output pin to pro micro with information if there is something too close

const int trigPin = 5;      // sensor trigger pin
const int echoPin = 4;      // sensor echo pin

int speed_from_blynk;       // slider value from Blynk
int x, y;                   // joystick axis data from Blynk
int stopped;                // information if IRT is stopped
int percentage;             // pwm to percentage
int memory;                 // remembers last direction
int relay_state;            // relay state
int times_stopped;          // remembers how many times IRT is stopped
int Start;                  //

String to_text;             // int to string type
String percentage_text;     // percentage text which is sent to blynk

long duration;              // information about how long it takes for the sound wave to return to the sensor
int distance;               // distance to nearest obstacle

unsigned long time_since_start; // time since start
unsigned long last_time;        // timestamp of last event 
unsigned long time_in_memory;

void setup() {
  pinMode(in1, OUTPUT);          // ====================
  pinMode(in2, OUTPUT);          //   
  pinMode(in3, OUTPUT);          //
  pinMode(in4, OUTPUT);          //  
  pinMode(pwm_pin, OUTPUT);      //  define pins modes
  pinMode(too_close, OUTPUT);    //
  pinMode(trigPin, OUTPUT);      //
  pinMode(echoPin, INPUT);       // 
  pinMode(relay, OUTPUT);        // ====================
  digitalWrite(relay, HIGH);     // turn off the relay
  Start = 1;                     // writes 1 to "start" variable
  Serial.begin(9600);            // starts serial monitor with predefined speed

  Blynk.begin(auth, ssid, pass); // connects to Blynk
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

}

void sensor() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculates distance
  distance = duration * 0.034 / 2;

  // if distance is lower than 200cm sends it to Blynk
  if (distance <= 200) {
    Blynk.virtualWrite(V2, distance);
  }

  // if distance is higher than 200cm sends to Blynk "Out of reach"
  else if (distance > 200) {
    char a[] = "Out of reach";
    Blynk.virtualWrite(V2, a);
  }

  // if distance is lower or equal to 35cm
  if (distance <= 35) {
      if (memory == 1){
        if (times_stopped < 3){                               // limits to 3 the number of times irt can perform an emergency reversal. In case it gets stuck
           if(time_since_start - time_in_memory >= 3000){
              time_in_memory = time_since_start;
              times_stopped ++;
           }
        digitalWrite(too_close, HIGH);           // sends information to pro micro that something is too close
        stopped = 1;                             // writes 1 to "stopped" variable, prevent from changing the direction of rotation of the motors 
        Blynk.virtualWrite(V4, "Em. reverse");   // sends text to Blynk lcd
        Blynk.virtualWrite(V5, "speed: 0%");     // sends text to Blynk lcd
        int z[] = {130, 0};                      // =======================================
        Blynk.virtualWrite(V1, z[0]);            // sets Blynk Joystick to reverse position
        Blynk.virtualWrite(V10, z[1]);           // =======================================
        digitalWrite(in1, LOW);                  //  
        digitalWrite(in3, LOW);                  // sets motors to reverse
        digitalWrite(in2, HIGH);                 //
        digitalWrite(in4, HIGH);                 // =======================================
        }
        else if (times_stopped >= 3){             // IRT is probably stuck :(
          digitalWrite(too_close, LOW);           
          stopped = 0;                            // writes 0 to "stopped" variable, allows to change the direction of rotation of the motors
          Blynk.virtualWrite(V4, "IRT is stuck"); // sends text to Blynk lcd
          Blynk.virtualWrite(V5, "speed: 0%");    // sends text to Blynk lcd
        }
     }
  }

  // if distance is higher than 35cm
  else if (distance > 35) {
    digitalWrite(too_close, LOW);                 // sends information to pro micro that there is nothing within 35 cm
    stopped = 0;                                  // distance to closest object is lower 35cm = false
    times_stopped = 0;                            // resets "times_stopped" variable to 0
  }
}

void loop() {

  Blynk.run();                                     // starts Blynk

  if (Start == 1){                                 // 
    Blynk.virtualWrite(V0, 1);                     // When d1 mini is turned on for the first time, it sets button "MOTORS" in Blynk App to OFF state
    Start = 0;                                     //
  }
  
  time_since_start = millis();                     // saves time since start to variable

  // runs sensor() function every 200 milliseconds
  if (time_since_start - last_time >= 100) {
    last_time = time_since_start;
    sensor();
  }
}
  // reads relay state from Blynk App, sets relay to this state
BLYNK_WRITE(V0) {
  relay_state = param.asInt();
  digitalWrite(relay, relay_state);
}
BLYNK_WRITE(V3) {
  speed_from_blynk = param.asInt();                  // reads speed from Blynk slider
  percentage = speed_from_blynk * 10 / 102;          // calculates speed to percentage
  to_text = String(percentage);                      // change percentage to string type
  percentage_text = "speed: " + to_text + "%";  
  Blynk.virtualWrite(V5, percentage_text);           // sends percentage to Blynk lcd
  analogWrite(pwm_pin, speed_from_blynk);            // sends speed to pro micro


}

void motor_dir(){
  
  // forward function
  if (x >= 128 && y >= 55 && y <= 200 && stopped == 0) {
    Blynk.virtualWrite(V4, "");
    digitalWrite(in2, LOW);
    digitalWrite(in4, LOW);
    digitalWrite(in1, HIGH);
    digitalWrite(in3, HIGH);
    Blynk.virtualWrite(V4, "Forward ");
    memory = 1;
  }
  
  // reverse function
  else if (x < 128 && y >= 55 && y <= 200 && stopped == 0) {
    Blynk.virtualWrite(V4, "");    
    digitalWrite(in1, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in4, HIGH);
    Blynk.virtualWrite(V4, "Reverse ");
    memory = 2;
  }
  
  // left turn function
  else if (y < 55 && x > 0 && x < 255 && stopped == 0) {
    Blynk.virtualWrite(V4, "");
    digitalWrite(in1, LOW);
    digitalWrite(in4, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    Blynk.virtualWrite(V4, "Left turn");
    memory = 3;
  }

  // right turn function
  else if (y > 200 && x > 0 && x < 255 && stopped == 0) {
    Blynk.virtualWrite(V4, "");
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in1, HIGH);
    digitalWrite(in4, HIGH);
    Blynk.virtualWrite(V4, "Right turn");
    memory = 4;
  }
}

BLYNK_WRITE(V1) {
  y = param.asInt();                  // reads y value from Blynk Joystick
}

BLYNK_WRITE(V10) {
  x = param.asInt();                  // reads x value from Blynk Joystick
  motor_dir();                        // calls motor_dir() function
}
'''
