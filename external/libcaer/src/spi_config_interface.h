#ifndef LIBCAER_SRC_SPI_CONFIG_INTERFACE_H_
#define LIBCAER_SRC_SPI_CONFIG_INTERFACE_H_

#include "libcaer/libcaer.h"

#define SPI_CONFIG_MSG_SIZE 6

PACKED_STRUCT(struct spi_config_params {
	uint8_t moduleAddr;
	uint8_t paramAddr;
	uint32_t param;
});

typedef struct spi_config_params *spiConfigParams;

static bool spiConfigSendMultiple(void *state, spiConfigParams configs, uint16_t numConfigs);
static bool spiConfigSendMultipleAsync(void *state, spiConfigParams configs, uint16_t numConfigs,
	void (*configSendCallback)(void *configSendCallbackPtr, int status), void *configSendCallbackPtr);

static bool spiConfigSend(void *state, uint16_t moduleAddr, uint16_t paramAddr, uint32_t param);
static bool spiConfigSendAsync(void *state, uint16_t moduleAddr, uint16_t paramAddr, uint32_t param,
	void (*configSendCallback)(void *configSendCallbackPtr, int status), void *configSendCallbackPtr);

static bool spiConfigReceive(void *state, uint16_t moduleAddr, uint16_t paramAddr, uint32_t *param);
static bool spiConfigReceiveAsync(void *state, uint16_t moduleAddr, uint16_t paramAddr,
	void (*configReceiveCallback)(void *configReceiveCallbackPtr, int status, uint32_t param),
	void *configReceiveCallbackPtr);

static inline int16_t safeFlip16(const int16_t value) {
	// Safe sign flip considering unequal positive/negative integer ranges.
	if (value == INT16_MIN) {
		return INT16_MAX;
	}

	return I16T(-value);
}

#endif /* LIBCAER_SRC_SPI_CONFIG_INTERFACE_H_ */
