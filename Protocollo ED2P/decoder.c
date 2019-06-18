//
//  main.c
//  ed2p-decoder
//
//  Created by Federico Runco on 18/06/2019.
//  Copyright © 2019 Federico Runco. All rights reserved.
//

#include <stdio.h>
#include <inttypes.h>

int psr (uint8_t[6]);

int main(int argc, const char * argv[]) {
    
    uint8_t datagram[6];
    FILE *fp;
    
    printf("ED2P Decoder - simulatore\n");
    fp = fopen("datagram.ed2p", "r");
    if (fp == NULL) return 1;
    fread(datagram, 1, sizeof(datagram), fp);
    fclose(fp);
    
    // effettivo parsing del valore dal pacchetto
    int val = psr(datagram);
    if (val >= 0) printf("Dato pacchetto: %d\n", val);
    else {
        if (val == -2) printf("Errore checksum, dato trasmesso male");
        if (val == -1) printf("Errore intestazione, sicuri sia il pacchetto giusto?");
    }
    // stop parsing valore
    
    return 0;
}

int psr (uint8_t datagram[6]){
    uint8_t ckAccum = 0;
    int len = 5;
    //if (strncmp(datagram, "E2DP", 4)) return -1; lentissimo
    if (datagram[0] != 'E') return -1;
    if (datagram[1] != 'D') return -1;
    if (datagram[2] != '2') return -1;
    if (datagram[3] != 'P') return -1;
    while(len--) ckAccum += datagram[len];
    if (ckAccum != datagram[5]) return -2;
    return (int)datagram[4];
}
