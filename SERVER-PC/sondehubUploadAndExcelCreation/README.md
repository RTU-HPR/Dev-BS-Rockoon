This is a code that uploads received data to sondehub and creates an excel file with the received data.
At it's current stage it does the following:
    reads data that gets dumped in the serial monitor from a simultaneously running C++ code;
    If the data fits the format, writes it into excel and sends it to sondehub. If not, crash;
What to know before running it:
    Important - change the paths for where to create the excel file. if the path doesn't exist, IT WILL CRASH!;
    Check what's going on in the code as it runs. Any deviations from the set standards will probably cause it to stop/break, if that happens check what the receiver is receiving.
    This is where further on we can se the Rockoon parameters, and configure what data gets uploaded to sondehub, what data gets saved to excel, etcetera.
To-do:
    comments.
