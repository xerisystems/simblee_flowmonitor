# simblee_flowmonitor
  SIMPLE SIMBLEE FLOW MONITOR
  
  This sketch reads from a FS400A Water Flow Sensor and displays the outputs via the Simblee mobile app.
  
  Instantaneous flow (L/hr) is shown along with previous recordings of total volume (L). The Simblee display code
  is optimised for an iPhone 8. The Simblee App works across platforms but I have not tested on other screen
  sizes or operating systems.
  
  What was the origin of this project?
  The original purpose of this project was to try and find out why a water tank was being emptied. The
  pump seemed to go on at random times.  I wanted to determine if there was a pattern and how much water 
  was being lost each time it came on.
  
  What could be improved?
  This is my first Simblee project. The main aim of this iteration was just to get it working. As such, the code
  is pretty messy and could definately be improved!
  I'm also not totally convinced that the calirbation of the flow sensor is right. This probably could be adjusted
  in code to avoid the tril/error/recompile process.
  
  Author: XeriSytems 2018
  
  References:
   Sparkfun Simblee Breakout can be purchased here: https://www.sparkfun.com/products/13632
   Flow Sensor can be purchased here: https://www.ebay.com.au/itm/FS400A-G1-1-2Mpa-Water-Flow-Sensor-Hall-Effect-Flowmeter-Counter-1-60L-min-/152172303810
   Basic flow sensor code can be found here: http://www.hobbytronics.co.uk/download/YF-S201.ino
   Some info on how to calibrate a flow sensor: 
       https://github.com/sekdiy/FlowMeter/wiki/Calibration
       http://wiki.seeedstudio.com/G3-4_Water_Flow_sensor/
       https://www.electroschematics.com/12145/working-with-water-flow-sensors-arduino/
       https://maker.pro/arduino/tutorial/how-to-interface-arduino-with-flow-rate-sensor-to-measure-liquid

