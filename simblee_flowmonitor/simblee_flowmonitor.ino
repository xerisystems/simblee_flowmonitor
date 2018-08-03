/*
 * SIMPLE SIMBLEE FLOW MONITOR
 * 
 * This sketch reads from a FS400A Water Flow Sensor and displays the outputs via the Simblee mobile app.
 * 
 * Instantaneous flow (L/hr) is shown along with previous recordings of total volume (L). The Simblee display code
 * is optimised for an iPhone 8. The Simblee App works across platforms but I have not tested on other screen
 * sizes or operating systems.
 * 
 * What was the origin of this project?
 * The original purpose of this project was to try and find out why a water tank was being emptied. The
 * pump seemed to go on at random times.  I wanted to determine if there was a pattern and how much water 
 * was being lost each time it came on.
 * 
 * What could be improved?
 * This is my first Simblee project. The main aim of this iteration was just to get it working. As such, the code
 * is pretty messy and could definately be improved!
 * I'm also not totally convinced that the calirbation of the flow sensor is right. This probably could be adjusted
 * in code to avoid the tril/error/recompile process.
 * 
 * Author: XeriSytems 2018
 * 
 * References:
 *  Sparkfun Simblee Breakout can be purchased here: https://www.sparkfun.com/products/13632
 *  Flow Sensor can be purchased here: https://www.ebay.com.au/itm/FS400A-G1-1-2Mpa-Water-Flow-Sensor-Hall-Effect-Flowmeter-Counter-1-60L-min-/152172303810
 *  Basic flow sensor code can be found here: http://www.hobbytronics.co.uk/download/YF-S201.ino
 *  Some info on how to calibrate a flow sensor: 
 *      https://github.com/sekdiy/FlowMeter/wiki/Calibration
 *      http://wiki.seeedstudio.com/G3-4_Water_Flow_sensor/
 *      https://www.electroschematics.com/12145/working-with-water-flow-sensors-arduino/
 *      https://maker.pro/arduino/tutorial/how-to-interface-arduino-with-flow-rate-sensor-to-measure-liquid
*/

#include <SimbleeForMobile.h>

int led = 2;                  // LED on Pin 2 for Sparkfun breakout

volatile int flow_frequency;  // Measures flow sensor pulses
float l_hour=0.0;             // Calculated litres/hour
uint8_t flowRateLabelId;
uint8_t flowsensor = 8;       // Sensor Input Pin

unsigned long currentTime;
unsigned long cloopTime;      // Counter loop time for recording flow
unsigned long startTime=0;    // Records time when flow detected
unsigned long stopTime=0;     // Records time when flow stopped

float IntervalVolume=0.0;     // Running total of volume for each interval reading
float TotalVolume=0.0;        // Running total of total volume since start
uint16_t TotalVolumeId;

// These are used to define the total grid layout for the phone screen. It's basically a really crude grid layout
const uint8_t GridRows = 11;  // Calibrated for an iPhone 8
const uint8_t GridCols = 8;   // Calibrated for an iPhone 8
uint16_t      GridRowX[GridRows];                   // Holds the x co-ords of each row (in points)
uint16_t      GridColY[GridCols];                   // Holds the y co-ords of each col (in points)

// NOTE: iPhone 8 Screen Units = 414 wide by 736 high Source: https://www.paintcodeapp.com/news/ultimate-guide-to-iphone-resolutions

// These are used to define the layout of the data table (which is a subset of the total screen)
const uint8_t TableRows = 6;
const uint8_t TableCols = 4;
uint16_t      TableRowX[TableRows];
uint16_t      TableColY[TableCols];

uint16_t      TableDataId[TableRows][TableCols];   // Arrary to hold id of each cell in the table for updating
float      TableData[TableRows][TableCols];        // Arrary to hold data for ach cell in the table

uint8_t       TableRowCounter = 0;                 // Used to refresh rows in table
uint16_t      DataCounter = 0;                     // Used to count total numer of data readings


// Interrupt function to count sensor pulses
int flow (uint32_t dummyPin)                      
{
   flow_frequency++;
}


void setup() {

    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH);                      // Turn LED on just to indicate things are working

    pinMode(flowsensor, INPUT);
    digitalWrite(flowsensor, HIGH);               // Optional Internal Pull-Up
    
    //Serial.begin(9600);
    
    attachPinInterrupt(flowsensor, flow, RISING); // Setup Interrupt

    delay(500);                                   // Add delay for things to settle a bit

    // Reset table data for initial view when mobile app starts
    for (int row=0; row < TableRows; row++) {
        for (int col=0; col < TableCols; col++) {
            TableData[row][col] = 0;                   
        }
    }

    currentTime = millis();
    cloopTime = currentTime;
    
    SimbleeForMobile.advertisementInterval = 675;
    SimbleeForMobile.deviceName = "Xeri";
    SimbleeForMobile.advertisementData = "Flow Sensor";
    
    // use a shared cache
    SimbleeForMobile.domain = "xerisystems.com";
    
    SimbleeForMobile.begin();

    delay(500);
    digitalWrite(led, LOW);                   // Turn LED off now
    
    flow_frequency = 0;                       // Reset flow frequency ie set number of pulses recorded to zero
}


void loop ()
// This loop basically takes a reading every second. It only records the total volume (between a start and stop time) when it detects flow has stopped.
{

   currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {
      cloopTime = currentTime; // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour Ref: https://github.com/sekdiy/FlowMeter/wiki/Calibration

            // This is important: before writing *any* UI element, make sure that the UI
            // is updatable!!! Failure to do so may crash your whole program.
            if (SimbleeForMobile.updatable)
            {
                SimbleeForMobile.updateValue(flowRateLabelId, l_hour);
            }
            if (l_hour > 0)
            {
                if (!startTime) { 
                  startTime = currentTime;                    // Only record start time once (after it is reset and flow detected)
                  digitalWrite(led, HIGH);
                }  
                stopTime = currentTime;                       // Update stop time each loop
                IntervalVolume += l_hour / 3600;              // l_hour is the flow rate per hour over one second, 3600 seconds in an hour
            } 
            else //  No flow so update the table only if we've previously recorded flow
            {
                if (startTime && (stopTime - startTime > 0)) // If flow detected and not on first loop
                { 
                    if (TableRowCounter >= TableRows) 
                    {
                        // Move all of the readings up one place to effectively scoll the table data so only last readings visible
                        for (int row=0; row < TableRows; row++) {
                            for (int col=0; col < TableCols; col++) {
                                TableData[row][col] = TableData[row+1][col];    // Move data up one row
                                if (SimbleeForMobile.updatable)
                                {
                                    SimbleeForMobile.updateValue(TableDataId[row][col], TableData[row][col]);
                                }                      
                            }
                        }
                        TableRowCounter = TableRows-1; // Forces all future data to be written to last row
                    }
                      
                    // Store values in data table
                    DataCounter++;
                    TableData[TableRowCounter][0] = DataCounter;
                    TableData[TableRowCounter][1] = startTime/1000;             // Timestamp
                    TableData[TableRowCounter][2] = (stopTime-startTime)/1000;  // Duration
                    TableData[TableRowCounter][3] = IntervalVolume;             // Total volume during this reading

                    TotalVolume += IntervalVolume;
                    
                    // Update readings on screen
                    if (SimbleeForMobile.updatable)
                    {                       
                        //SimbleeForMobile.updateColor(TableDataId[TableRowCounter][0], GRAY);            
                        SimbleeForMobile.updateValue(TableDataId[TableRowCounter][0], TableData[TableRowCounter][0]); // Index
                        SimbleeForMobile.updateValue(TableDataId[TableRowCounter][1], TableData[TableRowCounter][1]); // Timestamp
                        SimbleeForMobile.updateValue(TableDataId[TableRowCounter][2], TableData[TableRowCounter][2]); // Duration (s)
                        SimbleeForMobile.updateValue(TableDataId[TableRowCounter][3], TableData[TableRowCounter][3]); // VOlume (L)

                        SimbleeForMobile.updateValue(TotalVolumeId, TotalVolume);
                    }
                    
                    TableRowCounter++;
                    
                    startTime = 0;              // Reset start timer
                    IntervalVolume = 0;         // Reset interval volume
                    digitalWrite(led, LOW);
              } 
   
            }  
            flow_frequency = 0; // Reset Counter which is driven off the interrupt
    }

  // process must be called in the loop for SimbleeForMobile
  SimbleeForMobile.process();
}


void ui()
{

  // iPhone 8 Screen Units = 414 wide by 736 high Source: https://www.paintcodeapp.com/news/ultimate-guide-to-iphone-resolutions

  // color_t is a special type which contains red, green, blue, and alpha 
  // (transparency) information packed into a 32-bit value. The functions rgb()
  // and rgba() can be used to create a packed value.
  color_t darkgray = rgb(85,85,85);

  // These variable names are long...let's shorten them. They allow us to make
  // an interface that scales and scoots appropriately regardless of the screen
  // orientation or resolution.
  uint16_t wid = SimbleeForMobile.screenWidth;
  uint16_t hgt = SimbleeForMobile.screenHeight;


  uint16_t rowhgt = hgt/GridRows;   // Calculate the height of each row in the grid
  uint16_t colwid = wid/GridCols;   // Calculate the width of each column in the grid


  // Initialise the (x,y) points for each row and column in the master grid
  uint8_t row, col;

  for (row = 0; row < GridRows; row++) {
      GridRowX[row] = rowhgt*row;
  }

  for (col = 0; col < GridCols; col++) { 
      GridColY[col] = colwid*col;
  }
  

  // Initialise the (x,y) points for each row and column in the data table

  for (row = 0; row < TableRows; row++) {
      TableRowX[row] = rowhgt*row + GridRowX[4];  // Start table at row 4 in the master grid
  }


  int offset = 20; // to adjust titles so that data is more centred
  TableColY[0] = 5;                                    // Index
  TableColY[1] = GridColY[1]+offset;                  // Timestamp
  TableColY[2] = GridColY[3]+offset+10;               // Duration (s)
  TableColY[3] = GridColY[6]+offset;                  // Total Flow (L)


  // iPhone 8 Screen Units = 414 wide by 736 high Source: https://www.paintcodeapp.com/news/ultimate-guide-to-iphone-resolutions


  // The beginScreen() function both sets the background color and serves as a
  // notification that the host should try to cache the UI functions which come
  // between this call and the subsequent endScreen() call.
  SimbleeForMobile.beginScreen(BLACK);

  // SimbleeForMobile doesn't really have an kind of indicator- but there IS a
  // drawRect() function, and we can freely change the color of the rectangle
  // after drawing it! The x,y coordinates are of the upper left hand corner.
  // If you pass a second color parameter, you'll get a fade from top to bottom
  // and you'll need to update *both* colors to get the whole box to change.
  SimbleeForMobile.drawRect(GridColY[0],GridRowX[1],wid,40,darkgray); // uint8_t drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, color_t color);

  // Set title
  SimbleeForMobile.drawText(GridColY[1]+70, GridRowX[1]+10, "FLOW SENSOR", WHITE);

  // Set Flow Rate text fields
  SimbleeForMobile.drawText(GridColY[2], GridRowX[2], "Flow Rate (L/hr)  = ", WHITE);
  flowRateLabelId = SimbleeForMobile.drawText(GridColY[6], GridRowX[2], "", WHITE);

  // Setup table for logs
  SimbleeForMobile.drawRect(GridColY[0], GridRowX[3]-5,wid,30,darkgray); // uint8_t drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, color_t color);

  // Draw the table column headings
  SimbleeForMobile.drawText(TableColY[0], GridRowX[3], "#", WHITE);
  SimbleeForMobile.drawText(TableColY[1]-offset-10, GridRowX[3], "Timestamp", WHITE);
  SimbleeForMobile.drawText(TableColY[2]-offset-10, GridRowX[3], "Duration (s)", WHITE);
  SimbleeForMobile.drawText(TableColY[3]-offset-10, GridRowX[3], "Volume (L)", WHITE);
  
  // Set each cell of the table to blank and capture the id of the text label
  for (row = 0; row < TableRows; row++) {
    for (col = 0; col < TableCols; col++) {
      TableDataId[row][col] = SimbleeForMobile.drawText(TableColY[col], TableRowX[row], "", WHITE);
    }
  }

  SimbleeForMobile.drawRect(wid*0.75-15, GridRowX[GridRows-1]-5, wid*0.15+15, 30, GRAY);
  SimbleeForMobile.drawText(TableColY[1]+10, GridRowX[GridRows-1], "TOTAL VOLUME (L)", WHITE);
  TotalVolumeId = SimbleeForMobile.drawText(TableColY[3], GridRowX[GridRows-1], TotalVolume, WHITE);

  SimbleeForMobile.endScreen();


    if (SimbleeForMobile.updatable)
    {
        // Display any  values that may have been recorded while the app was not active
        for (int row = 0; row < TableRows; row++) {
          if (TableData[row][0] != 0) { // Only display the row if the index value is not zero
              for (int col = 0; col < TableCols; col++) {
                  SimbleeForMobile.updateValue(TableDataId[row][col], TableData[row][col]);
              }
          }
        }
        SimbleeForMobile.updateValue(TotalVolumeId, TotalVolume);
    } 
  
}

void ui_event(event_t &event)
{

}


void SimbleeForMobile_onDisconnect()
{
  // Do whatever needs to be done before we disconnect

  // don't leave the led on if they disconnect
  digitalWrite(led, LOW);
}




