/* -*- mode: c; c-basic-offset: 4 -*- */
#ifndef CLICKNET_XIA_H
#define CLICKNET_XIA_H

/*
 * <clicknet/xia.h> -- XIA packet header, only works in user-level click
 * 
 */

#define CLICK_XIA_XID_TYPE_UNDEF 0
#define CLICK_XIA_XID_TYPE_AD 1
#define CLICK_XIA_XID_TYPE_HID 2
#define CLICK_XIA_XID_TYPE_CID 3
#define CLICK_XIA_XID_TYPE_SID 4
#define CLICK_XIA_XID_TYPE_MAX 4

#define CLICK_XIA_XID_ID_LEN 20

struct click_xia_xid {
    uint16_t type;
    uint8_t id[CLICK_XIA_XID_ID_LEN];
};

struct click_xia_xid_edge
{
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    unsigned idx : 7;                   /* index of node this edge points to */
    unsigned visited : 1;               /* visited edge? */
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
    unsigned visited : 1;
    unsigned idx : 7;
#else
#   error "unknown byte order"
#endif
};

#define CLICK_XIA_XID_EDGE_UNUSED (127u)

struct click_xia_xid_node {
    click_xia_xid xid;
    click_xia_xid_edge edge[4];
};

struct click_xia_common {
    uint8_t ver;
    uint8_t rest[0];
};

// XIA network layer packet header
struct click_xia {
    uint8_t ver;			/* header version */
    uint8_t nxt;			/* next header */
    uint16_t plen;			/* payload length */
    uint8_t dsnode;			/* total number of all nodes (excl. dummy node) */
    uint8_t dnode;			/* total number of dest nodes (excl. dummy node) */
    uint8_t dint;                       /* index of first intent node within dest nodes */
    uint8_t sint;                       /* index of first intent node within source nodes */
    int8_t last;			/* index of last visited node (note: integral) */
    uint8_t hlim;			/* hop limit */
    click_xia_xid_node node[0];         /* XID node list */
};

#define CLICK_XIA_PROTO_XCMP 0

// XIA control message protocol header (followed by initial packet data)
struct click_xia_xcmp {
    uint8_t type;
    uint8_t code;
    uint8_t rest[0];
};

// XIA control message protocol header for echo
struct click_xia_xcmp_sequenced {
    uint8_t type;
    uint8_t code;
    uint16_t identifier;
    uint16_t sequence;
    uint8_t rest[0];
};

#define CLICK_XIA_XCMP_ECHO             0                   /* echo request              */
#define CLICK_XIA_XCMP_ECHO_REPLY       1                   /* echo reply                */
#define CLICK_XIA_XCMP_UNREACH          2                   /* dest unreachable          */
#define   CLICK_XIA_XCMP_UNREACH_XID_TYPE           0       /*   unknown XID type        */
#define   CLICK_XIA_XCMP_UNREACH_XID                1       /*   unknown XID             */
#define   CLICK_XIA_XCMP_UNREACH_PROTO              2       /*   unknown protocol        */
#define   CLICK_XIA_XCMP_UNREACH_FRAG               3       /*   frag required           */
#define CLICK_XIA_XCMP_TIMXCEED         3                   /* time exceeded             */
#define CLICK_XIA_XCMP_PARAMPROB        4                   /* bad header                */
#define   CLICK_XIA_XCMP_PARAMPROB_LENGTH           0       /*   invalid length          */
#define   CLICK_XIA_XCMP_PARAMPROB_NXIDS            1       /*   invalid number of XIDs  */
#define   CLICK_XIA_XCMP_PARAMPROB_NDST             2       /*   invalid number of dests */
#define   CLICK_XIA_XCMP_PARAMPROB_LAST             3       /*   invalid last            */

#endif
