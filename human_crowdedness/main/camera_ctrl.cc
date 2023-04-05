/**
 * @file camera_ctrl.cc
 * @brief Implements the interactions with the ESP-EYE camera.
 * @date 2023-03-12
 * @version 2.0.1
 * 
 * @author Borja García Quiroga <garcaqub@tcd.ie>
 * 
 * © 2023 Group LPL, CS7NS2-202223
 * 
 * This code has been developed for the Internet of Things (CS7NS2) module
 * as partial requisits for the MSc in Computer Science at Trinity College,
 * The University of Dublin, Ireland during Hilary term 2023.
*/

#include "camera_ctrl.h"

#include <math.h>

#include "esp_camera.h"
#include "esp_system.h"
#include "sensor.h"

int get_image_from_cam(int8_t* img) {

  // Read a frame from the camera.
  camera_fb_t* fb = esp_camera_fb_get();

  // If nothing was read, an error occurred.
  if (!fb) {
    return -1;
  }

  // Convert the data from the buffer to the right integers.
  for (int i = 0; i < cam_width * cam_height; i++) {
    img[i] = ((uint8_t *) fb->buf)[i] ^ 0x80;
  }

  // Free the camera.
  esp_camera_fb_return(fb);

  // Return an OK code if all went well.
  return ESP_OK;

}

int init_camera() {

  // Build the config data structure.
  gpio_config_t conf;
  conf.mode = GPIO_MODE_INPUT;
  conf.pull_up_en = GPIO_PULLUP_ENABLE;
  conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  conf.intr_type = GPIO_INTR_DISABLE;
  conf.pin_bit_mask = 1LL << 13;
  gpio_config(&conf);
  conf.pin_bit_mask = 1LL << 14;
  gpio_config(&conf);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = CAMERA_PIN_D0;
  config.pin_d1 = CAMERA_PIN_D1;
  config.pin_d2 = CAMERA_PIN_D2;
  config.pin_d3 = CAMERA_PIN_D3;
  config.pin_d4 = CAMERA_PIN_D4;
  config.pin_d5 = CAMERA_PIN_D5;
  config.pin_d6 = CAMERA_PIN_D6;
  config.pin_d7 = CAMERA_PIN_D7;
  config.pin_xclk = CAMERA_PIN_XCLK;
  config.pin_pclk = CAMERA_PIN_PCLK;
  config.pin_vsync = CAMERA_PIN_VSYNC;
  config.pin_href = CAMERA_PIN_HREF;
  config.pin_sscb_sda = CAMERA_PIN_SIOD;
  config.pin_sscb_scl = CAMERA_PIN_SIOC;
  config.pin_pwdn = CAMERA_PIN_PWDN;
  config.pin_reset = CAMERA_PIN_RESET;
  config.xclk_freq_hz = XCLK_FREQ_HZ;
  config.pixel_format = CAMERA_PIXEL_FORMAT;
  config.frame_size = CAMERA_FRAME_SIZE;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  // Init the camera and detect if it failed.
  if (ESP_OK != esp_camera_init(&config)) {
    return -1;
  }

  // Get the camera sensor and flip the image so that it is
  // in the same shape as tensorflow.
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1); // Flip it.

  return ESP_OK;

}

void taf_add_new_frame(int8_t *img) {

  // Convert the data from the buffer to the right integers.
  for (int i = 0; i < cam_width * cam_height * cam_channels; i++) {

    // Get the value we will change.
    double dif = 0.0;

    // If we have enough, we will calculate what to remove.
    if (taf_used_frames >= taf_using_frames) {
      dif -= ((double) taf_background[i] / taf_using_frames);
    } else {
      // Increment the counter.
      taf_used_frames++;
    }

    // Sum the new value.
    dif += ((double) img[i] / taf_using_frames);

    // Apply those changes.
    taf_background[i] += dif;

  }

}

float get_ratio_of_different(int8_t *img, int thresh) {

  // Init the number of pixels.
  int num_of_px = 0;

  // Convert the data from the buffer to the right integers.
  for (int i = 0; i < cam_width * cam_height * cam_channels; i++) {

    // Get the value we will change.
    double dif = fabs(taf_background[i] - img[i]);

    // If the difference is greater than the threshold, mark it as change.
    if (dif > thresh)
      num_of_px++;

  }

  return (float) num_of_px / (cam_width * cam_height * cam_channels);

}
