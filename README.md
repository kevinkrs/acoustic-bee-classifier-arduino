# Documentation

# Hardware List

| Arduino Portenta H7 |  |
| --- | --- |
| Arduino Portenta Vision Shield LoRa |  |
| Pentaband antenna |  |
| USB-C to USB-A cable |  |
| Solar panel |  |
| Solar power manager |  |
| 3.7V single cell Li-Po battery > 700mAh |  |

# Tech-Sheet

| Connection |  |
| --- | --- |
| Frequency | - 125kHz |
| Transmission | 0.3 - 5 kbps |
| Microphone |  |
| On Board |  |
| External  |  |

# Setup

## LoraWan

1. Get Arduino IDE
2. Connect Portenta H7 with Vision Shield to computer
3. Download `MKRWAN` library
4. Go to 
    
    `File > Examples > MKRWAN > MKRWANupdate_standalone`
    
5. Upload the code
6. Open serial monitor to check when ready 
    
    `Tools > Serial Monitor`
    

## Arduino Cli

<aside>
💡 Shell client to communicate with connected Arduino board

</aside>

## Edge Impulse

<aside>
💡 Platform for training machine learning algorithms and creating deployments specialised for small, low energy devices (Arduino, raspberry’s, mobile etc.)

</aside>

# Deploying scripts

1. Download the script files `.sh`
2. Go to file directory and run
    
    `sh flash.mac_command`
    
3. Run `edge-impulse-daemo` to start arduino

# Possibilites

- Detection if bee-hive has queen
- Detection of stress level
- Detection of varroa infection

<aside>
💡 Creating ML models based on audio data can result in an non-intrusive way of bee-hive monitoring.

</aside>

- Little effort to set-up
- Comparably cheap (~200€ for hardware)
