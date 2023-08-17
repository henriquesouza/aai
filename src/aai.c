#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>
#include<sys/time.h>
#include <stdbool.h>

#include "aai.h"

static allocationsQueue* q = NULL;
static void* (*real_malloc)(size_t) = NULL ;
static void* (*real_calloc)(size_t, size_t) = NULL ;
static void (*real_free)(void*) = NULL ;
static bool aaiLock = true;
static bool aaiLockCalloc = true;
static FILE* f = NULL;


time_t getCurrentTime()
{
    time_t t;
    return time(&t);
}

long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);

    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

void printQueue(allocationsQueue* q)
{
    allocationsQueue* next = q->first;

    while (next != NULL){
        aaiLock = false;

        // if (f == NULL) f = fopen("/data/local/tmp/file.csv", "w+");
        if (f == NULL) f = fopen("/tmp/file.csv", "w+");
        // if (f == NULL) f = fopen("/storage/emulated/0/Android/data/org.videolan.vlc/files/file.csv", "w+");
        // printf(
        //     "%p %c = %d, at %d\n",
        //     next->address, next->type, next->size, next->time);
        fprintf(
            f,
            "%p;%c;%ld;%lld\n",
            next->address, next->type, next->size, next->time);
        next = next->next;
        aaiLock = true;
    }
}

void enqueu(allocationsQueue* q, size_t size, char type, void* address)
{
    allocationsQueue* new_item = (allocationsQueue*) real_malloc(
        sizeof(allocationsQueue));
    new_item->next = NULL;
    new_item->size = size;
    new_item->type = type;
    new_item->address = address;
    new_item->time = timeInMilliseconds();

    allocationsQueue* previous = q->last;

    if (previous != NULL) previous->next = new_item;
    else q->first = new_item;

    q->last = new_item;
    aaiLockCalloc = !aaiLockCalloc;
}

void exitOnSeconds(int s)
{
    allocationsQueue* first = q->first;

    // Quit after s seconds of execution
    if ((timeInMilliseconds() - first->time) / 1000 >= s) {
        printQueue(q);
        aaiLock = false;
        exit(0);
    }
}

void* malloc(size_t size)
{
    if (real_malloc == NULL) real_malloc = dlsym(RTLD_NEXT, "malloc");

    if (q == NULL) q = (allocationsQueue*) real_malloc(
        sizeof(allocationsQueue));

    else if (aaiLock) exitOnSeconds(3);

    void* alloced = real_malloc(size);

    if (aaiLock) enqueu(q, size, 'm', alloced);

    return alloced;
}

void* calloc(size_t nmemb, size_t size)
{
    if (real_calloc == NULL) real_calloc = dlsym(RTLD_NEXT, "calloc");

    void* alloced = real_calloc(nmemb, size);

    if (aaiLock && !aaiLockCalloc){ // TODO: hypothesis: make it so no allocations are logged while a log is being registered
        exitOnSeconds(3);
        enqueu(q, size*nmemb, 'c', alloced);
        aaiLockCalloc = !aaiLockCalloc;
    }

    return alloced;
}

void free(void* address)
{
    if (real_free == NULL) real_free = dlsym(RTLD_NEXT, "free");

    if (aaiLock) exitOnSeconds(3);

    if (aaiLock) enqueu(q, 0, 'f', address);

    real_free(address);
}

