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
