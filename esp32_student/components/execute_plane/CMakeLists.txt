set(DSP_MFCC_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../shadow_device/esp32-dsp/dsp_mfcc/)

idf_component_register(
    SRCS
        "${DSP_MFCC_ROOT}/dsp_mfcc_mel.c"
        "${DSP_MFCC_ROOT}/dsp_mfcc_feature.c"
        "execute_plane.c"
        "test_mfcc_feature.c"
    INCLUDE_DIRS
        "${DSP_MFCC_ROOT}"
         "."
    PRIV_REQUIRES
        support_plane
        observe_plane
)
