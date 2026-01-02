#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define FILE_NAME "/tmp/dummy_data.bin"

#define NUM_ITER 8
#define MAX_ITER 64
#define FILE_SIZE_MB 32
#define BLOCK_SIZE 4096 // 4KB blocks
#define TOTAL_BLOCKS ((FILE_SIZE_MB * 1024 * 1024) / BLOCK_SIZE)

void compute_fletcher64_block(uint32_t blk[], size_t cnt, uint32_t *ps1, uint32_t *ps2)
{
    uint32_t s1 = 0;
    uint32_t s2 = 0;

    size_t words = cnt >> 4;
    for (size_t i = 0; i < words; i++)
    {
        s1 += blk[i];
        s2 += s1;
    }

    *ps1 += s1;
    *ps2 += s2;
}

uint64_t compute_fletcher64_file(const char *filename)
{
    uint32_t s1 = 0;
    uint32_t s2 = 0;
    uint32_t buffer[BLOCK_SIZE / 4];
    size_t b_read;

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;

    while ((b_read = fread(buffer, 1, BLOCK_SIZE, fp)) > 0)
    {
        compute_fletcher64_block(buffer, b_read, &s1, &s2);
    }

    fclose(fp);
    return ((uint64_t)s2 << 32) | s1;
}

void generate_random_block(uint8_t blk[], size_t blk_size)
{
    for (int j = 0; j < blk_size; j++)
    {
        blk[j] = rand() % 256;
    }
}

int create_dummy_file(const char *filename)
{
    uint8_t *block = NULL;
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("Error opening file ");
        return 1;
    }

    printf("Writing %d MB of random data...\n", FILE_SIZE_MB);
    block = malloc(BLOCK_SIZE);

    for (int i = 0; i < TOTAL_BLOCKS; i++)
    {
        generate_random_block(block, BLOCK_SIZE);
        fwrite(block, 1, BLOCK_SIZE, fp);
    }

    free(block);
    fclose(fp);
    printf("Write complete.\n");
    return 0;
}

size_t compute_checksum()
{
    uint8_t *block = NULL;
    uint64_t checksum = 0;
    size_t block_size = sizeof(uint32_t) * 1024 * 1024; 
    block = malloc(block_size);
    if (block == NULL)
        return 0;

    generate_random_block(block, block_size);
    for (size_t i = 0; i < block_size; i++)
    {
        checksum ^= (block[i] + (checksum << 1));
        checksum = (checksum << 7) | (checksum >> (64 - 7));
    }
    return checksum;
}

int complex_test_case(char *filename, size_t num_iter)
{
    uint64_t checksum = 0;

    printf("##################################\n");
    for (size_t i = 0; i < num_iter; i++)
    {
        printf("     Starting iteration %lu\n", i);
        if (create_dummy_file(filename))
            return 1;

        checksum = compute_fletcher64_file(filename);
        printf("Checksum: 0x%016lx\n", checksum);

        if (remove(filename) != 0)
            perror("Error deleting file");

        printf("##################################\n");
    }

    return 0;
}

void simple_test_case(size_t num_sec)
{
    uint64_t checksum = 0;
    uint64_t cumulative = 0;

    time_t start = time(NULL);

    while (difftime(time(NULL), start) <= num_sec)
    {
        checksum = compute_checksum();
        cumulative = cumulative^checksum;
    }
}

int main(int argc, char *argv[])
{
    size_t num_iter = NUM_ITER;
    char *filename = FILE_NAME;

    if (argc > 1 && argv[1] != NULL)
        num_iter = atoi(argv[1]);

    if (num_iter == 0)
        return 0;

    if (argc > 2 && argv[2] != NULL)
    {
        return complex_test_case(argv[2], num_iter);
    }

    simple_test_case(num_iter);
    return 0;
}