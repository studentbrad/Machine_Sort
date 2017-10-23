#include "HX711.h" //library for load cell
#include <Servo.h> //library for servos

HX711 cell(3, 2); //tell the program that there is a HX711 loadcell called cell 
Servo myservo1; //tell the program that there is a servo called myservo1
Servo myservo2; //tell the program that there is a servo called myservo2

#define COUNT_OVERFLOW 20 //the length of mylist which is 20

//Servo starting and ending positions ex: servo 1 starts at 87 degrees and ends at 175 degrees
#define SERVO1START 87 //start angle for servo 1
#define SERVO1END 175 //end angle for servo 1
#define SERVO2START 7 //start angle for servo 2
#define SERVO2END1 35 //type 1 end angle
#define SERVO2END2 62 //type 2 end angle
#define SERVO2END3 90 //type 3 end angle

//Define variables
long val = 0;
long mylist[COUNT_OVERFLOW]; //a list that stores 20 values read from the load cell to eventually be averaged
int i; //an integer for for loops in the code
int count = 0; //a counter for loops in the code
bool rec[3] = {false,false,false}; //this is a list to keep track of if the weights of the balls have been recorded.  If all weights are recorded rec = {true,true,true}
long weights[3] = {0,0,0}; //this is a list of all of the recorded weights.  One weight should be the wood, one should be the gum and one should be the marble
bool seprRec = false; //this will become true once the seperators (1&2) are calculated
long sepr1 , sepr2; //these are the seperator values
char serial_in; //this is a value that holds the letter that is input from the computer ('1' to record weight 1, '2' to record weight 2, '3' to record weight 3)

int process; //this variable keeps track of what the system should be doing.  If it is 1 then the system should be weighing the balls or calculating seperators.  If it is 2 the system should be sorting marbles

int servo1pos , servo2pos; //variables to store the servo positions

void setup() {
  Serial.begin(9600); //set up communication with the computer

  //setting up servos to their pins
  myservo1.attach(9);
  myservo2.attach(10);
  
  Serial.println("Weighing process has begun...");
  Serial.println("Enter '1','2' or '3' to record the weight of 3 balls...");
  Serial.println("Once all weights are recorded the program will start...");
  process = 1; //this is set to 1 so that the system is in the weighing process
}

void loop() {

  //WEIGHING PROCESS (recording weights of 3 balls)
  if(process == 1){
    //this loop takes the average of 20 values read from the load cell and loads them into a list of length 20
    count = count + 1;
    if(count == COUNT_OVERFLOW){ //if the list is full
      count = 0; //reset counter for the loop
      val=0; //reset val to 0
      //this for loop adds all of the values in the list mylist
      for(i=0;i<COUNT_OVERFLOW;i++){
        val = val+mylist[i]; //add all the values in mylist
      }
      val = val/COUNT_OVERFLOW; //divide the sum by 20 to get the average
    }
    
    mylist[count] = cell.read(); // add the most recent reading from the load cell to the list mylist
    
    //recording weights
    serial_in = Serial.read();
    if(serial_in == '1'){ //if the computer sends '1' then this code runs and records the weight from the load cell into weights[0]
      weights[0] = val;
      rec[0] = true;
      Serial.println("Weight 1 recorded!");
    }
    if(serial_in == '2'){ //if the computer sends '2' then this code runs and records the weight from the load cell into weights[1]
      weights[1] = val;
      rec[1] = true;
      Serial.println("Weight 2 recorded!");
    }
    if(serial_in == '3'){ //if the computer sends '3' then this code runs and records the weight from the load cell into weights[2]
      weights[2] = val;
      rec[2] = true;
      Serial.println("Weight 3 recorded!");
    }
  
    //finding midpoints
    if(rec[0] && rec[1] && rec[2] && !seprRec){
      sort(weights,3);
      sepr1 = (weights[0] + weights[1])/2; //calculate seperator 1
      sepr2 = (weights[1] + weights[2])/2; //calculate seperator 2
      seprRec = true; //this indicates that the seperators have been recorded
      Serial.println("All weights have been recorded...");
      Serial.println("Main program starting...");
      process = 2; //change the process to 2 so that the system starts sorting the balls
    }
  }

  //If process = 2 then the system will run this code which sorts the balls
  else if(process == 2){
    //STEP1 & STEP2: servo 1 pull and servo 2 pull
    servo1pos = SERVO1START;
    servo2pos = SERVO2START;
    myservo1.write(servo1pos);
    myservo2.write(servo2pos);

    //delay for 1 second
    delay(1000);

    //STEP3: servo 1 push
    servo1pos = SERVO1END;
    myservo1.write(servo1pos);

    //delay for 4 seconds to allow for time for ball to be weighed
    delay(4000);

    //STEP4: weighing
    count = 0;
    //this loop takes an average of 20 values to get a more accurate weight
    while(count<COUNT_OVERFLOW){ //while the list is not yet full
      mylist[count] = cell.read(); // store each of the 20 values read from the load cell into a list that is of length 20
      count = count + 1;
    }
    val = 0;
    for(i=0;i<COUNT_OVERFLOW;i++){
      val = val+mylist[i]; //add all the 20 values the were read by the load cell
    }
    val = val/COUNT_OVERFLOW; //divide the sum by 20

    //determine if the value read is less than seperator 1, inbetween seperator 1 and 2, or greater than seperator 2
    if(val<sepr1){
      Serial.println("Type 1 detected...");
      servo2pos = SERVO2END1;
    }
    else if(val>=sepr1 && val<=sepr2){
      Serial.println("Type 2 detected...");
      servo2pos = SERVO2END2;
    }
    else if(val>sepr2){
      Serial.println("Type 3 detected...");
      servo2pos = SERVO2END3;
    }
  
    //STEP5: servo 2 push
    myservo2.write(servo2pos);

    //delay for 2 seconds
    delay(2000);
    }
  }
  //END
  
}

//I got this algorithum from google
//This algorithum sorts a list from least to greatest value
//SORT ALGORITHUM FROM http://www.hackshed.co.uk/arduino-sorting-array-integers-with-a-bubble-sort-algorithm/
void sort(long a[], int size) {
    for(int i=0; i<(size-1); i++) {
        for(int o=0; o<(size-(i+1)); o++) {
                if(a[o] > a[o+1]) {
                    long t = a[o];
                    a[o] = a[o+1];
                    a[o+1] = t;
                }
        }
    }
}
