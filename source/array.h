#define ARRAY_SIZE(a) (sizeof(a))
#define ARRAY_ELEMENT_SIZE(a) (sizeof((a)[0]))
#define ARRAY_LENGTH(a) (ARRAY_SIZE(a)/ARRAY_ELEMENT_SIZE(a))
