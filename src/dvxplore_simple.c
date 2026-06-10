#include <libcaer/libcaer.h>
#include <libcaer/devices/dvxplorer.h> // Fixed header

#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static atomic_bool globalShutdown = ATOMIC_VAR_INIT(false);

static void globalShutdownSignalHandler(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        atomic_store(&globalShutdown, true);
    }
}

static void usbShutdownHandler(void *ptr) {
    (void) (ptr);
    atomic_store(&globalShutdown, true);
}

int main(void) {
    // Install signal handlers
#if defined(_WIN32)
    if (signal(SIGTERM, &globalShutdownSignalHandler) == SIG_ERR) {
        caerLog(CAER_LOG_CRITICAL, "ShutdownAction", "Failed to set signal handler for SIGTERM. Error: %d.", errno);
        return (EXIT_FAILURE);
    }
    if (signal(SIGINT, &globalShutdownSignalHandler) == SIG_ERR) {
        caerLog(CAER_LOG_CRITICAL, "ShutdownAction", "Failed to set signal handler for SIGINT. Error: %d.", errno);
        return (EXIT_FAILURE);
    }
#else
    struct sigaction shutdownAction;
    shutdownAction.sa_handler = &globalShutdownSignalHandler;
    shutdownAction.sa_flags   = 0;
    sigemptyset(&shutdownAction.sa_mask);
    sigaddset(&shutdownAction.sa_mask, SIGTERM);
    sigaddset(&shutdownAction.sa_mask, SIGINT);

    if (sigaction(SIGTERM, &shutdownAction, NULL) == -1) {
        caerLog(CAER_LOG_CRITICAL, "ShutdownAction", "Failed to set signal handler for SIGTERM. Error: %d.", errno);
        return (EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &shutdownAction, NULL) == -1) {
        caerLog(CAER_LOG_CRITICAL, "ShutdownAction", "Failed to set signal handler for SIGINT. Error: %d.", errno);
        return (EXIT_FAILURE);
    }
#endif

    // Open DVXplore
    caerDeviceHandle dvxplr_hndl = caerDeviceOpen(1, CAER_DEVICE_DVXPLORER, 0, 0, NULL);
    if (dvxplr_hndl == NULL) {
        return (EXIT_FAILURE);
    }

    // Fixed struct and function call for device info
    struct caer_dvx_info dvxplr_info = caerDVXplorerInfoGet(dvxplr_hndl);

    printf("%s --- ID: %d, Master: %d, DVS X: %d, DVS Y: %d, Firmware: %d.\n", 
        dvxplr_info.deviceString,
        dvxplr_info.deviceID, 
        dvxplr_info.deviceIsMaster, 
        dvxplr_info.dvsSizeX, 
        dvxplr_info.dvsSizeY,
        dvxplr_info.firmwareVersion);

    // Send default config
    caerDeviceSendDefaultConfig(dvxplr_hndl);

    // Start data streaming
    caerDeviceDataStart(dvxplr_hndl, NULL, NULL, NULL, &usbShutdownHandler, NULL);

    // Turn on blocking data exchange
    caerDeviceConfigSet(dvxplr_hndl, CAER_HOST_CONFIG_DATAEXCHANGE, CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, true);

    while (!atomic_load_explicit(&globalShutdown, memory_order_relaxed)) {
        caerEventPacketContainer packetContainer = caerDeviceDataGet(dvxplr_hndl);
        if (packetContainer == NULL) {
            continue; 
        }

        int32_t packetNum = caerEventPacketContainerGetEventPacketsNumber(packetContainer);

        for (int32_t i = 0; i < packetNum; i++) {
            caerEventPacketHeader packetHeader = caerEventPacketContainerGetEventPacket(packetContainer, i);
            if (packetHeader == NULL) {
                continue; 
            }

            // Fixed type checking logic
            if (caerEventPacketHeaderGetEventType(packetHeader) == POLARITY_EVENT) {
                caerPolarityEventPacket polarity = (caerPolarityEventPacket) packetHeader;

                // Ensure we have at least one event to read
                if (caerEventPacketHeaderGetEventNumber(packetHeader) > 0) {
                    caerPolarityEvent firstEvent = caerPolarityEventPacketGetEvent(polarity, 0);

                    int32_t ts = caerPolarityEventGetTimestamp(firstEvent);
                    uint16_t x = caerPolarityEventGetX(firstEvent);
                    uint16_t y = caerPolarityEventGetY(firstEvent);
                    bool pol   = caerPolarityEventGetPolarity(firstEvent);

                    printf("First polarity event - ts: %d, x: %d, y: %d, pol: %d.\n", ts, x, y, pol);
                }
            }
        }

        caerEventPacketContainerFree(packetContainer);
    }

    caerDeviceDataStop(dvxplr_hndl);
    caerDeviceClose(&dvxplr_hndl); // Fixed handle pointer mismatch

    printf("Shutdown successful.\n");
    return (EXIT_SUCCESS);
}