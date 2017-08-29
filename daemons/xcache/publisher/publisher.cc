#include <iostream>      // cout, endl
#include <assert.h>
#include <string.h>
#include <Xsocket.h>
#include <Xsecurity.h>
#include "publisher.h"
#include "publisher_cert.h"
#include <stdlib.h>
#include <stdio.h>

#include "../controller.h"
#include "xcache_cmd.pb.h"
#include <sys/types.h>    // stat
#include <sys/stat.h>     // stat
#include <unistd.h>       // stat

/*!
 * @brief An Object representing a Publisher named on instantiation
 */
Publisher::Publisher(std::string name)
{
	char keydir[MAX_KEYDIR_PATH_LEN];
	std::string publisher_dir = "/publisher/" + name;
	if(XrootDir((char *)keydir, MAX_KEYDIR_PATH_LEN) == NULL) {
		printf("ERROR: Unable to get XIA root directory name");
		assert(-1);
	}

	// Protect from buffer overflow attacks on strcat
	assert(strlen(keydir) + publisher_dir.size() < MAX_KEYDIR_PATH_LEN);
	_keydir = strcat(keydir, publisher_dir.c_str());

	_name = name;
}

Publisher::~Publisher()
{
	if(_keydir!= NULL) {
		free((void *)_keydir);
	}
}

/*!
 * @brief The publisher's name
 */
std::string Publisher::name()
{
	return _name;
}

/*!
 * @brief Build Content URI for the requested content
 */
std::string Publisher::content_URI(std::string content_name)
{
	return name() + "/" + content_name;
}

/*!
 * @brief Publisher's public key
 */
std::string Publisher::pubkey()
{
	char pubkeybuf[MAX_PUBKEY_SIZE];
	uint16_t pubkeylen = MAX_PUBKEY_SIZE;

	// Pubkey should be in a file in the credential directory
	std::string pubkeypath = keydir() + "/" + name() + ".pub";

	// If the pubkey is not available, try getting it
	if(!pubkey_present(pubkeypath)) {
		if(fetch_pubkey()) {
			return "";
		}
	}
	// Find the pubkey file in the Publisher's credential directory
	if(xs_readPubkeyFile(pubkeypath.c_str(), pubkeybuf, &pubkeylen)) {
		printf("Publisher::pubkey() cannot read key from %s\n",
				pubkeypath.c_str());
	}
	std::string pubkeystr(pubkeybuf, pubkeylen);
	return pubkeystr;
}

bool Publisher::keydir_present()
{
	struct stat statbuf;

	if(stat(_keydir, &statbuf)) {
		return false;
	}
	if(!S_ISDIR(statbuf.st_mode)) {
		std::cout << "ERROR: Creds directory invalid" << std::endl;
		return false;
	}
	return true;
}

/*!
 * @brief Create _keydir if not present
 *
 * @returns true on success or existing, false on failure
 */
bool Publisher::ensure_keydir_exists()
{
	if(!keydir_present()) {
		if(!mkdir(_keydir, S_IRWXU)) {
			std::cout << "ERROR creating creds directory" << std::endl;
			return false;
		}
	}
	return true;
}

/*!
 * @brief Check if the public key is in provided path
 *
 * For now, we just check to see if the path contains a file
 * TODO: make sure that a valid public key is available in file
 */
bool Publisher::pubkey_present(std::string path)
{
	struct stat statbuf;

	if(stat(path.c_str(), &statbuf)) {
		return false;
	}
	if(!S_ISREG(statbuf.st_mode)) {
		std::cout << "ERROR: pubkey path not a regular file" << std::endl;
		return false;
	}
	return true;
}

/*!
 * @brief Certificate name as registered with nameservice
 *
 * Currently we register <publisher_name>.publisher.cert.xia and the
 * certificate DAG with the nameservice. In future, there could be a
 * separate service or distributed source for finding certificate DAGs.
 */
std::string Publisher::cert_name()
{
	assert(_name.size() > 0);
	return _name + ".publisher.cert.xia";
}

/*!
 * @brief Fetch the public key to the provided path
 *
 * Query the nameserver to see if a certificate for this Publisher is
 * available for download. If one is available, we download the certificate
 * and extract the public key from it. Then we verify it against the
 * CA cert that we already have. If all checks out, store public key
 * for publisher and return success.
 */
bool Publisher::fetch_pubkey()
{
	sockaddr_x addr;
	socklen_t addrlen;
	int state = 0;
	bool retval = false;
	xcache_cmd resp;
	xcache_cmd cmd;
	size_t certlen;
	void *cert;
	std::string certpath;
	PublisherCert *pubcert;
	int flags = XCF_CACHE | XCF_BLOCK;

	// Get a reference to the controller
	xcache_controller *ctrl = xcache_controller::get_instance();

	// Build the Publisher cert name that nameserver should know
	std::string cert_url = cert_name();

	// Fetch Publisher cert DAG from nameserver
	if(XgetDAGbyName(cert_url.c_str(), &addr, &addrlen)) {
		std::cout << "Did not find address for " << cert_url << std::endl;
		goto fetch_pubkey_done;
	}

	// Fetch Publisher cert via XfetchChunk equivalent logic

	cmd.set_cmd(xcache_cmd::XCACHE_FETCHCHUNK);
	cmd.set_dag(&addr, addrlen);
	cmd.set_flags(flags);

	ctrl->xcache_fetch_content(&resp, &cmd, flags);
	certlen = resp.data().length();
	cert = malloc(resp.data().length());
	if(cert == NULL) {
		std::cout << "ERROR getting memory for cert" << std::endl;
		goto fetch_pubkey_done;
	}
	state = 1;

	memcpy(cert, resp.data().c_str(), certlen);

	// Write Publisher cert to disk
	if(store_publisher_cert(cert, certlen, certpath)) {
		std::cout << "Unable to store cert on disk" << std::endl;
		goto fetch_pubkey_done;
	}

	// Verify Publisher cert against CA cert
	pubcert = new PublisherCert(certpath);
	state = 2;

	if(!pubcert->is_valid()) {
		std::cout << "ERROR invalid publisher cert " << certpath << std::endl;
		goto fetch_pubkey_done;
	}

	// Extract Public key from Publisher cert
	if(!pubcert->extract_pubkey()) {
		std::cout << "ERROR extracting pubkey" << certpath << std::endl;
		goto fetch_pubkey_done;
	}
	retval = true;

	// Store valid Publisher cert and valid Pubkey in _keydir
	// TODO Currently storing before verification. Verify before store.
fetch_pubkey_done:
	switch(state) {
		case 2: delete pubcert;
		case 1: free(cert);
	};

	return retval;
}

/*!
 * @brief Write publisher certificate from memory to disk
 *
 * Provided certificate in cert is written to disk based on the
 * publisher's name. Correctness of certificate contents is not
 * checked at this time and must be ensured by caller.
 *
 * @returns true on success, false on failure
 * @returns path to certificate file if successful in storing
 */
bool Publisher::store_publisher_cert(void *cert, size_t len,
	std::string &certfilepath)
{
	FILE *certfile;
	int state = 0;
	bool retval = false;
	std::string certpath(_keydir);

	// Create directory to store Publisher credentials
	if(!ensure_keydir_exists()) {
		std::cout << "ERROR creds directory unavailable" << std::endl;
		goto store_publisher_cert_done;
	}

	// Now write certificate to disk
	certpath += "/" + _name + ".cert";
	certfile = fopen(certpath.c_str(), "w");
	state = 1;

	if(fwrite(cert, 1, len, certfile) != len) {
		std::cout << "ERROR writing certificate to disk" << std::endl;
		goto store_publisher_cert_done;
	}
	certfilepath = certpath;

	// Certificate successfully written to disk
	retval = true;

store_publisher_cert_done:
	switch(state) {
		case 1: if(fclose(certfile)) {
					std::cout << "Failed closing cert file" << std::endl;
				}
	}

	return retval;
}

/*!
 * @brief calculate NCID for the given content name
 *
 * NOTE: Returned string does not have "NCID:" prepended to it
 * @returns Hex string representing the NCID for given content name
 */
std::string Publisher::ncid(std::string content_name)
{
	std::string URI = content_URI(content_name);
	std::string pubkeystr = pubkey();
	std::string ncid_data = URI + pubkeystr;
	char ncidhex[XIA_SHA_DIGEST_STR_LEN];
	int ncidlen = XIA_SHA_DIGEST_STR_LEN;
	xs_getSHA1HexDigest((const unsigned char *)ncid_data.c_str(),
			ncid_data.size(), ncidhex, ncidlen);
	std::string ncidstr(ncidhex, ncidlen);
	return ncidstr;

}

/*!
 * @brief Directory containing all Publisher security credentials
 */
std::string Publisher::keydir()
{
	return _keydir;
}

std::string Publisher::privfilepath()
{
	std::string privkeypath = keydir() + "/" + name() + ".key";
	return privkeypath;
}

/*!
 * @brief Sign the given content to associate it with given content_URI
 */
int  Publisher::sign(std::string content_URI,
		const std::string &content,
		std::string &signature)
{
	int retval = -1;
	unsigned char sig_buf[MAX_SIGNATURE_SIZE];
	uint16_t siglen = MAX_SIGNATURE_SIZE;

	// Sign (Content URI + Content)
	std::string data = content_URI + content;
	// Private file path
	std::string privkeyfile = privfilepath();

	if(xs_signWithKey(privkeyfile.c_str(),
				(unsigned char *)data.c_str(), (int) data.size(),
				sig_buf, &siglen)) {
		printf("Publisher::sign() failed signing provided content\n");
		return -1;
	}
	std::string sigstr((char *)sig_buf, siglen);
	signature = sigstr;
	return retval;
}

