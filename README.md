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
=======
# LoRaWAN computer vision with Edge Impulse & Portenta H7

This is an example application that runs a computer vision model on the Portenta H7 and streams the results over LoRaWAN. The application uses the camera on the Portenta Vision Shield in combination with a machine learning model trained in Edge Impulse to determine when an interesting event happens, then sends this using the LoRa radio on the Portenta Vision Shield back to the network. This demo was built for The Things Conference 2021.

![Elephant](img/elephant.jpg) ![Not elephant](img/not-elephant.jpg)

**Elephant vs. not elephant**

> **Note:** This example was built using a pre-release version of the Portenta H7 libraries, and a preview version of Edge Impulse for the Portenta H7. There are expected issues with exposure, and bugs may arise.

## Requirements

You'll need the following hardware:

* [Portenta H7](https://store.arduino.cc/usa/portenta-h7) development board.
* [Portenta vision shield](https://www.arduino.cc/pro/hardware/product/portenta-vision-shield) - LoRa variant.

## How to build

1. Set your application EUI and application key in [src/ei_main.cpp](src/ei_main.cpp).
1. If you want to use a different channel plan (default: EU868) set it in [src/ei_main.cpp](src/ei_main.cpp) as well.
1. Install the Arduino CLI.
1. Build this application via:

    ```
    $ sh arduino-build.sh --build
    ```

1. Flash this application via:

    ```
    $ sh arduino-build.sh --flash
    ```

## Training a new model

The elephant model used in the demo is here: [Elephant tracker](https://studio.edgeimpulse.com/public/16116/latest/).

1. Load the Edge Impulse firmware for the Portenta H7: [instructions](https://docs.edgeimpulse.com/docs/arduino-portenta-h7).
1. Build a new model from scratch in Edge Impulse with the following settings:

    * Image width / height: 64x64 pixels.
    * Image color depth: Grayscale.
    * Transfer learning base model: MobileNetV2 0.1.

2. To avoid sending messages when just the classification for a single frame changes the output of the algorithm is smoothened. These parameters can be found in [ei_run_impulse.cpp](src/ingestion-sdk-c/ei_run_impulse.cpp) (search for the `ei_classifier_smooth_init` function).
3. Then, remove the `src/edge-impulse-sdk`, `src/model-paramters` and `src/tflite-model` folders.
4. In your Edge Impulse project go to the **Deployment** page and export as **C++ Library**.
5. Add the files in the export to the `src` directory and recompile the application.

## Debugging camera output

Issues with the camera? Install the [Edge Impulse CLI](https://docs.edgeimpulse.com/docs/cli-installation), and run `edge-impulse-daemon`. This will connect the development board to Edge Impulse from where you can see a live feed of the camera.

Alternatively, connect a serial monitor to the development board, press `b` to stop inferencing, then run `AT+RUNIMPULSEDEBUG`. This will print out the framebuffer after capturing and resizing the image. Write the framebuffer to `framebuffer.txt`, and then run:

```
$ edge-impulse-framebuffer2jpg -f framebuffer.txt -w 64 -h 64 -o framebuffer.jpg
```
