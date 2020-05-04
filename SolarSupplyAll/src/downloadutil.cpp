#include "downloadutil.h"
#include <iostream>


void downloadFailed(emscripten_fetch_t *fetch)
{
  //printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
  std::cout << "DL failed" << std::endl;
  emscripten_fetch_close(fetch); // Also free data on failure.
}


