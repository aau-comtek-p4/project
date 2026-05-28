#include "general/misc/crc.h"
#include "esp_rom_crc.h"
#include "general/interfaces/io/io.h"
uint32_t CRC32(uint8_t const *buf, uint64_t len) {
  return esp_rom_crc32_le(CRC32_START, buf, len);
}
