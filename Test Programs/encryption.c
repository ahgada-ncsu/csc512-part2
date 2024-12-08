#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME 256
#define BUFFER_SIZE 1024

unsigned char simple_xor_cipher(unsigned char byte, unsigned char key) {
    return byte ^ key;
}

int encrypt_decrypt_file(const char *input_file, const char *output_file, unsigned char key) {
    FILE *in_file, *out_file;
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;

    in_file = fopen(input_file, "rb");
    if (in_file == NULL) {
        printf("Error opening input file.\n");
        return 0;
    }

    out_file = fopen(output_file, "wb");
    if (out_file == NULL) {
        printf("Error opening output file.\n");
        fclose(in_file);
        return 0;
    }

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, in_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            buffer[i] = simple_xor_cipher(buffer[i], key);
        }
        fwrite(buffer, 1, bytes_read, out_file);
    }

    fclose(in_file);
    fclose(out_file);
    return 1;
}

int main(int argc, char *argv[]) {
    char input_filename[MAX_FILENAME];
    char output_filename[MAX_FILENAME];
    unsigned char encryption_key;

    printf("Enter input filename: ");
    fgets(input_filename, MAX_FILENAME, stdin);
    input_filename[strcspn(input_filename, "\n")] = 0;

    printf("Enter output filename: ");
    fgets(output_filename, MAX_FILENAME, stdin);
    output_filename[strcspn(output_filename, "\n")] = 0;

    printf("Enter encryption/decryption key (0-255): ");
    scanf("%hhu", &encryption_key);

    if (encrypt_decrypt_file(input_filename, output_filename, encryption_key)) {
        printf("File processed successfully.\n");
    } else {
        printf("File processing failed.\n");
    }

    return 0;
}