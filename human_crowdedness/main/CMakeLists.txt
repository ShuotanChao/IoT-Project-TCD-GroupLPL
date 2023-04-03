idf_component_register(
    SRCS
        "main.cc"
        "camera_ctrl.cc"
        "model.cc"

    PRIV_REQUIRES console tflite-lib esp32-camera screen spi_flash fb_gfx
    INCLUDE_DIRS "")
