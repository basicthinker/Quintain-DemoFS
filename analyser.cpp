/*
 * analyser.c
 * Quintain-DemoFS
 *
 * Created by Jinglei Ren on 5/10/2013
 * Copyright (c) 2013 Jinglei Ren <jinglei.ren@stanzax.org>
*/

#include <cstdio>
#include <fstream>
#include <stdint.h>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s [log_file_path]\n", argv[0]);
    return -1;
  }
  char *file_name = argv[1];

  fstream log_file(file_name, ios::in | ios::binary);
  uint64_t len;
  char *data = NULL;

  log_file.read((char *)&len, sizeof(len));
  while (log_file) {
    data = (char *)realloc(data, len);
    log_file.read(data, len);
    log_file.read((char *)&len, sizeof(len));
  }

  free(data);
  log_file.close();
  return 0;
}

