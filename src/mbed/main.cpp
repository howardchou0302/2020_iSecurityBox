/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "MFRC522.h"
#include "Servo.h"
#include "rfid.h"
#include "TCPSocket.h"
#include <time.h>
#include <string>
#include <iostream>

using namespace std;

#define WIFI_IDW0XX1    2

#if (defined(TARGET_DISCO_L475VG_IOT01A) || defined(TARGET_DISCO_F413ZH))
#include "ISM43362Interface.h"
ISM43362Interface wifi(/*MBED_CONF_APP_WIFI_SPI_MOSI, MBED_CONF_APP_WIFI_SPI_MISO, MBED_CONF_APP_WIFI_SPI_SCLK, MBED_CONF_APP_WIFI_SPI_NSS, MBED_CONF_APP_WIFI_RESET, MBED_CONF_APP_WIFI_DATAREADY, MBED_CONF_APP_WIFI_WAKEUP,*/ false);

#else // External WiFi modules

#if MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1
#include "SpwfSAInterface.h"
SpwfSAInterface wifi(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);
#endif // MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1

#endif

MFRC522 *mfrc522 = new MFRC522(D11, D12, D13, D10, A0);
Servo servo(PWM_OUT);

const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int main()
{
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi.get_mac_address());
    printf("IP: %s\n", wifi.get_ip_address());
    printf("Netmask: %s\n", wifi.get_netmask());
    printf("Gateway: %s\n", wifi.get_gateway());
    printf("RSSI: %d\n\n", wifi.get_rssi());
    TCPSocket socket;
    SocketAddress addr("192.168.50.124",8888);
    nsapi_error_t response;
    // Open a socket on the network interface, and create a TCP connection to addr
    response = socket.open(&wifi);
    if (0 != response){
        printf("Error opening: %d\n", response);
    }
    response = socket.connect(addr);
    if (0 != response){
        printf("Error connecting: %d\n", response);
    }
    socket.set_blocking(1);
    
    clock_t t = clock();
    int buffer_sent;
    servo.position(0);
    RFID_Reader rfidReader(mfrc522);
    printf("Scanning RFID...\n");
    printf("cycles per sec: %d\n", CLOCKS_PER_SEC);
    while(1)
    {
        if(rfidReader.read())
        {
            printf("New card detected: \n");
            t = clock();
            printf("Time: %d\n", t);
            printf("Card ID: ");
            char id[2 * rfidReader.getSize()];
            rfidReader.getCharID(id);
            for(int i = 0; i < 8; ++i) printf("%c", id[i]);
            printf("\n");
            buffer_sent = socket.send(id, 8);
            servo.position(180);
        }
        if(float(clock() - t)/CLOCKS_PER_SEC > 5){
            printf("time expired: %d\n", clock());
            servo.position(0);
            t = clock();
        }
    }
    socket.close();
    return 0;
}