# IRT-Prime

Intelligent Robot (IRT) Prime

This is my first robot. IRT Prime is based on a d1 mini and an Arduino Pro Micro. It's controlled over the internet using the Blynk app. It uses an ultrasonic sensor to prevent hitting objects when going forward. IRT Prime has four wheels and a left/right drive split that allows to rotate basically in place. It is powered by three lithium ion cells which make it possible to play with the robot for a long time.


![Front](https://github.com/Kami32l/IRT-Prime/blob/d779ccd4c7a5ad7a5e18825907f0f6db0b69bd4f/Images/front.jpg)


# More Pictures 
![Back](https://github.com/Kami32l/IRT-Prime/blob/d779ccd4c7a5ad7a5e18825907f0f6db0b69bd4f/Images/back.jpg)
![Side](https://github.com/Kami32l/IRT-Prime/blob/d779ccd4c7a5ad7a5e18825907f0f6db0b69bd4f/Images/side.jpg)


# Video
https://user-images.githubusercontent.com/79849248/171730335-9e75d77a-3ca0-435b-b4fc-5338508c1e0f.mp4


# Functions
- It can go forward and backward, direction controlled by the joystick in the app.
- Rotates almost in place to the left and to the right.
- It has adjustable speed.
- You can turn off the power to the motors, and thus stop the robot from the app.
- It has RGB LED to indicate the current direction of travel and emergency reversing.

![Emergency Reversing](https://github.com/Kami32l/IRT-Prime/blob/07122da56d5d4ad407308184c4e5718f321d7758/Images/stopping%20IRT%20Prime.gif)
Emergency Reversing


# More Functions
- The app displays the current direction of travel and speed in percentage.
- It has a sensor that displays in the app the distance to the nearest obstacle located up to 200 cm in front of the robot.
- It has an Emergency Reversing mode - when the robot approaches an obstacle at a distance of less than 35 cm - it reverses until the robot moves away at a distance greater than 35 cm.
- It has protection against blocking - when the Emergency Reversing sequence is repeated 3 times, the robot stops reversing.

![Double Emergency Reversing](https://github.com/Kami32l/IRT-Prime/blob/07122da56d5d4ad407308184c4e5718f321d7758/Images/stopping%20too%20close%20IRT%20Prime.gif)
Double Emergency Reversing


# Blynk App
![Blynk](https://github.com/Kami32l/IRT-Prime/blob/07122da56d5d4ad407308184c4e5718f321d7758/Images/Blynk/MainLCDJoystick_white.png)
![Blynk](https://github.com/Kami32l/IRT-Prime/blob/07122da56d5d4ad407308184c4e5718f321d7758/Images/Blynk/ButtonSliderValueLabel_white.png)


# Circuit
![Circuit](https://user-images.githubusercontent.com/79849248/120123243-03f83300-c1ae-11eb-8bcc-b6e7d204d58f.png)


# Code
D1 mini - esp8266
```
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
```


Arduino Pro Micro
```
#define ena 9                                       // output pin to l298, sets the speed of motors
#define enb 3                                       // output pin to l298, sets the speed of motors
#define led_r 5                                     // output pin to led rgb, red color
#define led_g 6                                     // output pin to led rgb, green color
#define led_b 10                                    // output pin to led rgb, blue color
#define too_close 14                                // input pin with information if there is something to close from d1 mini
#define in4 4                                       // =======================================
#define in3 15                                      // input pins, reads informations
#define in2 8                                       // about direction of rotation of motors
#define in1 7                                       // =======================================

byte PWM_PIN = 2;                                       // input pin, reads speed sent by d1 mini
int received;                                           // ======================================
int pwm_value;                                          //
int distance;                                           // variables used for speed control
int pwm_in_memory;                                      //
int pwm;                                                // ======================================

unsigned long timer;                                    // this variable is equal time from start
unsigned long time_in_memory, time_in_memory2;          // these variables remembers time that was assigned to them, used in timer

void setup() {
  pinMode(ena, OUTPUT);               // =====================
  pinMode(enb, OUTPUT);               //
  pinMode(led_r, OUTPUT);             //
  pinMode(led_g, OUTPUT);             //
  pinMode(led_b, OUTPUT);             //  define pins modes
  pinMode(too_close, INPUT);          //  
  pinMode(in4, INPUT);                //
  pinMode(in3, INPUT);                //
  pinMode(in2, INPUT);                //
  pinMode(in1, INPUT);                // =====================

  digitalWrite(led_r, HIGH);          // ====================
  digitalWrite(led_g, HIGH);          //  turns off rgb led
  digitalWrite(led_b, HIGH);          // ====================
  Serial.begin(9600);                 // starts serial monitor with predefined speed
}

//read and assign speed sent by d1 mini
void speed_read(){
  received = pulseIn(PWM_PIN, HIGH);
  pwm_value = map(received, 0, 1020, 0, 255);
  Serial.println(pwm_value);
  if (pwm_value - pwm_in_memory > 5 || pwm_in_memory - pwm_value > 5){
    pwm = pwm_value;
    pwm_in_memory = pwm_value;
  }
  //Serial.println(pwm);               // for debugging
}
 
void loop() {
  //reads whether robot is moving forward, in reverse, rotate left or right.
  if (digitalRead(in1) == HIGH && digitalRead(in3) == HIGH && digitalRead(in2) == LOW && digitalRead(in4) == LOW){
    digitalWrite(led_r, HIGH);
    digitalWrite(led_g, LOW);
    digitalWrite(led_b, HIGH);
  }
  else if (digitalRead(in2) == HIGH && digitalRead(in4) == HIGH && digitalRead(in1) == LOW && digitalRead(in3) == LOW){
    digitalWrite(led_r, HIGH);
    digitalWrite(led_g, HIGH);
    digitalWrite(led_b, LOW);
  }
  else if (digitalRead(in2) == HIGH && digitalRead(in3) == HIGH && digitalRead(in1) == LOW && digitalRead(in4) == LOW){
    digitalWrite(led_r, LOW);
  }
  else if (digitalRead(in1) == HIGH && digitalRead(in4) == HIGH && digitalRead(in2) == LOW && digitalRead(in3) == LOW){
    analogWrite(led_r, LOW);
  }
  distance = digitalRead(too_close);                        // reads information whether someting is too clsoe send by d1 mini
  //Serial.print("Safe distance? 1 - No, 0 - Yes: ");       // for debugging
  //Serial.println(distance);                               // for debugging

  timer = millis();

  // function that defines how often speed is read
  if(timer - time_in_memory2 >= 50 && distance == 0){
    speed_read();
  }
  // controls what will happen when something is too close
  while(distance == 1){
      digitalWrite(led_g, HIGH);
      digitalWrite(led_b, HIGH);
      digitalWrite(led_r, LOW);
      timer = millis();
      analogWrite(ena, 0);                          // stops motors
      analogWrite(enb, 0);                          // stops motors
      distance = digitalRead(too_close);            // reads distance again
      delay(15);
      // function that do emergency reversing
      if(timer - time_in_memory >= 3000 && distance == 1){
        time_in_memory = timer;
        pwm = 0;
        delay(1000);                                // defines for how long robot will be stopped
        analogWrite(ena, 127);
        analogWrite(enb, 127);
        delay(500);                                 // defines for how long robot will be driving in reverse
        analogWrite(ena, 0);
        analogWrite(enb, 0);
      }
    }

  // writes speed to motors
  if (pwm >= 50){
  analogWrite(ena, pwm);
  analogWrite(enb, pwm);
  }
  // turns off motors if speed sent by d1 mini is too small
  else if (pwm < 50){
  digitalWrite(ena, LOW);
  digitalWrite(enb, LOW);
  } 
}
```
