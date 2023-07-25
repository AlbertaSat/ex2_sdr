
## Run GPredict to track satellites and provide Doppler correction

1. Open a terminal (shortcut is ctrl-alt-t)
2. Enter the command
   ```
   gpredict
   ```
3. GPredict will start and all the satellites of interest should be displayed.
   [[GPredict_main.png]]
	1. The GNU Radio flowgraph will connect to this instance of GPredict to get real-time updates for the corrected receive frequency. The GNU Radio flowgraph uses that value directly for direct receive downconversion to baseband, and calculates the transmit frequency for transmit upconversion from baseband.
4. 
5. Open a second terminal (shortcut ctrl-alt-t)
6. Enter the commands
   ```
   conda activate gnuradio
   gnuradio-companion
   ```
6. This will start the GNU Radio graphical application.
7. 