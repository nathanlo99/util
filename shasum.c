
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define rot32(a , b) (((a) >> (b)) | ((a) << (32 - (b))))
#define ch(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define ep0(x) (rot32(x,2) ^ rot32(x,13) ^ rot32(x,22))
#define ep1(x) (rot32(x,6) ^ rot32(x,11) ^ rot32(x,25))
#define sig0(x) (rot32(x,7) ^ rot32(x,18) ^ ((x) >> 3))
#define sig1(x) (rot32(x,17) ^ rot32(x,19) ^ ((x) >> 10))

typedef struct {
  uint32_t values[8];
} shasum_t;

void print_hex32(const uint32_t a) {
  static const char *chars = "0123456789abcdef";
  for (int k = 0; k < 8; k++)
    printf("%c", chars[(a >> (28 - 4 * k)) & 15]);
}

void print_shasum(const shasum_t s, const char *name) {
  for (int i = 0; i < 8; i++)
    print_hex32(s.values[i]);
  printf("  %s\n", name);
}

uint32_t endian_swap(const uint32_t num) {
  return ((num >> 24) & 0x000000ff) | // move byte 3 to byte 0
         ((num <<  8) & 0x00ff0000) | // move byte 1 to byte 2
         ((num >>  8) & 0x0000ff00) | // move byte 2 to byte 1
         ((num << 24) & 0xff000000);
}

shasum_t SHA(FILE *f, const int is_stdin) {
  static const uint32_t k[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
      0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
      0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
      0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
      0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
      0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
      0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
      0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
  };
  static uint32_t w[64]; // For temporary storage during chunk processing

  uint32_t h0 = 0x6a09e667;
  uint32_t h1 = 0xbb67ae85;
  uint32_t h2 = 0x3c6ef372;
  uint32_t h3 = 0xa54ff53a;
  uint32_t h4 = 0x510e527f;
  uint32_t h5 = 0x9b05688c;
  uint32_t h6 = 0x1f83d9ab;
  uint32_t h7 = 0x5be0cd19;

  // Gets the size of the file
  fseek(f, 0L, SEEK_END);
  const uint64_t size = ftell(f);
  rewind(f);

  // We want to pad the message with a single 1 bit, and just enough
  // 0 bits to reach a total message size
  int32_t num_padding_bytes = 56 - size % 64;
  if (num_padding_bytes < 1)
    num_padding_bytes += 64;

  const uint64_t L = size * 8ULL;
  const uint64_t total_size = size + num_padding_bytes + 8;

  // Allocate the memory
  uint8_t *buffer = (uint8_t *)calloc(total_size + 1, sizeof(uint8_t));
  assert(total_size % 64 == 0);

  if (!buffer)
    fclose(f), fputs("File is too big\n", stderr), exit(1);
  if (size > 0 && fread(buffer, size, 1, f) != 1) {
    fclose(f), free(buffer);
    fputs("Could not read file\n", stderr), exit(1);
  }
  fclose(f);

  // Pad with the 1, (0's are already padded from calloc)
  buffer[size] = 0x80;

  // Pad last 64 bits with the size, in bits, of the original message
  for (int i = 0; i < 8; i++) {
    buffer[size + num_padding_bytes + i] = (L >> (56 - 8 * i)) & 255;
  }

  for (int i = 0; i < total_size; i += 64) {
    const uint32_t *chunk = (uint32_t*)&buffer[i];
    for (int j = 0; j < 16; j++)
      w[j] = endian_swap(chunk[j]);

    for (int j = 16; j < 64; j++)
      w[j] = w[j - 16] + sig0(w[j - 15]) + w[j - 7] + sig1(w[j - 2]);

    uint32_t a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;

    for (int j = 0; j < 64; j++) {
      const uint32_t t1 = h + ep1(e) + ch(e, f, g) + k[j] + w[j],
                     t2 = ep0(a) + maj(a, b, c);

      h = g; g = f; f = e; e = d + t1;
      d = c; c = b; b = a; a = t1 + t2;
    }

    h0 += a; h1 += b; h2 += c; h3 += d;
    h4 += e; h5 += f; h6 += g; h7 += h;
  }

  free(buffer);
  return (shasum_t){{h0, h1, h2, h3, h4, h5, h6, h7}};
}


int main(int argc, char *argv[]) {
  if (argc < 2) {
    // TODO Implement stdin reading and then SHA on the stdin
    printf("Usage: shasum [file] ...\n");
  } else {
    for (int argn = 1; argn < argc; argn++) {
      FILE *f = fopen(argv[argn], "r");
      if (f != NULL) {
        const shasum_t shasum = SHA(f, 0);
        print_shasum(shasum, argv[argn]);
        fclose(f);
      }
    }
  }
}
