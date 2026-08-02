import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button, TextBox
import time

FORWARD = False

# Serial port configuration
SERIAL_PORT = '/dev/cu.Maze2'  # Your Bluetooth port
SERIAL_PORT = '/dev/cu.usbserial-110'  # Your Bluetooth port
SERIAL_PORT = '/dev/cu.BLDC_1'  # Your Bluetooth port

# Initialize serial connection
try:
    ser = serial.Serial(SERIAL_PORT, timeout=1, baudrate=115200)  # No baud rate needed for Bluetooth on macOS
    ser.flushInput()  # Clear any stale data in the input buffer
    ser.flushOutput()  # Clear any stale data in the output buffer
    print(f"Successfully opened port {SERIAL_PORT}")
except serial.SerialException as e:
    print(f"Could not open port {SERIAL_PORT}: {e}")
    print(f"Could not open port {SERIAL_PORT}: {e}")
    exit()

# Parameters
MAX_POINTS = 500  # Number of points to display
speed_data = np.zeros(MAX_POINTS)  # Array to store speed values
accel_data = np.zeros(MAX_POINTS)  # Array to store acceleration values

# Set up the plot with dual Y-axes
fig, ax1 = plt.subplots(figsize=(10, 6))
ax2 = ax1.twinx()  # Create a second Y-axis sharing the same X-axis

# Plot speed on the left Y-axis (ax1)
line1, = ax1.plot(speed_data, 'b-', label='Speed')
ax1.set_ylim(-1.5, 1.5)  # Initial Y-axis limits for speed
ax1.set_xlim(0, MAX_POINTS-1)
ax1.set_xlabel('Sample')
ax1.set_ylabel('Speed', color='b')
ax1.tick_params(axis='y', labelcolor='b')
ax1.grid(True)

# Plot acceleration on the right Y-axis (ax2)
line2, = ax2.plot(accel_data, 'r-', label='Direction')
ax2.set_ylim(-1.5, 1.5)  # Initial Y-axis limits for acceleration
ax2.set_ylabel('Direction', color='r')
ax2.tick_params(axis='y', labelcolor='r')

# Title and legend
plt.title('Real-time Speed and Acceleration from ESP32')
fig.legend(loc='upper right', bbox_to_anchor=(0.95, 0.95))

# Adjust the layout to make space for buttons and textbox at the bottom
plt.subplots_adjust(bottom=0.2)  # Add more space at the bottom for buttons and textbox

# Define positions and create buttons
ax_button1 = plt.axes([0.05, 0.1, 0.1, 0.05])  # [left, bottom, width, height]
ax_button2 = plt.axes([0.2, 0.1, 0.1, 0.05])
ax_button3 = plt.axes([0.35, 0.1, 0.1, 0.05])
button1 = Button(ax_button1, 'Send M1' if FORWARD else 'C29')
button2 = Button(ax_button2, 'Send M0' if FORWARD else 'C21')
button3 = Button(ax_button3, 'Send M3' if FORWARD else 'C22')

# Define position and create textbox
ax_textbox = plt.axes([0.7, 0.1, 0.2, 0.05])  # [left, bottom, width, height]
textbox = TextBox(ax_textbox, 'Command: ', initial='')


# Button callback functions
def send_m1(event):
    try:
        # code = "M1\n" if FORWARD else "W90\n"
        code = "M1\n" if FORWARD else "C29\n"
        ser.write(code.encode('utf-8'))  # Send "M1" with a newline
        ser.flush()  # Ensure the data is sent immediately
        print("Sent: M1")
    except serial.SerialException as e:
        print(f"Error sending M1: {e}")
def send_m3(event):
    try:
        code = "M3\n" if FORWARD else "C22\n"
        ser.write(code.encode('utf-8'))  # Send "M1" with a newline
        ser.flush()  # Ensure the data is sent immediately
        print("Sent: M3")
    except serial.SerialException as e:
        print(f"Error sending M1: {e}")

def send_m0(event):
    try:
        code = "M0\n" if FORWARD else "C21\n"
        ser.write(code.encode('utf-8'))  # Send "M0" with a newline
        ser.flush()  # Ensure the data is sent immediately
        print("Sent: M0")
    except serial.SerialException as e:
        print(f"Error sending M0: {e}")

# Textbox callback function
def submit_command(text):
    if text.strip():  # Only send if the text is not empty
        try:
            command = f"{text}\n"  # Add a newline to the command
            ser.write(command.encode('utf-8'))  # Send the custom command
            ser.flush()  # Ensure the data is sent immediately
            print(f"Sent custom command: {text}")
        except serial.SerialException as e:
            print(f"Error sending custom command: {e}")
    else:
        print("No command entered (empty text)")

# Connect the buttons and textbox to their callback functions
button1.on_clicked(send_m1)
button2.on_clicked(send_m0)
button3.on_clicked(send_m3)
textbox.on_submit(submit_command)

# Function to read and parse data from the Bluetooth port
def read_data():
    try:
        # Check if there is data waiting to be read
        if ser.in_waiting > 0:
            # Step 1: Get raw value from serial
            raw_line = ser.readline().decode('utf-8').strip()
            if not raw_line:
                print("No data received (empty line)")
                return None
            
            print(f"Raw data: {raw_line}")  # Debug: Print raw data
            
            # Step 2: Extract values by splitting on space
            parts = raw_line.split()
            if len(parts) < 2:  # Expecting "Received:", value1, value2
                print("Error: Not enough values in line")
                return None
            
            # Step 3: Convert the last two parts to floats (speed and acceleration)
            try:
                speed = float(parts[-1])  # Second-to-last part is speed
                accel = float(parts[-2])  # Last part is acceleration
                # speed = float(parts[-2])  # Second-to-last part is speed
                # accel = float(parts[-1])  # Last part is acceleration
            except ValueError as e:
                print(f"Error converting to float: {e}")
                return None
            
            # Step 4: Print the extracted values to the screen
            print(f"Speed: {speed}, Acceleration: {accel}")
            
            return speed, accel
        else:
            print("No data waiting in buffer")
            return None
    except (ValueError, UnicodeDecodeError) as e:
        print(f"Error reading data: {e}")
        return None

# Update function for animation
def update(frame):
    global speed_data, accel_data
    
    # Read new values
    result = read_data()
    if result is not None:
        new_speed, new_accel = result
        
        # Shift data left and add new values on the right
        speed_data[:-1] = speed_data[1:]
        speed_data[-1] = new_speed
        
        accel_data[:-1] = accel_data[1:]
        accel_data[-1] = new_accel
        
        # Update the plots
        line1.set_ydata(speed_data)
        line2.set_ydata(accel_data)
        
        # Adjust Y-axis limits dynamically for speed (left axis)
        speed_max = np.max(np.abs(speed_data))
        if speed_max > ax1.get_ylim()[1] * 0.8:

            MXXX = 1.2 if FORWARD else 260
            ax1.set_ylim(-MXXX, MXXX)
        # ax1.set_ylim(-0.7, speed_max * 0.7)
        
        # Adjust Y-axis limits dynamically for acceleration (right axis)
        accel_max = np.max(np.abs(accel_data))
        if accel_max > ax2.get_ylim()[1] * 0.8:
            MXXX = 30 if FORWARD else 120
            ax2.set_ylim(-MXXX, MXXX)
    
    return line1, line2

# Animation
ani = FuncAnimation(fig, update, frames=None, interval=5, blit=True, cache_frame_data=False)  # 100ms update interval

# Show the plot
plt.show()

# Clean up
ser.close()
print("Serial port closed.")