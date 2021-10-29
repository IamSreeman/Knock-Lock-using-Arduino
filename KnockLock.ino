/*
   Analog Pin 0: Piezo sensor with 1M pulldown resistor connected across it
  
   Digital Pin 2: reset button for new password
   Digital Pin 3: servo motor
   Digital Pin 4: Red LED. 
   Digital Pin 5: Green LED. 
   
   Digital Pin 6: connected to pin 11 of LCD
   Digital Pin 7: connected to pin 12 of LCD
   Digital Pin 8: connected to pin 13 of LCD
   Digital Pin 9: connected to signal pin of servo motor
   Digital Pin 10: connected to pin 14 of LCD
   Digital Pin 11: connected to pin 6 of LCD
   Digital Pin 12: connected to pin 4 of LCD. 
  
 */

#include <Servo.h>
#include <LiquidCrystal.h>
Servo myservo;  // servo object to control a servo
 
// constants


const int Fade_Time = 150;     // time interval in milliseconds before we listen for another one/ Debounce timer
const int Threshold_Value = 80;           // Minimum knock strength to be able to be detected
const int Reject_Value = 75;        // acceptable error in the knock signal
const int Avg_Reject_Value = 50; // acceptable error in Avg_ timing of the knocks
const int Turn_Time = 8000;      // time interval for which servo motor will be active
const int maxCount = 20;       // Maximum number of knocks
const int knock_Time = 1200;     // maximum time for a knock to be completed

// variables.
int count = 0;
int oldcount = 0;
int Sensor_readingValue = 0;           // Last reading of the knock sensor.
int Reset_Button = false;   // to check if the reset button is pressed
int Secret_Code[maxCount] = {50, 50, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // default password
int New_knocks[maxCount];   // When the person knocks above array fills with delays between new knocks.

// Pins
const int Sensor_reading = 0;         // Piezo sensor on pin 0.
const int button_Pressed = 2;       // program a new code if value if HIGH.
const int Servo_Motor = 3;           // to rotate servo motor.
const int R_LED = 4;              // Red LED
const int G_LED = 5;            // Green LED

const int rs = 12, en = 11, d4 = 6, d5 = 7, d6 = 8, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  lcd.begin(16, 2);
  
  pinMode(Servo_Motor, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(button_Pressed, INPUT);
  
  Serial.begin(9600);                     
  Serial.println("Program start.");      
  lcd.print("Program start.");
  digitalWrite(G_LED, HIGH);      // tells the status of circuit
  lcd.clear();
}

void loop() {
    
  
  Sensor_readingValue = analogRead(Sensor_reading);  // Listen for the knock
  
  if (digitalRead(button_Pressed)==HIGH){     // if reset button is pressed
    Reset_Button = true;          
    digitalWrite(R_LED, HIGH);           // to tell the status of running program
  } else {
    Reset_Button = false;
    digitalWrite(R_LED, LOW);
  }
  
  if (Sensor_readingValue >=Threshold_Value){
    knock_Listener();
  }
  if(count % 5 ==0 && count >oldcount)                             // if number of failed attempts reach maximum value
    {
      Serial.print("Door Locked. Try again in");
      Serial.print(count*6);
      Serial.println("seconds"); // lock the door for count*6 second and stop hearing the knock
      lcd.print("Door Locked");
      for(int i = count*6; i>0; i= i-1){
        lcd.setCursor(0, 1);
        lcd.print(i);
        delay(1000);
//        lcd.clear();
        lcd.setCursor(0,1);
        }
      
//      delay(30000);
      lcd.clear();                                  
      Serial.println("Try again");            //start listening after 30 second
      lcd.print("Try again");
      oldcount=count;          
    }
    
} 

void unlock_Door(){            //function for running servo motor
  count = 0;
  Serial.println("Voila,enter now!"); // Door unlocks
  lcd.clear();
  lcd.print("Voila,enter now!");
  digitalWrite(Servo_Motor, HIGH);
  digitalWrite(G_LED, HIGH);            
  
  myservo.write(0);              // tell servo to go to 110 degree angle
  delay(5000);                   // close dore after 5 second
  lcd.clear();   
  myservo.write(180);               // servo motor returns to initial position
  Serial.println("Door Locked");  // Door locks again
  lcd.print("Door Locked");
  delay(400);
  lcd.clear();
  digitalWrite(Servo_Motor, LOW);            // Turn the motor off.
  
  for (int i=0; i < 5; i++){              //Green LED blinks for visual indication
      digitalWrite(G_LED, LOW);
      delay(100);
      digitalWrite(G_LED, HIGH);
      delay(100);
  }
   
}

// Records the timing of knocks.
void knock_Listener(){
  Serial.println("knock starting");   
//  lcd.print("knock starting");
  int i = 0;
  // First lets reset the listening array.
  for (i=0;i<maxCount;i++){
    New_knocks[i]=0;
  }
  
  int currentKnockNumber=0;               // Incrementer for the array.
  int startTime=millis();                 // starting time of the knock 
  int now=millis();
  
  digitalWrite(G_LED, LOW);            // blink the LED for indicating the knock
  if (Reset_Button==true){
     digitalWrite(R_LED, LOW);                        
  }
  delay(Fade_Time);                                 // wait before we listen to the next knock.
  digitalWrite(G_LED, HIGH);  
  if (Reset_Button==true){
     digitalWrite(R_LED, HIGH);                        
  }
  do {
    //listen for the next knock or wait for it to timeout. 
    Sensor_readingValue = analogRead(Sensor_reading);
    if (Sensor_readingValue >=Threshold_Value){                   //if we get another knock
      //record the delay time.
      Serial.println("knock.");
      now=millis();
      New_knocks[currentKnockNumber] = now-startTime;
      currentKnockNumber ++;                             //increase the counter value
      startTime=now;          
      // and reset our timer for the next knock
      digitalWrite(G_LED, LOW);  
      if (Reset_Button==true){
        digitalWrite(R_LED, LOW);                      
      }
      delay(Fade_Time);                              // wait for knock to die
      digitalWrite(G_LED, HIGH);
      if (Reset_Button==true){
        digitalWrite(R_LED, HIGH);                         
      }
    }

    now=millis();
    
  
  } while ((now-startTime < knock_Time) && (currentKnockNumber < maxCount));  //check if time exceeded or we reached maximum number of knocks
  
 
  if (Reset_Button==false){             // if reset switch is not pressed
    if (Is_password_correct() == true){              // if password is correct (valid knock pattern)
      unlock_Door();                     // open the door
    } else {
      count++;
      Serial.println("Wrong Password");    // wrong password
      lcd.clear();
      lcd.print("Wrong Password");
      digitalWrite(G_LED, LOW);               // red led indicates the status
      for (i=0;i<4;i++){          
        digitalWrite(R_LED, HIGH);
        delay(100);
        digitalWrite(R_LED, LOW);
        delay(100);
      }
      digitalWrite(G_LED, HIGH);
      delay(400);
      lcd.clear();
    }
  } else {                          // if reset switch is pressed 
    Is_password_correct();                  // blinking the green and red LEDs in alternate
    Serial.println("paswword changed successfully.");
    lcd.print("paswword changed");
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, HIGH);
    for (i=0;i<3;i++){
      delay(100);
      digitalWrite(R_LED, HIGH);
      digitalWrite(G_LED, LOW);
      delay(100);
      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, HIGH);      
    }
  }
}

boolean Is_password_correct(){                 // validate the knock and returns true of false
 
  int current_count = 0;
  int secret_Count = 0;
  int max_Interval = 0;               // We use this later to normalize the times.
  
  for (int i=0;i<maxCount;i++){
    if (New_knocks[i] > 0){
      current_count++;
    }
    if (Secret_Code[i] > 0){          
      secret_Count++;
    }
    
    if (New_knocks[i] > max_Interval){   // collect normalization data while we're looping.
      max_Interval = New_knocks[i];
    }
  }
  
  if (Reset_Button==true){                 //if reset button is pressed we will save information
      for (int i=0;i<maxCount;i++){               // normalise the times
        Secret_Code[i]= map(New_knocks[i],0, max_Interval, 0, 100); 
      }
    
      digitalWrite(G_LED, LOW);                       // flash LED to show information is geeting stored
      digitalWrite(R_LED, LOW);
      delay(1000);
      digitalWrite(G_LED, HIGH);
      digitalWrite(R_LED, HIGH);
      delay(50);
      for (int i = 0; i < maxCount ; i++){
        digitalWrite(G_LED, LOW);
        digitalWrite(R_LED, LOW);  
                                                           // only turn it on if there's a delay
        if (Secret_Code[i] > 0){                                   
          delay( map(Secret_Code[i],0, 100, 0, max_Interval)); // Expand the time back out to what it was.  Roughly. 
          digitalWrite(G_LED, HIGH);
          digitalWrite(R_LED, HIGH);
        }
        delay(50);
      }
    return false;   // dont unlock the door if new pasword is being stored
  }
  
  if (current_count != secret_Count){
    return false; 
  }
  
  //Now store the time interval between knocks
  int total_Time=0;
  int time_Difference=0;
  for (int i=0;i<maxCount;i++){                            // Normalize the times
    New_knocks[i]= map(New_knocks[i],0, max_Interval, 0, 100);      
    time_Difference = abs(New_knocks[i]-Secret_Code[i]);
    if (time_Difference > Reject_Value){                           //time interval is greater than rejec value Threshold_Value
      return false;
    }
    total_Time += time_Difference;
  }

  if (total_Time/secret_Count>Avg_Reject_Value){               //checking for Avg_ reject value constraint
    return false; 
  }
  
  return true;
  
}
