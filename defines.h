
#define LOG_ERROR(fmt, ...)                                 \
    do {                                                    \
        fprintf(stderr, "%s:%d:%s(): " fmt "\n",            \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);   \
    } while (0)

#define MIN(a, b) (((a) < (b))? (a) : (b))
#define MAX(a, b) (((a) > (b))? (a) : (b))
