cmake_minimum_required(VERSION 3.10)
project(sbc C)

set(CMAKE_C_STANDARD 99)

set(ESP32_DSP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../esp32_student/managed_components/espressif__esp-dsp/modules/)

# ================================ library ================================
add_library(esp_dsp_fft STATIC 
    ${ESP32_DSP_ROOT}/fft/float/dsps_fft2r_fc32_ansi.c
    ${ESP32_DSP_ROOT}/fft/float/dsps_fft2r_bitrev_tables_fc32.c
    ${CMAKE_CURRENT_LIST_DIR}/esp_dsp/dsps_pwroftwo.c
)

target_include_directories(esp_dsp_fft PUBLIC 
    ${ESP32_DSP_ROOT}/fft/include
    ${ESP32_DSP_ROOT}/common/include
    ${ESP32_DSP_ROOT}/common/include_sim
    /home/qing/esp/esp-idf/components/esp_common/include/
)

# ================================ test ================================
add_executable(test_dsp_fft tests/test_fft.c)
target_include_directories(test_dsp_fft PUBLIC 
    ${ESP32_DSP_ROOT}/fft/include
    ${ESP32_DSP_ROOT}/common/include
    ${ESP32_DSP_ROOT}/common/include_sim
    ${CMAKE_CURRENT_LIST_DIR}/tests
    /home/qing/esp/esp-idf/components/esp_common/include/
)
target_link_libraries(test_dsp_fft esp_dsp_fft -lm)

add_executable(test_dsp_mfcc dsp_mfcc/dsp_mfcc_feature.c tests/test_dsp_mfcc_feature.c)
target_include_directories(test_dsp_mfcc PUBLIC 
    ${ESP32_DSP_ROOT}/fft/include
    ${ESP32_DSP_ROOT}/common/include
    ${ESP32_DSP_ROOT}/common/include_sim
    ${CMAKE_CURRENT_LIST_DIR}/dsp_mfcc
    /home/qing/esp/esp-idf/components/esp_common/include/
)
target_link_libraries(test_dsp_mfcc esp_dsp_fft -lm)
