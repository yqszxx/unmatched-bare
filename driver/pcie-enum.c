#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "pcie.h"

#define PRIMARY     0X18
#define SECONDARY   0x19
#define SUBORDINATE 0x1a
#define VENDORID    0x00
#define DEVICEID    0x02
#define HEADERTYPE  0x0e

typedef struct a {
    bool bridge;
    uint8_t pri;
    uint8_t sec;
    uint8_t sub;
    struct a* father;
    struct a* childrenList[32];
    uint8_t b;
    uint8_t d;
    uint8_t f;
    uint16_t vendorID;
    uint16_t deviceID;
} Node;

Node A = {true, 0, 1, 1, 0, {NULL}, 0, 0, 0, 0x22bf, 0xbf22};

static int bus = 0;
static int devNum = 0;

void updateSubReg(Node* node) {
    if (node == NULL) {
        return;
    }
    if (node->bridge == true) {
        node->sub = node->sub + 1;
        pcieCfgWrite8(node->b, node->d, node->f, SUBORDINATE, node->sub);
    }
    updateSubReg(node->father);
}

void retractSubReg(Node* node) {
    if (node == NULL) {
        return;
    }
    if (node->bridge == true) {
        node->sub = node->sub - 1;
        pcieCfgWrite8(node->b, node->d, node->f, SUBORDINATE, node->sub);
    }
    retractSubReg(node->father);
}

void search(Node* node) {

    uint16_t vendorID;
    uint16_t deviceID;
    uint8_t  headerType;
    bus++; // every bridge's sec bus can hold max 32 devices
    updateSubReg(node->father);

    for (int i = 0; i < 32; i++) {
        vendorID = pcieCfgRead16(node->sec, i, 0, VENDORID);

        if (vendorID != 0xffff) {  // valid vendorID -> device exists
            deviceID = pcieCfgRead16(node->sec, i, 0, DEVICEID);
            headerType = pcieCfgRead8(node->sec, i, 0, HEADERTYPE);

            // create new node and initialize to zero and (void*)zero
            Node* newDevice = calloc(1, sizeof(Node));

            // update bridge node's regs
            node->childrenList[i] = newDevice;

            // initialize newDevice's regs
            // TODO: 先假设都是single function
            newDevice->father = node;
            newDevice->b = node->sec;
            newDevice->d = i;
            newDevice->f = 0;
            newDevice->vendorID = vendorID;
            newDevice->deviceID = deviceID;

            if ((headerType & 1)== 1) {  // bridge
                newDevice->bridge = true;
                newDevice->pri = node->sec;  // child bridge's pri = father bridge's sec
                newDevice->sec = bus + 1;
                newDevice->sub = bus + 1;

                pcieCfgWrite8(newDevice->b, newDevice->d, newDevice->f, PRIMARY, newDevice->pri);
                pcieCfgWrite8(newDevice->b, newDevice->d, newDevice->f, SECONDARY, newDevice->sec);
                pcieCfgWrite8(newDevice->b, newDevice->d, newDevice->f, SUBORDINATE, newDevice->sub);
                
                search(newDevice);
            } else { // end point
                newDevice->bridge = false;
            }
        }
    }  // end: for loop
}

void displayTopology(Node* node) {
    if (node == &A) {
        printf("Bridge:   bdf = (%2d, %2d, %2d)\r\n", node->b, node->d, node->f);
        printf("VendorID = %4x, DeviceID = %4x\r\n", node->vendorID, node->deviceID);
        printf("Pri = %2d, Sec = %2d, Sub = %2d\r\n", node->pri, node->sec, node->sub);
        printf("----------------------------------------------------\r\n");
        devNum++;
    }
    for (int i = 0; i < 32; i++) {
        if (node->childrenList[i] == NULL) {
            continue;
        }

        devNum++;
        if (node->childrenList[i]->bridge == true) {
            printf("Bridge:   bdf = (%2d, %2d, %2d)\r\n", node->childrenList[i]->b, node->childrenList[i]->d, node->childrenList[i]->f);
            printf("VendorID = %4x, DeviceID = %4x\r\n", node->childrenList[i]->vendorID, node->childrenList[i]->deviceID);
            printf("Pri = %2d, Sec = %2d, Sub = %2d\r\n", node->childrenList[i]->pri, node->childrenList[i]->sec, node->childrenList[i]->sub);
            printf("----------------------------------------------------\r\n");
            displayTopology(node->childrenList[i]);
        } else {  // EP
            printf("EndPoint: bdf = (%2d, %2d, %2d)\r\n", node->childrenList[i]->b, node->childrenList[i]->d, node->childrenList[i]->f);
            printf("VendorID = %4x, DeviceID = %4x\r\n", node->childrenList[i]->vendorID, node->childrenList[i]->deviceID);
            printf("----------------------------------------------------\r\n");
        }
    }
}

void pcieEnum() {
    pcieCfgWrite8(A.b, A.d, A.f, PRIMARY, A.pri);
    pcieCfgWrite8(A.b, A.d, A.f, SECONDARY, A.sec);
    pcieCfgWrite8(A.b, A.d, A.f, SUBORDINATE, A.sub);
    A.vendorID = pcieCfgRead16(A.b, A.d, A.f, VENDORID);
    A.deviceID = pcieCfgRead16(A.b, A.d, A.f, DEVICEID);

    search(&A);

    displayTopology(&A);

    printf("Devices NUM = %d\r\n", devNum);
}