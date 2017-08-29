#ifndef _CID_HEADER_H
#define _CID_HEADER_H

#include "Xsecurity.h"
#include "content_header.h"
#include "chdr.pb.h"

class CIDHeader : public ContentHeader {
	public:
		CIDHeader(const std::string &data, time_t ttl);
		CIDHeader(const std::string &buf) {deserialize(buf);}
		virtual ~CIDHeader() {}

		virtual std::string id() { return _id; }
		virtual void set_id(std::string id) { _id = id; }

		virtual std::string store_id() { return _id; }
		virtual void set_store_id(std::string store_id) {
			_id=_store_id=store_id;
		}

		virtual size_t content_len() { return _len; }
		virtual void set_content_len(size_t len) { _len = len;}

		virtual time_t ttl() { return _ttl; }
		virtual void set_ttl(time_t ttl) { _ttl = ttl; }
		virtual std::string serialize();
		virtual bool deserialize(const std::string &buf);
	private:
		std::string _sha1hash(const std::string &data);
};

std::string CIDHeader::_sha1hash(const std::string &data)
{
	char hash[XIA_SHA_DIGEST_STR_LEN];
	xs_getSHA1HexDigest((const unsigned char *)data.c_str(), data.size(),
			hash, sizeof(hash));
	std::string hashstr(hash, XIA_SHA_DIGEST_STR_LEN);
	return hashstr;
}

CIDHeader::CIDHeader(const std::string &data, time_t ttl)
{
	_len = data.size();
	_ttl = ttl;
	_id = "CID:" + _sha1hash(data);
	_store_id = _id;
}

std::string
CIDHeader::serialize()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	ContentHeaderBuf chdr_buf;
	CIDHeaderBuf *cid_header = chdr_buf.mutable_cid_header();
	cid_header->set_len(_len);
	cid_header->set_ttl(_ttl);
	cid_header->set_id(_id);
	std::string serialized_header;
	chdr_buf.SerializeToString(&serialized_header);
	return serialized_header;
}

bool
CIDHeader::deserialize(const std::string &buf)
{
	ContentHeaderBuf chdr_buf;
	if(chdr_buf.ParseFromString(buf) == false) {
		return false;
	}
	if(!chdr_buf.has_cid_header()) {
		return false;
	}
	const CIDHeaderBuf cid_hdr = chdr_buf.cid_header();
	_len = cid_hdr.len();
	_ttl = cid_hdr.ttl();
	_id = _store_id = cid_hdr.id();
	return true;
}
#endif // _CID_HEADER_H