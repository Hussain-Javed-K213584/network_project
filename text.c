#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include "aes.h"
#include "proto.h"
int main(void)
{
    char key[32];
    char iv[32];
    //initiliaze key and iv
    rand_str((unsigned char*)key, 33-1);
    rand_str((unsigned char*)iv, 13-1);
    char plaintext[101], ciphertext[101];
    printf("Input the plaintext: ");
    memset(plaintext, 0, sizeof(plaintext));
    fgets(plaintext, 101, stdin);
    memset(ciphertext, 0, sizeof(ciphertext));
    int ciphertext_len = encrypt((char*)plaintext, 101, (char*)key, (char*)iv, (char*)ciphertext);
    printf("Ciphertext is: ");
    BIO_dump_fp(stdout, (const char*)ciphertext, ciphertext_len);
    char clearText[101];
    memset((char*)clearText, 0, sizeof(clearText));
    int clearText_len = decrypt((char*)ciphertext, ciphertext_len, (char*)key, (char*)iv, (char*)clearText);
    
    printf("The decrpyted text is: %s\n", clearText);
}