# Derichs_2025

1. Mainboard:
    * Read signal from bluetooth
    * Read digital signal from On_Off board
    * From combo and send signal to another board

2. On_Off board: 
    * Prefix: O
    * Read signal from main board and control output port 
    * Check and compare current threshold of 4 ports and convert to on/off signal

3. BLDC board:
    * Read signal from mainboard and control BLDC motor
    * Chassis will have 2 mode is full control with M0-M8 code or control each wheel only with prefix W. W{L/R/S}{-255 -> 255}. L is left, R is right or S is speed for 2 wheel in full control mode.
    * Control BLDC with 2 mode:
        - Bi-direction mode: control by speed from -255 to 255 
        - Servo mode: control by get delta of each motor. 
        - Prefix: B
        - second character: 1/2 (motor 1 or motor 2)
        - code values:
            + E: enable servo mode
            + D: disable servo mode
            + R: reset servo mode
            + O: offset of servo mode (current position)
            + value from -255 to 255: speed or delta movation.
