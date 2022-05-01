#ifndef NAMES_H
#define NAMES_H

#include "fcntl.h"

const size_t MAPPING_SIZE = 4 * 1024;

const unsigned int params = 0777;

const char * InputFile = "input.shared";
const char * InputSemaphore = "/input.semaphore";

const char * ErrorFile = "error.shared";
const char * ErrorSemaphore = "/error.semaphore";

#endif