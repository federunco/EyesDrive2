//
//  main.c
//  ed2p-encoder
//
//  Created by Federico Runco on 18/06/2019.
//  Copyright © 2019 Federico Runco. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <inttypes.h>

clock_t start, end;
double cputim;

int main(int argc, const char * argv[]) {
    uint8_t datagram[6] = { 'E', 'D', '2', 'P', 'D', 'C'};
    uint8_t val;
    uint8_t checksum = 0, len = 5;
    printf("ED2P Encoder - simulatore\n");
    do {
        printf("\nMappa valore (0-255)> ");
        scanf("%" SCNu8, &val);
    } while (val < 0 || val > 255);
    start = clock();
    datagram[4] = val;
    while(len--) checksum += datagram[len];
    datagram[5] = checksum;
    end = clock();
    cputim = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("\nTempo di esecuzione su CPU: %f sec\n", cputim);
    printf("\nHex dump pacchetto:\n");
    char buffer[9] = {0};
    for (int j = 0; j < 6; j++){
        uint8_t d = datagram[j];
        printf("%02x ", d);
        buffer[j] = isprint(d) == 0 ? '.' : d;
    }
    printf(" | %s\n", buffer);
    printf("Salvataggio su file in corso... ");
    FILE *fp;
    if (fp == NULL) return 1;
    fp = fopen("datagram.ed2p", "w");
    fwrite(datagram, 1, sizeof(datagram), fp);
    fclose(fp);
    printf("[OK]\n");

    return 0;
}
