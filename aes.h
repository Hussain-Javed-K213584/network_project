#ifndef AES
#define AES

void handleErrors(void);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char* key, unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char* key, unsigned char *iv, unsigned char* plaintext);
void rand_str(char* dest, int length);
#endif 