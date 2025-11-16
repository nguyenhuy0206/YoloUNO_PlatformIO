#include "tinyml.h"

// Globals, for the convenience of one-shot setup.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 16 * 1024; // Adjust size based on your model
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML()
{
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(exported_model_int8_tflite); // g_model_data is from model_data.h
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

int getSeasonFromMonth(int month)
{
    if (month >= 1 && month <= 3)  return 0;  // spring
    if (month >= 4 && month <= 6)  return 1;  // summer
    if (month >= 7 && month <= 9)  return 2;  // autumn
    return 3;                                  // winter
}

void tiny_ml_task(void *pvParameters)
{
    setupTinyML();

    while (1)
    {
        float ta = glob_temperature;
        float rh = glob_humidity;

        float region_id = 0.0f;   // Việt Nam = 0

        time_t now = time(nullptr);
        struct tm *t = localtime(&now);
        int month = t->tm_mon + 1;
        float season_id = getSeasonFromMonth(month);

        // ---- WRITE INTO INPUT TENSOR (4 floats) ----
        input->data.f[0] = ta;
        input->data.f[1] = rh;
        input->data.f[2] = region_id;
        input->data.f[3] = season_id;

        // ---- INFER ----
        if (interpreter->Invoke() != kTfLiteOk)
        {
            Serial.println("Invoke failed!");
            vTaskDelay(5000);
            continue;
        }

        // Softmax 3 classes: 0/1/2
        float p0 = output->data.f[0];
        float p1 = output->data.f[1];
        float p2 = output->data.f[2];

        int pred = 0;
        float maxp = p0;
        if (p1 > maxp) { maxp = p1; pred = 1; }
        if (p2 > maxp) { pred = 2; }

        Serial.print("Comfort Prediction = ");
        Serial.print(pred);
        Serial.print(" | probs = ");
        Serial.print(p0,4); Serial.print(" ");
        Serial.print(p1,4); Serial.print(" ");
        Serial.print(p2,4); Serial.println();

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
