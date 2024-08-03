#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
/* Function Definitions */

// 2nd
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Now i need to extract substring from larger string i.e [.bmp from beautiful.bmp]
    // So strstr is used here to extract the same.
    if (strcmp(strstr(argv[2], ".bmp"), ".bmp") == 0)
    {
        printf("INFO : BMP file Validated Successfully\n");
        // To store that extracted file name into the structure.
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // Now i need to extract substring from larger string i.e   .txt from secret.txt
    // So strstr is used here to extract the same.
    if (strcmp(strstr(argv[3], ".txt"), ".txt") == 0)
    {
        printf("INFO : Secret file Validated Successfully\n");
        encInfo->secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    // This is optional (So we are checking whether the 4th argument is passed or not) --> To know that check the 4th argument is not empty(NULL)
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "default.bmp";
    }

    // If all these 3 if conditions are true returning e_success
    return e_success;
}

// 12th (Getting size of secret file)
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}

// Function to get the image size of both ( BMP file size and secret file size)
// 8th
Status check_capacity(EncodeInfo *encInfo)
{
    // You need to encode data from RGB data only and not from header, so we should know the number of pixels , by using these number of pixels you can get number of bytes...
    // the RGB Data is available.
    // Here get_image_size_for_bmp is going to return the size of RGB data so you need to collect it somewhere..
    // Collecting that size in image_capacity(storing to structure).

    // 9th ( get_image_size_for_bmp-------------function see line 279 for definition)
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); // Get bmp file size

    // 11th ( For getting the size of secret file......see line 51).. after getting size you are going to store back into the structure only.
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret); // Get secret file size

    // here For encoding total bytes required is 312
    // So, the image capacity should be greater than 312 bytes, then only you can encode data.
    if (encInfo->image_capacity > (16 + 32 + 32 + 32 + encInfo->size_secret_file * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
// 14th
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(str, 54, 1, fptr_src_image);   // Reading 54 bytes from beautiful.bmp and storing it to str
    fwrite(str, 54, 1, fptr_dest_image); // Writing the same 54 bytes stored in str to stego.bmp
    return e_success;
}
// 18th (this function is called in line 106 inside(encode_data_to_image)).
Status encode_byte_to_lsb(char data, char *image_buffer) // image_buffer is having 8 byte data
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> i) & 1);
    }
}
// 17th
Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo) // .txt ,4,struct
{
    for (int i = 0; i < size; i++)
    { // reading 8 bytes from source image and storing it into a new array(image_data)
        fread(encInfo->image_data, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(data[i], encInfo->image_data);             // at first data[i] -->data[0] = #   second time data[1] = *
        fwrite(encInfo->image_data, 8, 1, encInfo->fptr_stego_image); // writing 8 bytes of data read from line 105 to fptr_stego_image
    }
}
// 16th (Refer project notes page 4(Right part) to understand in detail)
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string, strlen(magic_string), encInfo);
    return e_success;
}
// 22nd
Status encode_size_to_lsb(int size, EncodeInfo *encInfo)
{
    char str[32]; // .txt has size as 4(decimal) 8 byte was enough but here for safer side we are 32 bytes (size of (int)= 4 * 8 = 32) larger bytes..
    fread(str, 32, 1, encInfo->fptr_src_image);
    for (int i = 0; i < 32; i++) // to perform encoding (i.e copy bits to lsb)
    {
        str[i] = (str[i] & 0xFE) | ((size >> i) & 1);
    }

    fwrite(str, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}
// 20th (integer 4(.txt) which is 8 byte  so for safer side 32 byte(size of int) see above definition)
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{ // 21st
    encode_size_to_lsb(size, encInfo);
    return e_success;
}
// 24th
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{   // 25th (jumps to line 101)
    encode_data_to_image(file_extn, strlen(file_extn), encInfo); // file_exten ---> .txt
    return e_success;
}
// 27th
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{ // 28th jumps to definition line 117
    encode_size_to_lsb(file_size, encInfo);
    return e_success;
}
// 30th
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo->size_secret_file];
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    fread(str, encInfo->size_secret_file, 1, encInfo->fptr_secret); // reading data from secret.txt  ( My password is SECRET ;) ) and storing it to str array
    // 31st (jumps to line 101)
    encode_data_to_image(str, encInfo->size_secret_file, encInfo);
    return e_success;
}
// 33rd
Status copy_remaining_img_data(EncodeInfo *encInfo)
{ // To find the left over bytes from beautiful.bmp i.e subtracting the stego image size from the original image size gives left over bytes.
    int len = (encInfo->image_capacity) /* RGB DATA */ + 54 /* Header DATA */ - ftell(encInfo->fptr_stego_image);
    char str[len];
    fread(str, len, 1, encInfo->fptr_src_image);    // Reading this left over data from beautiful.bmp and storing it to str array
    fwrite(str, len, 1, encInfo->fptr_stego_image); // Copying the data read above which is stored in str to the stego image.
    return e_success;
}

// 4th
Status do_encoding(EncodeInfo *encInfo)
{
    // 5th.......see line 309 for definition
    if (open_files(encInfo) == e_success) // Since encInfo is already holding an address, here passing only as encInfo (not as &encInfo)
    {
        printf("INFO : Open File is Success\n");
        // 7th (See line 59 for definition)
        if (check_capacity(encInfo) == e_success)
        {
            printf("INFO : Check capacity is Success\n");

            // 13th (See line 84 for definition)
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("INFO : BMP header copied successfully\n");
                // 15th (See line 111 for definition)
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("INFO : Encoded Magic string successfully\n");

                    // Here extracting the previously stored extension (.txt) from secret_fname  and storing it to a new array named (extn_secret_file) using strcpy
                    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, ".")); // .txt
                    // printf("%s\n", encInfo->extn_secret_file);

                    // 19 th(see line 130 for def) (here in the below function extracting the length of extension of secret file which got stored previously line 188 by using strcpy)
                    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
                    {
                        printf("INFO : Encoded extension size successfully\n");
                        // 23rd ( line 136 for definition)
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("INFO : Encoded secret file extension successfully\n");

                            // 26th ( see line 142 for definition)
                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("INFO : Encoded secret file size successfully\n");
                                // 29th (line 148 for definition)
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("INFO : Encoded secret file data successfully\n");
                                    // 32nd (line 158 for definition)
                                    if (copy_remaining_img_data(encInfo) == e_success)
                                    {
                                        printf("INFO : Remaining image data copied successfully\n");
                                    }
                                    else
                                    {
                                        printf("INFO : Failed to copy image data\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("INFO : Failed to encode secret file data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("INFO : Failed to encode secret file size\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("INFO : Failed to encode secret file extension\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("INFO : Encoding extension size failed\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("INFO : Encoding Magic string is not successfull\n");
                    return e_failure;
                }
            }
            else
            {
                printf("INFO : BMP header is not copied successfully\n");
                return e_failure;
            }
        }

        else
        {
            printf("INFO : Check capacity is Failure\n");
            return e_failure;
        }
    }
    else
    {
        printf("INFO : Open file is Failure\n");
        return e_failure;
    }
    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

// 10th
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    // Seek to 18th byte, because the width is starting from 18th byte.
    // Here file pointer(fptr_image) will be moved to 18th byte.
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int) .......read 4 bytes from 18 i.e 19,20,21,22(19-22   where the width is present).
    // Here we are using fread and not fscanf because it is a binary file(Non-ASCII) and it will not be in understandable manner so using fread.
    fread(&width, sizeof(int) /*4 bytes will be read*/, 1, fptr_image);
    printf("INFO : Width = %u\n", width);

    // Read the height (an int)........read 4 bytes from 22 i.e 23,24,25,26(23-26  where the height is present).
    fread(&height, sizeof(int), 1, fptr_image);
    printf("INFO : Height = %u\n", height);

    // Return image capacity
    return width * height * 3; // (here no.ofpixels * 3bytes(RGB))
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

// 6th
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");

    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");

    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");

    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}
