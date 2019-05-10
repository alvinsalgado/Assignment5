#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TLB_SIZE 16
#define PAGES 256
#define PAGE_MASK 255
#define PAGE_SIZE 256
#define OFFSET_BITS 8
#define OFFSET_MASK 255
#define MEMORY_SIZE PAGES * PAGE_SIZE
#define BUFFER_SIZE 10

struct tlbentry {
unsigned char logical;
unsigned char physical;
};

struct tlbentry tlb[TLB_SIZE];
int tlbindex = 0;

// Number of physical page for logical page
int pagetable[PAGES];

signed char main_memory[MEMORY_SIZE];
signed char *backing;

int max(int a, int b)
{
    if (a > b)
    return a;
    return b;
}

// Searches for physical address. If found return physical address else return -1.
int search_tlb(unsigned char logical_page) {
    int i;
    for(i = max(tlbindex - TLB_SIZE,0); i < tlbindex; i ++)
    {
        struct tlbentry * entry = &tlb[i% TLB_SIZE];

        if(entry->logical == logical_page)
        {
            return entry->physical;
        }
    }
    return -1;
}

// Add new entries to the TLB
void add_to_tlb(unsigned char logical, unsigned char physical) {
    struct tlbentry * entry = &tlb[tlbindex % TLB_SIZE];
    tlbindex++;
    entry->logical = logical;
    entry->physical = physical;
}

int main(int argc, const char *argv[])
{
    // Check if user enters 3 argument else exit.
    if (argc != 3) {
        fprintf(stderr, "Please enter 3 args: ./virtualmem backingstore input\n");
        exit(0);
    }
    // Set all page table entries with -1.
    for (int i = 0; i < PAGES; i++) {
        pagetable[i] = -1;
    }
    // The file is the bin file, BACKING_STORE.bin
    const char *backing_filename = argv[1];
    // The file is the input file, addresses.txt
    const char *input_filename = argv[2];
    // Open file and return file descriptor for the bin file
    int backing_fd = open(backing_filename, O_RDONLY);
    backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0);
    FILE *input_fp = fopen(input_filename, "r");

    // Use a character buffer for reading lines of input file.
    char buffer[BUFFER_SIZE];

    // Used to know the total number of compute stats collected
    int total_addresses = 0;
    int tlb_hits = 0;
    int page_faults = 0;

    // Used to know the number of the next unallocated physical page in main memory
    unsigned char free_page = 0;

    while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL) {
        int logical_address = atoi(buffer);
        int offset = logical_address & OFFSET_MASK;
        int logical_page = (logical_address >> OFFSET_BITS) & PAGE_MASK;
        int physical_page = search_tlb(logical_page);
        total_addresses++;
        // An if statement dealing with TLB HIT
        if(physical_page != -1)
        {
            tlb_hits++;
        }else
        {
            physical_page = pagetable[logical_page];
            // An if statement dealing with page fault
            if(physical_page = -1)
            {
                page_faults++;
                physical_page = free_page;
                free_page++;

                memcpy(main_memory + physical_page * PAGE_SIZE, backing + logical_page * PAGE_SIZE, PAGE_SIZE);

                pagetable[logical_page] = physical_page;

            }

        }
        add_to_tlb(logical_page,physical_page);
        int physical_address = (physical_page << OFFSET_BITS) | offset;

        signed char value = main_memory[physical_page * PAGE_SIZE + offset];

        printf("Virtual Address: %d Physical Address: %d Value: %d\n", logical_address, physical_address, value);


}

printf("Number of Addresses = %d\n", total_addresses);
printf("Page Faults = %d\n", page_faults);
printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
printf("TLB Hits = %d\n", tlb_hits);
printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));

return 0;
}
