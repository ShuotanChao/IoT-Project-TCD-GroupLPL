/**
 * @file main.cc
 * @brief Implements all the main functions and actions.
 * @date 2023-02-27
 * @version 3.0.0
 * 
 * @author Borja García Quiroga <garcaqub@tcd.ie>
 * 
 * © 2023 Group LPL, CS7NS2-202223
 * 
 * This code has been developed for the Internet of Things (CS7NS2) module
 * as partial requisits for the MSc in Computer Science at Trinity College,
 * The University of Dublin, Ireland during Hilary term 2023.
*/

#include <stdlib.h>

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "camera_ctrl.h"
#include "model.h"

// Set the model variables we will use from all functions.
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

// Set the memory allocation variables we need.
constexpr int alloc_size = 81 * 1024;
static uint8_t *alloc_space;

// Set the model constants.
constexpr int num_of_labels = 2;
constexpr int index_nothing = 0;
constexpr int index_people = 1;
const char* labels_array[num_of_labels] = {
    "nothing",
    "people",
};

/**
 * @brief Print a message before killing the execution.
 * 
 * Print a message before killing the execution.
 * 
 * @param error The message that will be printed before 
 */
void kill_with_error(const char * error) {

  std::cerr << "[KILL PROCESS] " << error << std::endl;
  exit(1);

}

/**
 * @brief Setup the system.
 * 
 * Sets up the whole system, including memory allocation, model loading and
 * camera initialization.
 * 
 * NOTE: Some tutorials say we must name this funtion setup to get Arduino
 * compatibility. I think we do not care because we are using ESP-IDF, but
 * there's no harm in doing it any way.
 */
void setup() {

  // Load the TensorFlow model into the C/C++ interface we can interact with.
  model = tflite::GetModel(g_person_detect_model_data);

  // Check that this model uses the same version we have installed in components.
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    std::cerr << "TF version " << model->version() << " != " << TFLITE_SCHEMA_VERSION << std::endl;
    kill_with_error("Wrong TF model version!");
  }
  
  // Allocate the memory that the processes will use.
  alloc_space = (uint8_t *) heap_caps_malloc(alloc_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

  // If the returned value is NULL, it means that the memory allocation failed.
  if (alloc_space == NULL) {
    kill_with_error("Memory allocation failed!\n");
  }

  // Allocate space for the fake background.
  taf_background = (double *) heap_caps_malloc(cam_width * cam_height * cam_size * sizeof(double), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

  // If the returned value is NULL, it means that the memory allocation failed.
  if (taf_background == NULL) {
    kill_with_error("2nd memory allocation failed!\n");
  }

  // Init it.
  for (int i = 0; i < cam_width * cam_height * cam_channels; i++) {
    taf_background[i] = 0.0;
  }

  // Build the resolver that we will use to build the interpreter.
  static tflite::MicroMutableOpResolver<5> resolver;
  resolver.AddAveragePool2D();
  resolver.AddConv2D();
  resolver.AddDepthwiseConv2D();
  resolver.AddReshape();
  resolver.AddSoftmax();

  // Now, we build the interpreter.
  // We have to do it in two lines so that memory allocation does not crash.
  static tflite::MicroInterpreter local_interpreter(model, resolver, alloc_space, alloc_size);
  interpreter = &local_interpreter;

  // Then, we allocate the actual tensors.
  if (kTfLiteOk != interpreter->AllocateTensors()) {
    kill_with_error("Tensor allocation failed!");
  }

  // Then, store the info about where the input must go into a global variable
  // to be used elsewhere.
  input = interpreter->input(0);

  // Init the camera.
  if (ESP_OK != init_camera()) {
    kill_with_error("Camera init failed!");
  }

}

/**
 * @brief Main loop of the application.
 * 
 * This function is the actual main loop of the application that repeats until
 * infinity.
 * 
 * NOTE: Some tutorials say we must name this funtion setup to get Arduino
 * compatibility. I think we do not care because we are using ESP-IDF, but
 * there's no harm in doing it any way.
 */
void loop() {

  // Get image from provider.
  if (ESP_OK != get_image_from_cam(input->data.int8)) {
    kill_with_error("Camera failed when reading!");
  }

  // Add a new frame if there are not enough.
  taf_add_new_frame(input->data.int8);

  // If there are not enough frames yet, pass.
  if (taf_used_frames < taf_using_frames) {
    vTaskDelay(10);
    return;
  }

  // Predict the label.
  if (kTfLiteOk != interpreter->Invoke()) {
    kill_with_error("Prediction failed!");
  }

  // Obtain the predictions from the model.
  TfLiteTensor* output = interpreter->output(0);

  // Process the inference results.
  int8_t score_nothing = output->data.uint8[index_nothing];
  int8_t score_people = output->data.uint8[index_people];

  float score_nothing_float = (score_nothing - output->params.zero_point) * output->params.scale;
  float score_people_float = (score_people - output->params.zero_point) * output->params.scale;

  // Print how likely it is to get people in this frame.
  std::cout << "People score: " << score_people_float * 100 << std::endl;

  // Get percentage of differences.
  float ratio = get_ratio_of_different(input->data.int8, 50);

  // Here is where the response is.
  int result = 0;

  // Pass from probability to numbers.
  if (score_people_float >= 0.65 && ratio > 0.40)
    result = 2;
  else if (score_people_float >= 0.75 || ratio > 0.30)
    result = 1;
  else
    result = 0;

  /**
   * @todo HERE IS WHERE THE WIFI / BLUETOOTH CONNECTION SHOULD BE IMPLEMENTED.
   * 
   * result = 2: Means that the room is too full.
   * result = 1: Means that there are some people in the room.
   * result = 0: Means that the room is empty.
   */

  // Delay the next task so that we can get different results.
  vTaskDelay(100);

}

void main_task(void) {

  // Set up the components.
  setup();

  // Main loop.
  while (true) {

    loop();
    
  }

}

/**
 * @brief Main function.
 * 
 * Main function that is called when the chip launches.
 */
extern "C" void app_main() {

  xTaskCreate((TaskFunction_t)&main_task, "main_task", 4 * 1024, NULL, 8, NULL);
  
  vTaskDelete(NULL);

}
