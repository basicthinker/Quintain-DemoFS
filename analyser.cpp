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

#define NUM_BUCKS (1024 * 16)

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

void handle_data(const unsigned char *data, const uint64_t len,
    const uint64_t dedup_len) {
  Digest digest;
  uint64_t pos = 0;
  uint64_t digest_len = 0;

  for (pos = 0; pos < len; pos += digest_len) {
    digest_len = pos + dedup_len > len ? len - pos : dedup_len;
    MD5(data + pos, digest_len, (unsigned char *) digest.value());
    index_set.insert(digest);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s [log_file_path] [dedup_length]\n", argv[0]);
    return -1;
  }
  char *file_name = argv[1];
  const uint64_t dedup_len = atoi(argv[2]);

  fstream log_file(file_name, ios::in | ios::binary);
  uint64_t seg_len;
  uint64_t total_len = 0;
  char *data = NULL;

  log_file.read((char *) &seg_len, sizeof(seg_len));
  while (log_file) {
    data = (char *) realloc(data, seg_len);
    log_file.read(data, seg_len);
    if (log_file) {
      total_len += seg_len;
      handle_data((unsigned char *) data, seg_len, dedup_len);
    } else {
      printf("[Err] failed to read after %lu bytes.\n", total_len);
      break;
    }
    log_file.read((char *) &seg_len, sizeof(seg_len));
  }

  uint64_t rest_len = index_set.size() * dedup_len;
  printf("Total file data length: %lu bytes\n", total_len);
  printf("Data length after dedup: %lu bytes\n", rest_len);
  printf("Dedup ratio: %f%% (dedup lenght = %lu)\n",
      (double)rest_len/total_len * 100, dedup_len);

  free(data);
  log_file.close();
  return 0;
}

