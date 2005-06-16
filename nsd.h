/*
 * nsd.h -- nsd(8) definitions and prototypes
 *
 * Copyright (c) 2001-2004, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
 */

#ifndef	_NSD_H_
#define	_NSD_H_

#include <signal.h>

#include "dns.h"
#include "edns.h"
#include "options.h"

#define	NSD_RUN	0
#define	NSD_RELOAD 1
#define	NSD_SHUTDOWN 2
#define	NSD_STATS 3
#define	NSD_QUIT 4

enum nsd_server_kind
{
	NSD_SERVER_KIND_MAIN,
	NSD_SERVER_KIND_CHILD
};
typedef enum nsd_server_kind nsd_server_kind_type;

#ifdef INET6
#define DEFAULT_AI_FAMILY AF_UNSPEC
#else
#define DEFAULT_AI_FAMILY AF_INET
#endif

enum nsd_socket_kind
{
	NSD_SOCKET_KIND_UDP,
	NSD_SOCKET_KIND_TCP,
	NSD_SOCKET_KIND_NSDC
};
typedef enum nsd_socket_kind nsd_socket_kind_type;

#ifdef BIND8_STATS

/* Counter for statistics */
typedef	unsigned long stc_t;

#define	LASTELEM(arr)	(sizeof(arr) / sizeof(arr[0]) - 1)

#define	STATUP(nsd, stc) nsd->st.stc++
/* #define	STATUP2(nsd, stc, i)  ((i) <= (LASTELEM(nsd->st.stc) - 1)) ? nsd->st.stc[(i)]++ : \
				nsd->st.stc[LASTELEM(nsd->st.stc)]++ */

#define	STATUP2(nsd, stc, i) nsd->st.stc[(i) <= (LASTELEM(nsd->st.stc) - 1) ? i : LASTELEM(nsd->st.stc)]++
#else	/* BIND8_STATS */

#define	STATUP(nsd, stc) /* Nothing */
#define	STATUP2(nsd, stc, i) /* Nothing */

#endif /* BIND8_STATS */

struct nsd_socket
{
	nsd_socket_kind_type    kind;
	struct addrinfo	*	addr;
	int			s;
};
typedef struct nsd_socket nsd_socket_type;

struct nsd_child
{
	/* The child's process id.  */
	pid_t pid;

	/*
	 * Socket used by the parent process to send commands and
	 * receive responses to/from this child process.
	 */
	int child_fd;

	/*
	 * Socket used by the child process to receive commands and
	 * send responses from/to the parent process.
	 */
	int parent_fd;
};
typedef struct nsd_child nsd_child_type;


/* NSD configuration and run-time variables */
typedef struct nsd nsd_type;
struct nsd
{
	/*
	 * Global region that is not deallocated until NSD shuts down.
	 */
	region_type    *region;

	/* Run-time variables */
	pid_t		pid;
	volatile sig_atomic_t mode;
	nsd_server_kind_type server_kind;
	struct namedb	*db;
	int		debug;

	/*
	 * Number of servers is specified in the 'options'
	 * structure.
	 */
	nsd_child_type  *children;

	/* NULL if this is the parent process.  */
	nsd_child_type  *this_child;

	/* Configuration */
	const char       *options_file;
	nsd_options_type *options;

	uid_t	uid;
	gid_t	gid;

	/* All sockets that NSD is using for incoming data.  */
	size_t           socket_count;
	nsd_socket_type *sockets;

	edns_data_type edns_ipv4;
#if defined(INET6)
	edns_data_type edns_ipv6;
#endif

	/* Maximum is specified in the 'options' structure.  */
	size_t current_tcp_connection_count;

#ifdef	BIND8_STATS

	char	*named8_stats;

	struct nsdst {
		time_t	boot;
		stc_t	qtype[257];	/* Counters per qtype */
		stc_t	qclass[4];	/* Class IN or Class CH or other */
		stc_t	qudp, qudp6;	/* Number of queries udp and udp6 */
		stc_t	ctcp, ctcp6;	/* Number of tcp and tcp6 connections */
		stc_t	rcode[17], opcode[6]; /* Rcodes & opcodes */
		/* Dropped, truncated, queries for nonconfigured zone, tx errors */
		stc_t	dropped, truncated, wrongzone, txerr, rxerr;
		stc_t 	edns, ednserr, raxfr, nona;
	} st;
#endif /* BIND8_STATS */
};

/* nsd.c */
pid_t readpid(const char *file);
int writepid(nsd_type *nsd);
void sig_handler(int sig);
void bind8_stats(nsd_type *nsd);

/* server.c */
int server_init(nsd_type *nsd);
void server_main(nsd_type *nsd);
void server_child(nsd_type *nsd);

#endif	/* _NSD_H_ */
