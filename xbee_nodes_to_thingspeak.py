import serial
import thingspeak
import time

# ThingSpeak Information
channel_id = 3343217
write_key = "ZT5UD9F97582K4Q2"
channel = thingspeak.Channel(id=channel_id, api_key=write_key)

# Serial Setup
ser = serial.Serial('COM3', 9600, timeout=1) # Change the COM port if necessary

SENSITIVITY = 65535

# Store latest readings
analog_values = {
    "N1": 0,
    "N2": 0,
    "N3": 0
}

def find_alert_level(adc_reading):
    if adc_reading < SENSITIVITY * 0.25:
        return 0
    elif adc_reading < SENSITIVITY * 0.5:
        return 1
    elif adc_reading < SENSITIVITY * 0.75:
        return 2
    else:
        return 3
    
while True:
    try:
        line = ser.readline().decode().strip()
        
        if line:
            node_id, value = line.split(',')
            
            if node_id in analog_values:
                analog_values[node_id] = value
                
        # Compute alert levels
        level1 = find_alert_level(analog_values["N1"])
        level2 = find_alert_level(analog_values["N2"])
        level3 = find_alart_level(analog_values["N3"])
        
        # Determine system status
        system_status = 0
        active_node = 0
        
        if level1 >= 2:
            system_status = 1
            active_node = 1
        if level2 >= 2:
            system_status = 1
            active_node = 2
        if level3 >= 2:
            system_status = 1
            active_node = 3
        
        # Upload to ThingSpeak
        channel.update({
            'field1': analog_values["N1"],
            'field2': level1,
            'field3': analog_values["N2"],
            'field4': level2,
            'field5': analog_values["N3'],
            'field6': level3,
            'field7': active_node,
            'field8': system_status
        })
        
        time.sleep(15)
        
    except Exception as e:
        print("Error:", e)
        time.sleep(5)