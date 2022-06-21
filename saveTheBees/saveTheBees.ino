// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK 0


/* Includes ---------------------------------------------------------------- */
#include <PDM.h>
#include <saveTheBees_inferencing.h>
#include <MKRWAN_v2.h>
#include "arduino_secrets.h"

LoRaModem modem;

String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;


/** Audio buffers, pointers and selectors */
typedef struct {
  int16_t *buffer;
  uint8_t buf_ready;
  uint32_t buf_count;
  uint32_t n_samples;
} inference_t;

static inference_t inference;
static signed short sampleBuffer[2048];
static bool debug_nn = false;  // Set this to true to see e.g. features generated from the raw signal
static volatile bool record_ready = false;

const int ON = LOW;  // Voltage level is inverted
const int OFF = HIGH;
const long DELAY = 500;
static bool first_round = true;

static bool is_connected = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("SaveTheBees V1");

  pinMode(LEDR, OUTPUT);  // Set red LED as output
  pinMode(LEDG, OUTPUT);  // Set green LED as output
  

  // Set up connection with The Things Network
  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  //summary of inferencing settings (from model_metadata.h)
  ei_printf("Inferencing settings:\n");
  ei_printf("\tInterval: ");
  ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
  ei_printf(" ms.\n");
  ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 8);
  ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

  if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false) {
    ei_printf("ERR: Failed to setup audio sampling\r\n");
    return;
  }
}

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */

void loop_connection() {
    Serial.println("Trying to connect...");
    int connected = modem.joinOTAA(appEui, appKey);
    Serial.println(connected);

    if (!connected) {
      digitalWrite(LEDR, OFF);
      delay(DELAY);
      digitalWrite(LEDR, ON);
      delay(DELAY);
      digitalWrite(LEDR, OFF);
      delay(DELAY);
      digitalWrite(LEDR, ON);
      Serial.println("Something went wrong; are you indoor? Move near a window. Retrying in 2 minute.");
      delay(120000);
    } else {

      digitalWrite(LEDR, OFF);

      Serial.println("Connected successfully");
      is_connected = true;
      digitalWrite(LEDG, ON);
      delay(DELAY);
      digitalWrite(LEDG, OFF);
      delay(DELAY);
      digitalWrite(LEDG, ON);
      delay(DELAY);
      digitalWrite(LEDG, OFF);
    }
}


void loop_classification() {


  // Short time intervals for demonstration reasons. LoRa transmission of results takes up to 5 minutes
    if (first_round) {
      ei_printf("Starting inferencing in 1 minute...");
      delay(60000);
      first_round = !first_round;
    } else {
      ei_printf("Starting inferencing in 5 minutes...\n");
      delay(300000);
    }

    digitalWrite(LEDG, ON);
    ei_printf("Recording...\n");

    bool m = microphone_inference_record();
    if (!m) {
      ei_printf("ERR: Failed to record audio...\n");
      return;
    }

    ei_printf("Recording done\n");

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", r);
      return;
    }

    // Get predictions

    String results = "";
    // Backup: Saving as float in 2 dimensional array to get all decimals.
    float res[2];

    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
              result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      ei_printf("    %s: ", result.classification[ix].label);
      results += result.classification[ix].value;
      results += " ";
      res[ix] = result.classification[ix].value;
      ei_printf_float(result.classification[ix].value);
      ei_printf("\n");
    }
    digitalWrite(LEDG, OFF);

  #if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: ");
    ei_printf_float(result.anomaly);
    ei_printf("\n");
  #endif


    int err;
    modem.beginPacket();
    modem.setPort(3);
    modem.print(results);
    err = modem.endPacket(true);

    Serial.println(err);

    // Find out why always error and no downlink message, even tough everything works fine.
    if (err < 0) {
      Serial.println("Results sent correctly");
    } else {
      Serial.println("Error sending results");
      is_connected = false;
    }
    Serial.println();
  }


void loop_saveRecording() {
  // Save data to SD Card
  // TODO
}


void loop() {
  if(!is_connected){
    loop_connection();
  } else {
     loop_classification();
  }
 
 
}

/**
 * @brief      PDM buffer full callback
 *             Copy audio data to app buffers
 */
static void pdm_data_ready_inference_callback(void) {
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

  if ((inference.buf_ready == 0) && (record_ready == true)) {
    for (int i = 0; i < bytesRead >> 1; i++) {
      inference.buffer[inference.buf_count++] = sampleBuffer[i];

      if (inference.buf_count >= inference.n_samples) {
        inference.buf_count = 0;
        inference.buf_ready = 1;
        break;
      }
    }
  }
}

/**
 * @brief      Init inferencing struct and setup/start PDM
 *
 * @param[in]  n_samples  The n samples
 *
 * @return     { description_of_the_return_value }
 */
static bool microphone_inference_start(uint32_t n_samples) {
  inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

  if (inference.buffer == NULL) {
    return false;
  }

  inference.buf_count = 0;
  inference.n_samples = n_samples;
  inference.buf_ready = 0;

  // configure the data receive callback
  PDM.onReceive(&pdm_data_ready_inference_callback);

  // optionally set the gain, defaults to 24
  // Note: values >=52 not supported
  //PDM.setGain(40);

  PDM.setBufferSize(2048);

  // initialize PDM with:
  // - one channel (mono mode)
  if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
    ei_printf("ERR: Failed to start PDM!");
    microphone_inference_end();
    return false;
  }

  return true;
}

/**
 * @brief      Wait on new data
 *
 * @return     True when finished
 */
static bool microphone_inference_record(void) {
  bool ret = true;


  record_ready = true;
  while (inference.buf_ready == 0) {
    delay(10);
  }

  inference.buf_ready = 0;
  record_ready = false;

  return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
  numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);

  return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void) {
  PDM.end();
  ei_free(inference.buffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif