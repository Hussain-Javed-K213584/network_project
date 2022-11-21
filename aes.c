#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "aes.h"

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char* key, unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cfb128(), NULL, key, iv))
        handleErrors();

    
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char* key, unsigned char *iv, unsigned char* plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len, plaintext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())) //initialize
        handleErrors();

    
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cfb128(), NULL, key, iv)) //init decryption
        handleErrors();

    
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) //provide ciphertext and recieve plaintext
        handleErrors();
    plaintext_len = len;

    //Finalize decrpytion. Final size can be greater because of padding
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
void rand_str(char* dest, int length)
{
    srand(time(0));
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}