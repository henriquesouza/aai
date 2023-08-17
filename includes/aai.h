typedef struct allocationQueue_t {
    size_t size;
    // time_t time;
    long long time;
    char type;
    void* address;
    void* next;
    void* last;
    void* first;
} allocationsQueue;

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
// void* realloc(size_t size);
void free(void* p);

