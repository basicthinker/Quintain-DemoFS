/*
 * analyser.c
 * Quintain-DemoFS
 *
 * Created by Jinglei Ren on 5/10/2013
 * Copyright (c) 2013 Jinglei Ren <jinglei.ren@stanzax.org>
*/

#include <cstdio>
#include <fstream>
#include <unordered_set>
#include <stdint.h>
#include <openssl/md5.h>

#define DEDUP_LEN 1024 // bytes

using namespace std;

class Digest {
public:
  unsigned char *Value() {
    return value_;
  }
  size_t Key() const {
    return *(size_t *)value_;
  }

private:
  unsigned char value_[MD5_DIGEST_LENGTH];
};

template <>
class hash<Digest> : public unary_function<struct digest, size_t> {
  size_t operator()(const Digest &digest) const {
    return digest.Key();
  }
};

void handle_data(const unsigned char *data, const uint64_t len) {
  Digest digest;
  uint64_t pos = 0;
  uint64_t dedup_len;

  while (pos < len) {
    dedup_len = pos + DEDUP_LEN > len ? len - pos : DEDUP_LEN;
    MD5(data + pos, dedup_len, digest.Value());
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s [log_file_path]\n", argv[0]);
    return -1;
  }
  char *file_name = argv[1];

  fstream log_file(file_name, ios::in | ios::binary);
  uint64_t seg_len;
  char *data = NULL;
  
  log_file.read((char *)&seg_len, sizeof(seg_len));
  while (log_file) {
    data = (char *)realloc(data, seg_len);
    log_file.read(data, seg_len);
    handle_data((unsigned char *)data, seg_len);
    log_file.read((char *)&seg_len, sizeof(seg_len));
  }

  free(data);
  log_file.close();
  return 0;
}

