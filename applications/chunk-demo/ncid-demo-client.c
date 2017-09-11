#include "ncid-demo-client.h"

NCIDDemoClient::NCIDDemoClient()
{
	std::cout << "Starting NCIDDemoClient" << std::endl;
	std::cout << "Opening a socket to publish data" << std::endl;

	if((_sockfd = Xsocket(AF_XIA, SOCK_STREAM, 0)) < 0) {
		throw "Unable to create socket for client";
	}

	// Get a handle to Xcache
	if(XcacheHandleInit(&_xcache)) {
		throw "Unable to talk to xcache";
	}

	std::cout << "Started NCIDDemoClient" << std::endl;
}

NCIDDemoClient::~NCIDDemoClient()
{
	// Cleanup state before exiting
	if (_sockfd >= 0) {
		Xclose(_sockfd);
	}
}

int NCIDDemoClient::fetch()
{
	// For now, just create a chunk of data
	std::string data("Some Random Data");

	// Store it as a named chunk
	int ret;
	char **buf;
	int flags = XCF_BLOCK;
	*buf = NULL;
	std::string name = _publisher + "/" + _content_name;
	ret = XfetchNamedChunk(&_xcache, (void **)buf, flags, name.c_str());
	if (ret < 0) {
		std::cout << "Failed publishing named chunk" << std::endl;
	}
	if(ret > 0) {
		std::string chunk(*buf, (size_t)ret);
		std::cout << "Chunk contains:" << chunk << std::endl;
	}

	if(*buf) {
		free(*buf);
	}
	return ret;
}

int main()
{
	NCIDDemoClient *client = new NCIDDemoClient();
	int retval = client->fetch();
	delete client;
	return retval;
}
