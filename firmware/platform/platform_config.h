#define PLATFORM_DUMMY  (0)
#define PLATFORM_STM32F103X (1)

#define PLATFORM    PLATFORM_STM32F103X

#if (PLATFORM == PLATFORM_STM32F103X)
    #define STM32F103xB
#endif
