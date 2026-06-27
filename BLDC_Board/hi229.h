#ifndef HI229_H
#define HI229_H


int get_direction(HardwareSerial &serialPort = Serial) {
    static byte previousByte = 0;
    static int count = 0;

    if (serialPort.available()) {
        byte byteInt = serialPort.read();

        if (byteInt == 0xA5 && previousByte == 0x5A) {
            count = 2;  // Detected start sequence
        } else {
            count++;
            if (count == 13) {
                int angle = byteInt * 256 + previousByte;  // Combine last two bytes
                if (angle > 0x7FFF) {
                    angle = 0xFFFF - angle;
                    angle = -angle;
                }
                count = 0;  // Reset to look for the next packet
                previousByte = byteInt;
                return angle;
            }
        }
        previousByte = byteInt;
    }
    return 0xFFF;
}

void reset_direction(HardwareSerial &serialPort = Serial){
    serialPort.println("AT+RST");
    delay(1000);
    serialPort.println("AT+RST");
}

#endif