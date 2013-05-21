/*
 * test.cpp
 * Quintain-DemoFS
 *
 * Created by Jinglei Ren on 5/11/2013
 * Copyright (c) 2013 Jinglei Ren <jinglei.ren@stanzax.org>
 */

#include <fstream>
#include <cstdint>
#include <cstdlib>

#define NUM_CASES 10
#define SEG_LEN 1000

using namespace std;

void fill_data(char *data, int len, int mode) {
  int i;
  switch (mode) {
  case 0:
    for (i = 0; i < len; ++i) {
      *(data + i) = (char)rand();
    }
    break;
  case 1: // half same
    for (i = 0; i < len / 2; ++i) {
      *(data + i) = (char)mode;
    }
    for (; i < len; ++i) {
      *(data + i) = (char)rand();
    }
    break;
  case 2: // full same
    for (i = 0; i < len; ++i) {
      *(data + i) = (char)mode;
    }
    break;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s [output file]\n",  argv[0]);
    return -1;
  }

  fstream test_file(argv[1], ios::out | ios::binary);
  char data[SEG_LEN];
  uint64_t seg_len = SEG_LEN;

  for (int i = 0; i < NUM_CASES; ++i) {
    fill_data(data, SEG_LEN, i % 3);
    test_file.write((char *)&seg_len, sizeof(seg_len));
    test_file.write(data, SEG_LEN);
  }

  seg_len = (SEG_LEN >> 2) * 3;
  fill_data(data, seg_len, 2);
  for (int i = 0; i < 2; ++i) {  
    test_file.write((char *)&seg_len, sizeof(seg_len));
    test_file.write(data, seg_len);
  }

  printf("SEG_LEN = %d, NUM_CASES = %d\n", SEG_LEN, NUM_CASES);
  printf("total data:\t%lu\n", SEG_LEN * NUM_CASES + seg_len);
  printf("dedup by half:\t%d\t%d\n", 7 * SEG_LEN, 7 * SEG_LEN - (SEG_LEN >> 2));
  printf("dedup by whole:\t%d\t%d\n", 9 * SEG_LEN, 9 * SEG_LEN - (SEG_LEN >> 2));
  test_file.close();
  return 0;
}
