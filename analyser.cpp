/*
 * analyser.cpp
 * Quintain-DemoFS
 *
 * Created by Jinglei Ren on 5/10/2013
 * Copyright (c) 2013 Jinglei Ren <jinglei.ren@stanzax.org>
 */

#include <cstdio>
#include <fstream>
#include <unordered_set>
#include <cstring>
#include <cstdint>
#include <openssl/md5.h>
#include "easyzlib.h"

#define NUM_BUCKS (1024 * 16)
#define LOG_PATH_LEN 256

using namespace std;

class Digest {
public:
  const unsigned char *value() const {
    return value_;
  }

  size_t Key() const {
    return *(size_t *) value_;
  }

private:
  unsigned char value_[MD5_DIGEST_LENGTH];
};

namespace std {
template<>
struct hash<Digest> : public unary_function<Digest, size_t> {
  size_t operator()(const Digest &digest) const {
    return digest.Key();
  }
};

template<>
struct equal_to<Digest> : binary_function<Digest, Digest, bool> {
  bool operator()(const Digest &a, const Digest &b) const {
    return 0 == memcmp(a.value(), b.value(), MD5_DIGEST_LENGTH);
  }
};
}

unordered_set<Digest> index_set(NUM_BUCKS);
uint64_t total_len = 0;
uint64_t compress_len = 0;
unsigned char *dest = NULL;

void handle_data(const unsigned char *data, const uint64_t len,
    const uint64_t dedup_len) {
  total_len += len;
  
  // deduplication
  Digest digest;
  uint64_t pos = 0;
  uint64_t digest_len = 0;
  for (pos = 0; pos < len; pos += digest_len) {
    digest_len = pos + dedup_len > len ? len - pos : dedup_len;
    MD5(data + pos, digest_len, (unsigned char *) digest.value());
    index_set.insert(digest);
  }

  // compression
  long dest_len = EZ_COMPRESSMAXDESTLENGTH(len);
  dest = (unsigned char *)realloc(dest, dest_len);
  int err = ezcompress(dest, &dest_len, data, len);
  if (err) {
    fprintf(stderr, "[analyser] failed to compress: err = %d\n", err);
    compress_len += len;
  } else {
    //printf("\t%f", (double)dest_len/len);
    compress_len += dest_len;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Usage: %s [log_path_prefix] [number_of_files] [dedup_length]\n", argv[0]);
    return -1;
  }
  const char *file_pre = argv[1];
  const int num_files = atoi(argv[2]);
  const uint64_t dedup_len = atoi(argv[3]);

  char file_name[LOG_PATH_LEN];
  for (int i = 0; i < num_files; ++i) {
    sprintf(file_name, "%s%d", file_pre, i);
    fstream log_file(file_name, ios::in | ios::binary);
    uint64_t seg_len;
    char *data = NULL;

    log_file.read((char *) &seg_len, sizeof(seg_len));
    while (log_file) {
      data = (char *) realloc(data, seg_len);
      log_file.read(data, seg_len);
      if (log_file) {
        handle_data((unsigned char *) data, seg_len, dedup_len);
      } else {
        printf("[Err] failed to read after %lu bytes.\n", total_len);
        break;
      }
      log_file.read((char *) &seg_len, sizeof(seg_len));
    } // while
    free(data);
    log_file.close();
  } // for

  uint64_t rest_len = index_set.size() * dedup_len;
  printf("\nTotal file data length: %lu bytes\n", total_len);
  printf("Data length after dedup: %lu bytes\n", rest_len);
  printf("Dedup ratio: %f%% (dedup lenght = %lu)\n",
      (double)rest_len/total_len * 100, dedup_len);
  printf("Compression ratio: %f%%\n\n", (double)compress_len/total_len * 100);

  if (dest) free(dest);
  return 0;
}

