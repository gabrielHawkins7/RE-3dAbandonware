#include <ctype.h>
#include <stdio.h>
#include <string.h>

int hash(int param_1) {
  int iVar1;
  iVar1 = (param_1 % 127773) * 16807 + (param_1 / 127773) * -2836;
  if (iVar1 < 1) {
    iVar1 += 2147483647;
  }
  return iVar1;
}

int CHKSUM(char *serial_key) {

  size_t len = strlen(serial_key);
  int num_blocks = (len + 3) >> 2;

  unsigned int state = 0;
  const char *p = serial_key;

  for (int i = 0; i < num_blocks; i++) {
    unsigned int block = 0;
    for (int j = 0; j < 4; j++) {
      block = (block << 8) | (unsigned char)*p++;
    }

    state ^= block;

    if (state & 0x80000000)
      state &= 0x7FFFFFFF;
    if (state == 0)
      state = 1;

    state = hash(hash(state));
  }

  unsigned int masked = state & 0xFEF7FDFF;
  unsigned int high_part = masked << 21;
  if (high_part & 0x80000000)
    high_part &= 0x7FFFFFFF;
  unsigned int low_part = (masked >> 11) & 0x1FFFFF;
  unsigned int combined = high_part | low_part;

  if (combined == 0)
    combined = 1;

  return hash(combined) % 13824;
}

int validate_key(char *key, char *HWID, int PARAM_1) {
  short len = strlen(key);

  /* 1. Basic length and prefix match */
  if (len != 19 && len != 23 && len != 26 && len != 30)
    return 0;
  if (key[0] != HWID[0] || key[1] != HWID[1])
    return 0;

  /* 2. Characters 2-3 must be digits */
  if (!isdigit(key[2]) || !isdigit(key[3]))
    return 0;

  /* 3. Product identifier at position 4 */
  if (PARAM_1 == 0) {
    if (key[4] != 'M' && key[4] != 'C')
      return 0;
  } else if (PARAM_1 == 1) {
    if (key[4] != 'C')
      return 0;
  }
  /* 4. Position 5: allowed letters */
  if (!strchr("BCENRU", key[5]))
    return 0;

  /* 5. Position 6: allowed letters */
  if (!strchr("BCDFGMJKLNPSTZ", key[6]))
    return 0;

  /* 6. Position 7: hyphen */
  if (key[7] != '-')
    return 0;

  /* 7. Positions 8-14 must be digits */
  for (int i = 8; i < 15; i++) {
    if (!isdigit(key[i]))
      return 0;
  }

  /* 8. Position 15: hyphen */
  if (key[15] != '-')
    return 0;
  /* 9. Positions 16-18: 3-letter base-24 checksum value */
  int chkValue = 0;
  for (int i = 16; i < 19; i++) {
    char c = key[i];
    if (c < 'A' || c > 'Z' || c == 'I' || c == 'O')
      return 0;
    chkValue *= 24;
    if (c <= 'H')
      chkValue += c - 'A';     /* 0..7 */
    else if (c <= 'N')         /* I,O already excluded, so J..N */
      chkValue += c - 'J' + 8; /* 8..12 */
    else
      chkValue += c - 'P' + 13; /* 13..23 */
  }

  char chkbuf[18];
  for (int i = 0; i <= sizeof(chkbuf); i++) {
    chkbuf[i] = 'A';
  }

  // printf("Cheksum Buffer: %s \n", chkbuf);
  //  memcpy_s(chkbuf, 15, key, strlen(key));
  memcpy(chkbuf, key, 16);

  // printf("Cheksum Buffer: %s \n", chkbuf);

  int chksum = CHKSUM(chkbuf);
  printf("chkvalue: %d | chksum: %d\n", chkValue, chksum);
  if (chksum != chkValue)
    return 0;

  if (key[19] == '\0')
    return 1;

  return 0;
}
