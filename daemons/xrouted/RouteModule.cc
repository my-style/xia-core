#include "RouteModule.hh"
#include <syslog.h>


RouteModule::RouteModule(const char *name)
{
	_hostname       = name;
	_broadcast_sock = -1;
	_local_sock     = -1;
	_source_sock    = -1;
	_recv_sock      = -1;
}


pthread_t RouteModule::start()
{
	int rc;

	// connect to the click route engine
	_xr.setRouter(_hostname);
	if ((rc = _xr.connect()) != 0) {
		syslog(LOG_ALERT, "unable to connect to click (%d)", rc);
		exit(-1);
	}

	makeLocalSocket();

	init();
	_enabled = true;

	rc = pthread_create(&_t, NULL, run, (void*)this);
	if (rc == 0) {
		return _t;
	} else {
		return 0;
	}
}

int RouteModule::makeSocket(Graph &g, sockaddr_x *sa)
{
	int sock = Xsocket(AF_XIA, SOCK_DGRAM, 0);

	if (sock < 0) {
		syslog(LOG_ALERT, "Unable to create socket: %s", strerror(errno));
		return -1;
	}

	g.fill_sockaddr(sa);

	if (Xbind(sock, (struct sockaddr*)sa, sizeof(sockaddr_x)) < 0) {
		syslog(LOG_ALERT, "unable to bind to DAG : %s", g.dag_string().c_str());
		Xclose(sock);
		sock = -1;
	}

	return sock;
}

int RouteModule::getXIDs(std::string &ad, std::string &hid)
{
	char s[MAX_DAG_SIZE];

	int sock = Xsocket(AF_XIA, SOCK_DGRAM, 0);

	if (sock < 0) {
		syslog(LOG_ALERT, "Unable to read local XIA address");
		return -1;
	}

	// get our AD and HID
	if (XreadLocalHostAddr(sock, s, MAX_DAG_SIZE, NULL, 0) < 0 ) {
		syslog(LOG_ALERT, "Unable to read local XIA address");
		Xclose(sock);
		return -1;
	}

	Xclose(sock);

	Graph g(s);
	ad  = g.intent_AD_str();
	hid = g.intent_HID_str();

	return 0;
}


int RouteModule::makeLocalSocket()
{
	struct sockaddr_in sin;

	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	sin.sin_port = htons(LOCAL_PORT);
	sin.sin_family = AF_INET;

	_local_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (_local_sock < 0) {
		syslog(LOG_ALERT, "Unable to create the local control socket");
		return -1;
	}
	if (bind(_local_sock, (struct sockaddr*)&sin, sizeof(struct sockaddr)) < 0) {
		syslog(LOG_ALERT, "unable to bind to localhost:%u", LOCAL_PORT);
		return -1;
	}

	return 0;
}


int RouteModule::readMessage(char *recv_message, struct pollfd *pfd, unsigned npfds, int *iface)
{
	int rc = -1;
	sockaddr_x theirDAG;
	struct timespec tspec;

	tspec.tv_sec = 0;
	tspec.tv_nsec = 500000000;

	rc = Xppoll(pfd, npfds, &tspec, NULL);
	if (rc > 0) {
		int sock = -1;

		for (int i = 0; i < 3; i++) {
			if (pfd[i].revents & POLLIN) {
				sock = pfd[i].fd;
				break;
			}
		}

		memset(&recv_message[0], 0, BUFFER_SIZE);

		if (sock <= 0) {
			// something weird happened
			return 0;

		} else if (sock == _local_sock) {
			*iface = FALLBACK;
			if ((rc = recvfrom(sock, recv_message, BUFFER_SIZE, 0, NULL, NULL)) < 0) {
				syslog(LOG_WARNING, "local message receive error");
			}
		} else {
			struct msghdr mh;
			struct iovec iov;
			struct in_pktinfo pi;
			struct cmsghdr *cmsg;
			struct in_pktinfo *pinfo;
			char cbuf[CMSG_SPACE(sizeof pi)];

			iov.iov_base = recv_message;
			iov.iov_len = BUFFER_SIZE;

			mh.msg_name = &theirDAG;
			mh.msg_namelen = sizeof(theirDAG);
			mh.msg_iov = &iov;
			mh.msg_iovlen = 1;
			mh.msg_control = cbuf;
			mh.msg_controllen = sizeof(cbuf);

			cmsg = CMSG_FIRSTHDR(&mh);
			cmsg->cmsg_level = IPPROTO_IP;
			cmsg->cmsg_type = IP_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(pi));

			mh.msg_controllen = cmsg->cmsg_len;

			if ((rc = Xrecvmsg(sock, &mh, 0)) < 0) {
				perror("recvfrom");

			} else {
				for (cmsg = CMSG_FIRSTHDR(&mh); cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg)) {
					if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
						pinfo = (struct in_pktinfo*) CMSG_DATA(cmsg);
						*iface = pinfo->ipi_ifindex;
					}
				}
			}
		}
	}
	return rc;
}


int RouteModule::sendMessage(sockaddr_x *dest, const Xroute::XrouteMsg &msg)
{
	int rc;
	string message;
	msg.SerializeToString(&message);

	rc = Xsendto(_source_sock, message.c_str(), message.length(), 0, (sockaddr*)dest, sizeof(sockaddr_x));
	if (rc < 0) {
		syslog(LOG_WARNING, "unable to send %s msg: %s", Xroute::msg_type_Name(msg.type()).c_str(), strerror(errno));
	}
	return rc;
}

void *RouteModule::run(void *inst)
{
	RouteModule *re = (RouteModule*)inst;
	while (re->_enabled) {
		re->handler();
	}
	return NULL;
}

int RouteModule::wait()
{
	return pthread_join(_t, NULL);
}
