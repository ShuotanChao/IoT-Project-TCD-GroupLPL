/**
 * @file camera_ctrl.h
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

#ifndef CAMERA_CTRL_H_
#define CAMERA_CTRL_H_

#include "esp_camera.h"
#include "esp_system.h"
#include "sensor.h"

/**
 * COLOR:
 * 
 * PIXFORMAT_RGB565,    // 2BPP/RGB565
 * PIXFORMAT_YUV422,    // 2BPP/YUV422
 * PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
 * PIXFORMAT_JPEG,      // JPEG/COMPRESSED
 * PIXFORMAT_RGB888,    // 3BPP/RGB888
 * 
 * SIZE:
 * 
 * FRAMESIZE_96X96,    // 96x96
 * FRAMESIZE_QQVGA,    // 160x120
 * FRAMESIZE_QQVGA2,   // 128x160
 * FRAMESIZE_QCIF,     // 176x144
 * FRAMESIZE_HQVGA,    // 240x176
 * FRAMESIZE_QVGA,     // 320x240
 * FRAMESIZE_CIF,      // 400x296
 * FRAMESIZE_VGA,      // 640x480
 * FRAMESIZE_SVGA,     // 800x600
 * FRAMESIZE_XGA,      // 1024x768
 * FRAMESIZE_SXGA,     // 1280x1024
 * FRAMESIZE_UXGA,     // 1600x1200
 */

#define CAMERA_PIXEL_FORMAT PIXFORMAT_GRAYSCALE
#define CAMERA_FRAME_SIZE FRAMESIZE_96X96

#define CAMERA_MODULE_NAME "ESP-EYE"
#define CAMERA_PIN_PWDN -1
#define CAMERA_PIN_RESET -1
#define CAMERA_PIN_XCLK 4
#define CAMERA_PIN_SIOD 18
#define CAMERA_PIN_SIOC 23

#define CAMERA_PIN_D7 36
#define CAMERA_PIN_D6 37
#define CAMERA_PIN_D5 38
#define CAMERA_PIN_D4 39
#define CAMERA_PIN_D3 35
#define CAMERA_PIN_D2 14
#define CAMERA_PIN_D1 13
#define CAMERA_PIN_D0 34
#define CAMERA_PIN_VSYNC 5
#define CAMERA_PIN_HREF 27
#define CAMERA_PIN_PCLK 25

#define XCLK_FREQ_HZ 15000000

// Set the camera constants.
constexpr int cam_width = 96;
constexpr int cam_height = 96;
constexpr int cam_channels = 1;
constexpr int cam_size = cam_width * cam_height * cam_channels;

// TAF variables.
constexpr int taf_using_frames = 100;
static int taf_used_frames = 0;
static double *taf_background;

/**
 * @brief Get ain image from the camera.
 * 
 * Get an image from the ESP-EYE camera.
 * 
 * @param img Where the image will be stored.
 * 
 * @returns A status. If is ESP_OK, it's that everything worked.
 */
int get_image_from_cam(int8_t* img);

/**
 * @brief Init the camera.
 * 
 * Init the ESP-EYE camera.
 * 
 * @returns A status. If is ESP_OK, it's that everything worked.
 */
int init_camera();

/**
 * @brief Init the TAF backfround with a new frame.
 * 
 * Init the TAF backfround with a new frame.
 * 
 * @param new_frame The new frame to be added.
 */
void taf_add_new_frame(int8_t *img);

/**
 * @brief Get the ratio of pixels whose difference from TAF is over thresh.
 * 
 * Get the ratio of pixels of img whose difference from the TAF background is
 * over the threshold thresh.
 * 
 * @param img The image to compare.
 * @param thresh The threshold.
 * 
 * @return The ratio of different pixels.
 */
float get_ratio_of_different(int8_t *img, int thresh);

#endif  // CAMERA_CTRL_H_
