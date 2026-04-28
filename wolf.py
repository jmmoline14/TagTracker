import serial
import requests
# open the serial connection on COM17, this is the port that I'm using to connect the 
# Xbee directly, if you use a different port make sure to change this obviously
ser = serial.Serial('COM17', 9600, timeout=1)
# Make a quick dictionary for accessing which wolf is which
THINGSPEAK_API_KEY = "MY_API_KEY"
sheepnum = {
    "0013A200420108FE" : "Sheep1",
    "0013A20042010909" : "Sheep2",
    "0013A20042010911" : "Sheep3"
}
# Helper function for going through the frame
def read_frame():
    # Wait for start delimiter
    while True:
        byte = ser.read(1)

        if byte == b'\x7E':
            break
    
    # Read length
    length = int.from_bytes(ser.read(2), 'big')
    
    # Read frame data + checksum
    frame = ser.read(length + 1)
    
    return frame
def upload_to_thingspeak(sheep_name, payload):
    url = "https://api.thingspeak.com/update"
    params = {
        "api_key": THINGSPEAK_API_KEY,
        "field1": sheep_name,
        "field2": payload
    }
    response = requests.get(url, params=params)
    print(f"ThingSpeak response: {response.text}")  # returns entry_id if successful, 0 if failed
while True:
    frame = read_frame()
    
    # Check for ZB Receive Packet (0x90)
    if frame[0] == 0x90:
        # Extract source address (bytes 1-8)
        src_addr = frame[1:9].hex().upper()
        
        # Extract payload (starts at byte 12)
        payload = frame[12:-1].decode('utf-8', errors='ignore')
        
        #print(f"From: {src_addr}")
        #print(f"Message: {payload}")
        sheep = sheepnum.get(src_addr, "Unknown Sheep")
        #print(sheep)
        upload_to_thingspeak(sheep, payload)
        