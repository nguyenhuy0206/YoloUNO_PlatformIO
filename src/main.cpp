#include "global.h"
#include "dht_anomaly_model.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"


// Globals, for the convenience of one-shot setup.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
constexpr int kTensorArenaSize = 8 * 1024; // Adjust size based on your model
uint8_t tensor_arena[kTensorArenaSize];
} // namespace


void setup() {
  Serial.begin(115200);
  Serial.println("TensorFlow Lite Init....");
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  model = tflite::GetModel(dht_anomaly_model_tflite); // g_model_data is from model_data.h
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                           model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);


  Serial.println("TensorFlow Lite Micro initialized on ESP32.");
  xTaskCreate( led_blinky, "Task LED Blink" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( neo_blinky, "Task NEO Blink" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( temp_humi_monitor, "Task TEMP HUMI Monitor" ,2048  ,NULL  ,2 , NULL);
  
}

void loop() {
  
  // Prepare input data (e.g., sensor readings)
  // For a simple example, let's assume a single float input
  input->data.f[0] = 20.5; 
  input->data.f[1] = 80.5; 

  // Run inference
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke failed");
    return;
  }

  // Get and process output
  float result = output->data.f[0];
  Serial.print("Inference result: ");
  Serial.println(result);

  delay(5000); 

}