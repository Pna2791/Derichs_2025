// Controller: ESP32 LOLIN32 LITE

// Uncomment exactly one:
// #define ROBOT_TYPE_MANUAL
// #define ROBOT_TYPE_AUTO

#if defined(ROBOT_TYPE_AUTO)
    #include "main_auto.h"
#else
    #include "main_manual.h"
#endif

