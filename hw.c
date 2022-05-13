#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAXARGS 80
unsigned char heap[64];

int blockIsFree(unsigned char index) {
    return (heap[index] & 1) == 1 ? 0 : 1;
}

int getSize(int index) {
    return heap[index] >> 1;
}

void freeBlock(int index) {
    unsigned char* p = heap + index - 1;
    unsigned char* k = heap + (index - 1) + (heap[index - 1] >> 1) - 2 + 1;
    unsigned char block_size = (*p) >> 1;
    *p = (*p) & 0xFE;
    *k = (*k) & 0xFE;
}

void block_coalescing(int current_header, int current_footer) {
    int next_header = current_footer + 1;
    if (current_footer < 63 && blockIsFree(next_header)) {
        heap[current_header] = (getSize(next_header) + getSize(current_header)) << 1;
        int new_footer = current_header;
        new_footer++;

        int i;
        for (i = 0; i < getSize(current_header) - 2; i++) {
            heap[new_footer] = 0;
            new_footer++;
        }

        heap[new_footer] = heap[current_header];
        current_footer = new_footer;

    }

    int prev_footer = current_header - 1;
    if (current_header > 0 && blockIsFree(prev_footer)) {
        heap[current_footer] = (getSize(prev_footer) + getSize(current_footer)) << 1;
        int new_header = current_footer; 
        new_header--;

        int x;
        for (x = 0; x < getSize(current_footer) - 2; x++) {
            heap[new_header] = 0;
            new_header--;
        }

        heap[new_header] = heap[current_footer];
    }
}

void printMem(int index, int num_of_chars) {
    printf("\n");
    unsigned char* p = heap + index;

    int i = 0;
    while (i < num_of_chars) {
        printf("%x ", p[i]);
        i += 1;
    }
}

void writeMem(int index, char* str) {
    unsigned char* p = heap + index;

    int i = 0;
    while (i < strlen(str)) {
        p[i] = str[i];
        i += 1;
    }
}

void blocklist() {
    printf("\n");

    int i = 0;
    while (i < 64) {
        unsigned char payload_size = heap[i] >> 1;
        unsigned char status = heap[i] & 1;

        if (status) {
            printf("%d, %d, allocated\n", i + 1, payload_size - 2);
        } else {
            printf("%d, %d, free\n", i + 1, payload_size - 2);
        }

        i += payload_size;
    }
}

void allocateBlock(int sz){
    int i = 0; // pointer to beginning of heap
    int found = 0; // a flag
    int taken = 0;

    //printf("%i\n", heap[0]&1); for checking if allocated or no
    while (!found && i < 64) {
        //printf("%i\n", i); printing index
        taken = (heap[i] & 1);

        if (!taken && ((heap[i] >> 1) - 2) >= sz) {
            // block is free and fits the requested size
            found = 1; 
            break;
        }
        //printf("%i with index\n", heap[i]>>1, i); checking for size at current index
        i += (heap[i] >> 1); // go to header of next block
    }
    
    if (!found) {
        return;
    }

    printf("\n%i\n", i + 1);

    if (((heap[i] >> 1) - sz - 2) <= 2) {
        heap[i] += 1; //set allocated bit on header
        i += (heap[i] >> 1) -1; //move to the footer
        heap[i] += 1; // set allocated bit on footer
    } else {
        int oldsize = (heap[i] >> 1); //get the old size of the block
        heap[i] = ((sz + 2) << 1) + 1; // set current block header to right size and allocated
        i+= (heap[i] >> 1) -1;
        heap[i] = ((sz + 2) << 1) + 1;
        i += 1;
        heap[i] = (oldsize - (sz + 2)) << 1;
        i += (heap[i] >> 1) - 1;
        heap[i] = (oldsize - (sz + 2)) << 1;
    }
}

void tokenize(char* cmdline, char* args[MAXARGS]) {
    int i = 0;
    *args = strtok(cmdline, " \t\n");
    
    while (1) {
        if (args[i] == NULL) {
            break;
        }

        args[++i] = strtok(NULL, " \t\n");
    }
}

void execute(char* args[MAXARGS]) {
    if (!strcmp(args[0], "malloc")) {
        allocateBlock(atoi(args[1]));
    } else if (!strcmp(args[0], "free")) {
        freeBlock(atoi(args[1]));
        block_coalescing(atoi(args[1]) - 1, (atoi(args[1]) - 1) + (heap[(atoi(args[1]) - 1)] >> 1) - 2 + 1);
    } else if (!strcmp(args[0], "blocklist")) {
        blocklist();
    } else if (!strcmp(args[0], "writemem")) {
        writeMem(atoi(args[1]), args[2]);
    } else if (!strcmp(args[0], "printmem")) {
        printMem(atoi(args[1]), atoi(args[2]));
    }
}

int main() {
    char cmdline[MAXARGS];
    char* args[MAXARGS];

    heap[0] = (64 << 1);
    heap[63] = (64 << 1);

    int i;
    for (i = 1; i < 63;i++) {
        heap[i] = 0;
    }

    do {
        printf("\n> ");
        fgets(cmdline, MAXARGS, stdin);
        tokenize(cmdline, args);

        if (strcmp(args[0], "quit") == 0) {
            exit(0);
        }
            
        execute(args);
    } while (1);

    return 0;
}