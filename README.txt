		Update List for Vent5 Code 
	

List of changes: 
(add new lines to top of list whenever changes are made
--------------------Format--------------------
Date of Update: 7/27/22
Editor: Seyon 
Changes to code: Cleaned up code, ready to share
---> 
Emulated flight tested (y/n): not needed
Notes/Observations:
---> 
Future suggestions or plans for changes: 
---> 
----------------------------------------------
Date of Update: 6/27/22
Editor: Seyon, ETJ uploading
Changes to code: changed code for GL197 added radio functionality
---> 
Emulated flight tested (y/n): ?
Notes/Observations:
---> 
Future suggestions or plans for changes: 
---> 
----------------------------------------------
Date of Update: 6/9/22 push #1
Editor: Ethan Thomson-Jewell
Changes to code: 
---> added avgAscentRateAfterVenting function to record average ascent rates at 30s intervals after each round of venting is finished
---> finished the first prevent funciton for GL196
---> made functions that calculate the estimated time needed for the 
Emulated flight tested (y/n): N
Notes/Observations:
---> Not yet tested the aveAscentRateAfterVenting functions
Future suggestions or plans for changes: 
---> going to change the prevent and vent functions to the specifications of GL196
----------------------------------------------
Date of Update: 6/8/22 push #2
Editor: Ethan Thomson-Jewell
Changes to code: 
Tested previously pushed code with the emulator, prevent functions work, calling of termination function works
Emulated flight tested (y/n): YES
Notes/Observations:
---> When testing with the emulator, whenever servo was not moving or commanded to move, the ascent rate was calculated correctally.
This means when the vent was open during a preventing cycle the average ascent rate could be used possibly, exept for the calculation right before or 
right after the servo moves. The plan for the next flight is to use the calculated ascent rate 30s, 60s, 90s, and 120s after venting is over to make decition of how
long to vent the next time.
Future suggestions or plans for changes: 
---> going to change the prevent and vent functions to the specifications of GL196
----------------------------------------------
Date of Update: 6/8/22
Editor: Ethan Thomson-Jewell
Changes to code: 
---> Moved jiggle if statements down into a jiggle function:   ### The jiggle function is currently commented out and is not in use ebcause it doesnt work yet ###
Funciton is to jiggle the servo if the servo is not within a specified range (Currently +/- 40 degrees) ie it got stuck for some reason
Keep in mind: The range of +/- 40 is because there is not currently a fix for the servoPosition discrepency issue. possibly this is caused by calibration issue 
of output voltage when getting the servoPos values. This has not been resolved
---> Added a Max and Min servo position variable that logs the max and min degrees that the servo can reach. these are found out using a testing loop in the beginning 
of system update. this loop only triggers once. This was origionally going to be in systemSetup but I have not gotten that to work yet, not sure why.
these variables are also logged to the SD card now
--->Added the 'SERVOPOS' serial command that returns current servo position in degrees
---> Moved the checking of flapperstate if statement to the beginnign of the 2s loop. before it required a GPS lock to correct an out of place flapper, now it wont
Emulated flight tested (y/n): 
Notes/Observations:
----------------------------------------------------
Date of Update: 6/7/22
Editor: Ethan Thompson-Jewell
Major changes to code: 
---> moved ascent rate logging to SD card (either pressure or GPS) outside of the GPSlock == true if else statement. Lines 149 System update is where new is
---> Added GPS time stamp [lines 48-50]
---> flight time to be logged to SD card. no using flightTime for any logic yet. im scared to mess something up. might do it for GL197 [line 53]
---> 10-cycle ave ascent rate now logged to SD every cycle [line 128 Sysyem update]
---> Confirmed suggest state and command state are logged
---> Servo commanded position is now logged to SD card every cycle and updated each time closeVent() and openVent() are called
---> Flight timer now starts after pressure has increased 4 times 
Emulated flight tested (y/n):
Notes/Observations: 
Future suggestions or plans for changes: 

## make timers in code are based off of flight timer not currTimeS
	prevent 1 & 2
	big vent timer
	terminate timer

##closeServo issue <<--------- wednesday with seyon
	*Delay because of jiggle function is back bc of this so i am setting clsoe servo to 132
## descent logic. make sure that vent knows when it in decent 
----------------------------------------------------
Date of Update: 6/6/22
Editor: Ethan Thompson-Jewell
Major changes to code: 
-------> changed suggest state to fix the problem that every 4 cycles the gps alt would not update and thus teh ascent rate would be set equal to zero
this change was in state_machine ln 35ish
-------> changed line 212 if statement to check if servoPos is +/- 40 from 132 degrees rather than 180 because that is how much the servo can close to not 180. this is something that 
may need to be changed with each avionics package. unless we impliment a function that puches servo to fully closed then sets a new closed servo position
-------> Changed around ave_ascent_rate calculation in system update. just moved a repeated if statement around in a loop. ~line 65-80
-------> Edited jiggle function and changes 'closeServo' variable so Vent doesnt try to jiggle flapper every cycle.
-------> If not said in last update of code, added a millis section to what is logged by SD card
Emulated flight tested (y/n): Yes
Notes/Observations: 
---> Code was able to prevent twice at 75K and 85K feet. 
---> During both prevents the ascent Rate was reduced to target amount (10% of pre-vent rate)
---> After prevent2 and termination were both started because of timers which were still quite close to desired altitude for both protocols
Future suggestions or plans for changes: 
--Should modify emulation to test vent's reaction to following scenarios:
*Ascent rate not reduced by desired amount during 1 round of venting (Ex prevent 1 doesnt reduce ascent rate by 10%)
*Different slowAscent/float scenarios (venting was too effective) 
*termination based on altitude rather than time since venting (time since prevent logic worked when tested)
