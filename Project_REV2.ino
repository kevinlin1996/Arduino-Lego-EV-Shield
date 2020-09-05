//MEC830 PROJECT
//SECTION 04
//GROUP 02
//FALL 2018


//Include statements
#include <Wire.h>
#include <EVShield.h>
#include <EVs_NXTCam.h>
#include <EVs_NXTTouch.h>
#include <EVs_NXTLight.h>
#include <USART.h>

//Constants

const int grey1x4 = 30;
const int red2x2 = 30;
const int red2x4 = 60;
const int blue2x4 = 90;

const int greyColour = 0;
const int redColour = 2;
const int blueColour = 3;

EVShield evshield(0x34, 0x36);
EVs_NXTCam myCam;  //make object myCam
EVs_NXTTouch myTouch;
EVs_NXTLight myLight;

// declare varibales

int n;
uint8_t i;
int nblobs;
uint8_t color[8];
uint8_t left[8];
uint8_t top[8];
uint8_t bottom[8];
uint8_t right[8];
uint16_t red_area_total;
uint16_t red_blob_area;
uint16_t blue_area_total;
uint16_t blue_blob_area;
uint16_t width;
uint16_t height;
uint8_t blockIdentity = 0;
//0 = 1x4 Grey, 1 = 2x2 Red, 2 = 2x4 Red, 3 = 2x4 Blue
uint8_t movement = 0;
boolean movementDirection = false;

void setup()
{

  initUSART();
  printString("starting NXTCam program");
  evshield.init( SH_HardwareI2C );
  myCam.init( &evshield, SH_BAS1 );

  // if there was previous run of this program,
  // the tracking might still be on, so disable it.
  myCam.disableTracking();

  // setup myCam for Object mode and sort by size.
  // also let it begin tracking.
  myCam.selectObjectMode();
  myCam.sortSize();
  myCam.enableTracking();
  delay(1000);

  //Motor port and touch port initialization
  myTouch.init(&evshield, SH_BAS1); //myTouch @ BAS1
  myLight.init(&evshield, SH_BAS2); //myLight @ BAS2
  evshield.bank_a.motorReset();
  evshield.bank_b.motorReset();
  //Will use bank B for all our motors
  //Motor 2 will be used for the reading arm
  //Motor 1 will be used for the sorting arm
}

void loop() {

  //CHANGE TO IR REFLECTIVE SENSOR. TEST MOTIONS WITH TOUCH SENSOR FIRST
  //To "activate" Cam reading and motors
  if (myTouch.isPressed() == true) {
    myCam.issueCommand('J'); // lock buffer

    //delay for NXT Cam to pick up blobs
    delay(1500);

    //NXT Cam get's blobs
    myCam.getBlobs(&nblobs, color, left, top, right, bottom);
    delay(500);

    myCam.issueCommand('K'); // unlock buffer

    printString("number of blobs: ");
    printWord(nblobs);
    printString("\r\n");

    //reset area values
    red_area_total = 0;
    red_blob_area = 0;
    blue_area_total = 0;
    blue_blob_area = 0;

    //reset block identity
    blockIdentity = 0;

    //reset movement stuff
    movement = 0;
    movementDirection = false; //false = CW, true = CCW


    //find total area of blobs
    for (i = 0; i < nblobs; i++) {

      //check colour & add area
      //red
      if (color[i] == redColour)  {
        printString("red blob found at");

        //red area
        width = abs(right[i] - left[i]);
        height = abs(top[i] - bottom[i]);
        red_blob_area = width * height;
        red_area_total = red_area_total + red_blob_area;
      }

      //blue
      if (color[i] == blueColour)  {
        printString("blue blob found");

        //blue area
        width = abs(right[i] - left[i]);
        height = abs(top[i] - bottom[i]);
        blue_blob_area = width * height;
        blue_area_total = blue_area_total + blue_blob_area;
      }
    }//end of for loop


    //Area calculations below

    //more blue area than red
    //NOTE: arbitrary safeguard area check is anded here to prevent false readings (change as nessicary)
    if ((blue_area_total > red_area_total) && (blue_area_total > 150)) {
      printString("Large Blue block found");
      blockIdentity = 3;
    }

    //More red area than blue
    else if (red_area_total > blue_area_total) {

      //large red block found
      //NOTE: arbitrary safeguard area check is anded here to prevent false readings (change as nessicary)
      if ((red_area_total > 1500) && (red_area_total > 150)) {
        printString("Big Red block found");
        blockIdentity = 2;
      }

      //small red block found
      //NOTE: arbitrary safeguard area check is anded here to prevent false readings (change as nessicary)
      else if ((red_area_total > 0 || red_area_total < 1500) && (red_area_total > 150)) {
        printString("Small red block found");
        blockIdentity = 1;
      }
    }
    
    //Grey block case
    else {
      printString("Large Grey block found");
      blockIdentity = 0;
    }
    //end of block identity section



    //Move sorting arm into position
    if (blockIdentity == 0) { //1x4 Grey Block, Bottom Cup #1
      evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Forward, SH_Speed_Slow, grey1x4, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
      movement = grey1x4;
      movementDirection = true;
    }
    else if (blockIdentity == 1) { //2x2 Red Block, Top Cup #1
      evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Reverse, SH_Speed_Slow, red2x2, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
      movement = red2x2;
      movementDirection = false;
    }
    else if (blockIdentity == 2) { //2x4 Red Block, Top Cup #2
      evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Reverse, SH_Speed_Slow, red2x4, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
      movement = red2x4;
      movementDirection = false;
    }
    else if (blockIdentity == 3) { //2x4 Blue Block, Top Cup #3
      evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Reverse, SH_Speed_Slow, blue2x4, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
      movement = blue2x4;
      movementDirection = false;
    }
    delay(1000);

    //Move holding arm out of the way
    evshield.bank_b.motorRunDegrees(SH_Motor_2, SH_Direction_Reverse, SH_Speed_Slow, 90, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);

    //Delay to drop part in
    //delay(5000);

    //Reset Motors back into position
    //Sorting Arm
    if (movementDirection) { //moved CW, must go CCW back
      evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Reverse, SH_Speed_Slow, movement, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
    }
    else if (!movementDirection) {
      evshield.bank_b.motorRunDegrees(SH_Motor_1, SH_Direction_Forward, SH_Speed_Slow, movement, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
    }

    //Reading Arm
    evshield.bank_b.motorRunDegrees(SH_Motor_2, SH_Direction_Forward, SH_Speed_Slow, 90, SH_Completion_Wait_For, SH_Next_Action_BrakeHold);
  }
}
