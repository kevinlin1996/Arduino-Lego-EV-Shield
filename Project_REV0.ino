//EVShield NXTcam
#include <Wire.h>
#include <EVShield.h>
#include <EVs_NXTCam.h>
#include <USART.h>


EVShield    evshield(0x34, 0x36);
EVs_NXTCam myCam;  //make object myCam

// declare varibales
int         n;
uint8_t         color[8];
uint8_t         left[8];
uint8_t         top[8];
uint8_t         bottom[8];
uint8_t         right[8];


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
}


void loop()
{
  myCam.issueCommand('J'); // lock buffer
  delay(500);

  myCam.getBlobs(&nblobs, color, left, top, right, bottom);
  delay(500);

  myCam.issueCommand('K'); // unlock buffer

  printString("number of blobs: ");
  printWord(nblobs);
  printString("\r\n");

  uint8_t i;
  uint8_t block_colour = 0;
  //1 is red, 2 is blue, 0 is grey
  uint16_t total_blob_area = 0;
  uint16_t width;
  uint16_t height;

  //find total area of blobs
  for (int i = 0; i < nblobs; i++) {
    width = abs(right[i] - left[i]);
    height = abs(top[i] - bottom[i]);
    blob_area = width * height;
    total_blob_area = total_blob_area + blob_area;

    //check colour
    //red
    if (color[i] == 1)  {
      printString("red blob found at");
      block_colour = 1;
    }
    //blue
    if (color[i] == 2)  {
      printString("blue blob found");
      block_colour = 2;
    }
  }//end of for loop

  //delay for reading
  delay(8000);

  //big blob found
  if (total_blob_area > 1500) {
    printString("Big block found");

  }
  //small blob found
  else if (total_blob_area > 0 || total_blob_area < 1500) {
    printString("small block found");
  }
  else {

  }

  //insert motor control stuff here

}



}
