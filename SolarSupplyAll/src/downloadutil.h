#pragma once


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#endif


void downloadFailed(emscripten_fetch_t *fetch);

