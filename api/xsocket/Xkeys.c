// COMPILE: gcc -o genkeypair genkeypair.c -lcrypto -lssl
#include "Xsocket.h"
#include "Xinit.h"
#include "Xutil.h"
#include "Xkeys.h"

#define KEY_BITS 1024
#define MAX_KEYDIR_PATH_LEN 1024

// Convert a SHA1 hash to a hex string
static void sha1_hash_to_hex_string(unsigned char *digest, int digest_len, char *hex_string, int hex_string_len)
{
    int i;
	assert(digest_len == SHA_DIGEST_LENGTH);
	assert(hex_string_len == (2*SHA_DIGEST_LENGTH) + 1);
    for(i=0;i<digest_len;i++) {
        sprintf(&hex_string[2*i], "%02x", (unsigned int)digest[i]);
    }
    hex_string[hex_string_len-1] = '\0';
}

// Calculate SHA1 hash of a public key in PEM format
static int sha1_hash_of_pubkey(unsigned char *hash, int hashlen, char *pemkey, int keylen)
{
	int i, j;
	char *pubkeystr;
	int keystartoffset = strlen("-----BEGIN PUBLIC KEY-----\n");
	int keyendoffset = strlen("-----END PUBLIC KEY-----\n");

	assert(hash != NULL);
	assert(hashlen == SHA_DIGEST_LENGTH);

	pubkeystr = (char *)calloc(keylen - keystartoffset - keyendoffset, 1);
	if(pubkeystr == NULL) {
		return -1;
	}
	for(i=keystartoffset,j=0; i<keylen-keyendoffset-1; i++) {
		if(pemkey[i] != '\n') {
			pubkeystr[j++] = pemkey[i];
		}
	}
	//printf("Stripped Pubkey:%s:\n", pubkeystr);
	SHA1((const unsigned char *)pubkeystr, strlen(pubkeystr), hash);
	free(pubkeystr);
	return 0;
}

// Write keys in PEM format to filesystem
static int write_key_files(const char *keydir, char *pubkeyhashstr, RSA *r)
{
	int state = 0;
	int retval = -1;
	BIO *pubfilebio = NULL;
	BIO *privfilebio = NULL;
	char *pubfilepath = NULL;
	char *privfilepath = NULL;
	int filepathlen = strlen(keydir) + strlen("/") + strlen(pubkeyhashstr) + 1;

	privfilepath = (char *)calloc(filepathlen, 1);
	if(privfilepath == NULL) {
		goto cleanup_write_key_files;
	}
	state = 1;

	pubfilepath = (char *)calloc(filepathlen + strlen(".pub"), 1);
	if(pubfilepath == NULL) {
		goto cleanup_write_key_files;
	}
	state = 2;

	sprintf(privfilepath, "%s/%s", keydir, pubkeyhashstr);
	sprintf(pubfilepath, "%s.pub", privfilepath);
	pubfilebio = BIO_new_file(pubfilepath, "w+");
	if(pubfilebio == NULL) {
		goto cleanup_write_key_files;
	}
	state = 3;
	
	if(PEM_write_bio_RSA_PUBKEY(pubfilebio, r) != 1) {
		goto cleanup_write_key_files;
	}
	privfilebio = BIO_new_file(privfilepath, "w+");
	if(privfilebio == NULL) {
		goto cleanup_write_key_files;
	}
	state = 4;

	if(PEM_write_bio_RSAPrivateKey(privfilebio, r, NULL, NULL, 0, NULL, NULL) != 1) {
		goto cleanup_write_key_files;
	}
	retval = 0;
cleanup_write_key_files:
	switch(state) {
		case 4: BIO_free_all(privfilebio);
		case 3: BIO_free_all(pubfilebio);
		case 2: free(pubfilepath);
		case 1: free(privfilepath);
	}
	return retval;
}

static int file_exists(char *filepath)
{
	struct stat fileinfo;
	if(stat(filepath, &fileinfo)) {
		return 0;
	}
	if(!S_ISREG(fileinfo.st_mode)) {
		return 0;
	}
	return 1;
}

static int dir_exists(const char *keydir)
{
	struct stat keydirinfo;

	// Verify that keydir exists
	if(stat(keydir, &keydirinfo)) {
		return 0;
	}
	//if(!keydirinfo.st_mode & S_IFDIR) {
	if(!S_ISDIR(keydirinfo.st_mode)) {
		return 0;
	}
	return 1;
}

int generate_keypair(const char *keydir, char *pubkeyhashstr, int hashstrlen)
{
	int retval = -1;
	int state = 0;
    int keylen;
    RSA *r = NULL;
    BIGNUM *bne = NULL;
	BIO *pubkeybuf = NULL;
	char *pubkeystr = NULL;
	unsigned char pubkeyhash[SHA_DIGEST_LENGTH];
    unsigned long e = RSA_F4;

	// Check that the directory provided by user is valid
	if(!dir_exists(keydir)) {
		goto cleanup_generate_keypair;
	}

    // Create BIGNUM argument for key generation
    bne = BN_new();
	if(bne == NULL) {
		goto cleanup_generate_keypair;
	}
	state = 1;

    if(BN_set_word(bne, e) != 1) {
        goto cleanup_generate_keypair;
    }
 
	// Create a new RSA key pair
    r = RSA_new();
    if(RSA_generate_key_ex(r, KEY_BITS, bne, NULL) != 1) {
        goto cleanup_generate_keypair;
    }
	state = 2;
 
    // Derive filename from hash of public key and write keys to filesystem
	pubkeybuf = BIO_new(BIO_s_mem());
	if(pubkeybuf == NULL) {
		goto cleanup_generate_keypair;
	}
	state = 3;

	if(PEM_write_bio_RSA_PUBKEY(pubkeybuf, r) != 1) {
		goto cleanup_generate_keypair;
	}
	keylen = BIO_pending(pubkeybuf);
	pubkeystr = (char *)calloc(keylen+1, 1);
	if(pubkeystr == NULL) {
		goto cleanup_generate_keypair;
	}
	state = 4;

	if(BIO_read(pubkeybuf, pubkeystr, keylen) <= 0) {
		goto cleanup_generate_keypair;
	}
	//printf("Pubkey:%s:\n", pubkeystr);
	if(sha1_hash_of_pubkey(pubkeyhash, SHA_DIGEST_LENGTH, pubkeystr, keylen+1)) {
		goto cleanup_generate_keypair;
	}
	sha1_hash_to_hex_string(pubkeyhash, SHA_DIGEST_LENGTH, pubkeyhashstr, hashstrlen);
	//printf("Pubkeyhash:%s:\n", pubkeyhashstr);
	if(write_key_files(keydir, pubkeyhashstr, r)) {
		goto cleanup_generate_keypair;
	}
	// Successfully written files
	retval = 0;
 
cleanup_generate_keypair:
	switch(state) {
		case 4: free(pubkeystr);
		case 3: BIO_free_all(pubkeybuf);
    	case 2: RSA_free(r);
    	case 1: BN_free(bne);
	};
 
    return retval;
}

// Check that key files matching pubkeyhashstr exist in keydir
int exists_keypair(const char *keydir, const char *pubkeyhashstr)
{
	int retval = 0;
	char *privfilepath;
	char *pubfilepath;
	int privfilepathlen;
	int pubfilepathlen;
	if(!dir_exists(keydir)) {
		return retval;
	}
	privfilepathlen = strlen(keydir) + strlen("/") + strlen(pubkeyhashstr) + 1;
	pubfilepathlen = privfilepathlen + strlen(".pub");
	privfilepath = (char *)calloc(privfilepathlen, 1);
	pubfilepath = (char *)calloc(pubfilepathlen, 1);
	sprintf(privfilepath, "%s/%s", keydir, pubkeyhashstr);
	sprintf(pubfilepath, "%s.pub", privfilepath);
	if(file_exists(privfilepath) && file_exists(pubfilepath)) {
		retval = 1;
	}
	free(privfilepath);
	free(pubfilepath);
	return retval;
}

/*
int main(int argc, char* argv[]) 
{
	char pubkeyhexdigest[SHA_DIGEST_LENGTH*2+1];
    int retval = generate_keypair("key", pubkeyhexdigest, sizeof(pubkeyhexdigest));
	if(!retval) {
		printf("Generated key:%s:\n", pubkeyhexdigest);
	}
	return retval;
}
*/
