/* wdb.h - WDB protocol definition header */

/*
 * Copyright (c) 1995,1999-2000,2003-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use of this
 * software may be licensed only pursuant to the terms of an applicable Wind
 * River license agreement.
 */

/*
modification history
--------------------
03s,06jul07,tbu  Added 64 bits support
	   +x_p  completed 64 bits support
	   +p_b  completed 64 bits support
03r,30may07,dbt  Added WDB_COMMAND_SEND WDB request.
03w,14jun07,tbu  Added WDB_CONNECT_FORCE
03v,12jun07,x_p  Remove usage of TGT_ADDR_T
03u,21may07,tbu  Fixed the argv to be TGT_LONG_T in case of a sys ctx
03t,22mar07,x_p  64 bits changes
03s,06mar07,tbu  Changed the WDB_CORE for passing the pointer size
03r,02feb07,tbu  Added the support for 64 bits (and more)
03q,16jun06,jeg  removed the signal handling enum to define it inside the
		 foundation tree.
03p,12jun06,jeg  added signal handling enum.
03o,05dec05,dbt  No longer include rpc.h on runtime.
		 Moved xdr routine prototypes to xdrwdb.h
		 Cleanup
03n,24nov05,dbt  Added xdr_VOID prototype.
03m,22nov05,dbt  Removed handling of width parameter in xdr_WDB_OPQ_DATA_T.
03l,15nov05,dbt  Moved WDB_ACTION_STOP_MASK to wdbDbgLib.h.
03k,29apr05,bpn  Added WDB_RET_TYPE_CLEAR macro (SPR#108704).
03j,04jun04,dbt  Added ctors and dtors fields in WDB_MOD_INFO.
03i,06may04,dbt  Updated Modules & Symbols synchronization structures.
03h,28apr04,elg  Change context name in exception information structure.
03g,27apr04,elg  Add signal number information in exception event.
03f,13apr04,elg  Remove coprocessor types.
03e,23mar04,elg  Rework way contexts are transmitted in events.
03d,05mar04,elg  Modify context structure to have more generic.
03c,22jan04,elg  Add new events for breakpoint synchronization.
		 Add new event for agent mode change notification.
		 Modify exception event information.
03c,26feb04,mil  Added SPE support for PPC85XX.
03b,05nov03,x_p  Changed CTX_CREATE_DESC to handle argc and envp for Linux
03a,13oct03,elg  Modify breakpoint information.
02z,30sep03,elg  Finalize WDB protocol.
		 Add CPU family and CPU variant fields.
02y,29sep03,fle  rather include rpc/rpc.h than xdr/rpc.h
02x,08sep03,tbu  added WDB_LINUX_INFO type
02w,20may03,elg  Add RTP context type.
02v,27feb03,elg  Merge file with BSD.
02u,04feb03,elg  Add WDB merged protocol.
02t,19dec00,elg  Documentation check up.
02s,05sep00,elg  Modify the WDB_RT_INFO structure (SPR 32724).
02r,29aug00,elg  Modify wdbCore structure (SPR 32749).
02q,06jun00,dbt  Added several fields in module descriptor.
02p,30may00,dbt  Added partition ID in section descriptor.
02o,12apr00,elg  Add address information for shared libraries and shared data.
02n,06mar00,elg  Add WDB_VXWORKS_INFO structure.
02m,31jan00,dbt  Added event to notify tgtsvr when a context is updated,
		 updated WDB version
02l,30jan00,dbt  Added user services.
02k,25jan00,elg  Fix doc generation error.
02j,10jan00,dbt  Code cleanup for module/symbol synchronization code review.
02i,22nov99,elg  Add float support in WDB_FUNC_CALL.
02h,04nov99,dbt  Updated symbol table synchronization support.
02g,02nov99,elg  Update WDB_CTX_CREATE_DESC for PD creation.
02f,26oct99,dbt  Added support for trigger.
02e,17sep99,dbt  added support for WDB symbol table synchronization.
02d,15sep99,elg  Add support for overlapped memory model.
02c,02sep99,bpn  The WDBPORT define makes reference to the communication port
                 variable (SPR #28606).
02b,06aug99,elg  Add tool Id in WDB_CALL_RET_INFO.
02a,28jun99,elg  Add WDB_CORE support.
01z,06apr99,elg  add new WDB requests for Aeolus.
01a,04apr95,ms	 derived from work by tpr (and pme and ms).
		  merged wdb.h, wdbtypes.h, comtypes.h, and  xdrwdb.h
		  removed #ifdef UNIX
		  made all data types of the form wdbXxx_t.
		  added types WDB_STRING_T, WDB_OPQ_DATA_T, TGT_ADDR_T
		    TGT_INT_T, and UINT32
		  made most event data an array of ints.
		  removed obsolete data types.
*/

#ifndef __INCwdbh
#define __INCwdbh

#ifdef __cplusplus
extern "C" {
#endif

/*
DESCRIPTION

The WDB protocol provides and extensible interface for remote debugging.

The WDB agent has a fixed UDP port number in order to remove the complexity
of having a portmapper (which adds significant overhead if the
agent is configured as a ROM-monitor).

Every RPC request sent to the agent has a four byte sequence number
appended to the begining of the parameters. The two most significant
bytes are the ID of the host.

Every RPC reply from the agent has a four byte error status appended
to the begining of the reply. One of the bits of the error status
is reserved to mean that events are pending on the target, and another one is
reserved to mean that another request is needed to get a full result. The rest
of the error status is either OK (on success) or the reason
for failure. If a procedure fails, then the rest of the reply is
not decoded (e.g., it contains garbage).

REQUIREMENTS

Both the host and target must support 32 bit integers.

INTERFACE

errCode = wtxBeCall (HBackend, procNum, &params, &reply);

DATA TYPES

	UINT32		/@ 32 bit unsigned integer (defined in host.h)  @/
	BOOL		/@ TRUE or FALSE @/
	STATUS		/@ OK or ERROR @/
	TGT_ADDR_T	/@ target address @/
	TGT_INT_T	/@ target int @/
	TGT_LONG_T	/@ target long @/
	WDB_STRING_T	/@ pointer to a string (a local address) @/
	WDB_OPQ_DATA_T	/@ pointer to a block of memory (a local address) @/

Each side must also provide the following XDR filters:

	xdr_UINT32	/@ encode/decode a 32 bit integer @/
*/

/* includes */

#ifdef HOST
#include <host.h>
#else /* HOST */
#include <wdb/wdbOsLib.h>
#endif /* HOST */

/* definitions */

/*
 * WDB function numbers.
 * Each remote service is identified by an integer.
 */

	/* Session Management */

#define WDB_TARGET_PING			 0	/* check if agent is alive */
#define	WDB_TARGET_CONNECT		 1	/* connect to the agent */
#define WDB_TARGET_DISCONNECT		 2	/* terminate the connection */
#define WDB_TARGET_MODE_SET		 3	/* change the agents mode */
#define WDB_TARGET_MODE_GET		 4	/* get the agent mode */

	/* Memory Operations */

#define	WDB_MEM_READ			10	/* read a memory block */
#define	WDB_MEM_WRITE			11	/* write a memory block */
#define	WDB_MEM_FILL			12	/* fill memory with pattern */
#define WDB_MEM_MOVE			13	/* move memory on target */
#define	WDB_MEM_CHECKSUM		14	/* checksum a memory block */
#define WDB_MEM_PROTECT			15	/* write (un)protecting */
#define WDB_MEM_CACHE_TEXT_UPDATE	16	/* called after loading text */
#define WDB_MEM_SCAN			17	/* scan memory for a pattern */
#define WDB_MEM_MANY_WRITE		18	/* scatter write */
#define WDB_MEM_WRITE_MANY_INT		19	/* scatter write of ints */

	/* Context Control */

#define	WDB_CONTEXT_CREATE		30	/* create a new context */
#define	WDB_CONTEXT_KILL		31	/* remove a context */
#define	WDB_CONTEXT_SUSPEND		32	/* suspend a context */
#define WDB_CONTEXT_RESUME		33	/* resume a context */
#define WDB_CONTEXT_STATUS_GET		34	/* get the context status */
#define WDB_CONTEXT_ATTACH		35	/* attach ctx to the agent */
#define WDB_CONTEXT_DETACH		36	/* detach ctx from the agent */

	/* Register Manipulation */

#define	WDB_REGS_GET			40	/* get register(s) */
#define	WDB_REGS_SET			41	/* set register(s) */

	/* Virtual I/O */

#define	WDB_VIO_WRITE			51	/* write a virtual I/O buffer */

	/* Eventpoints */

#define	WDB_EVENTPOINT_ADD		60	/* add an eventpoint */
#define	WDB_EVENTPOINT_DELETE		61	/* delete an eventpoint */

	/* Events */

#define	WDB_EVENT_GET			70	/* get info about an event */

	/* debugging */

#define WDB_CONTEXT_CONT		80	/* continue a context */
#define	WDB_CONTEXT_STEP		81	/* step a context */
#define	WDB_CONTEXT_STOP		82	/* stop a context */

	/* Miscelaneous */

#define	WDB_FUNC_CALL			90	/* spawn a function */
#define WDB_EVALUATE_GOPHER		91	/* evaluate a gopher tape */
#define WDB_DIRECT_CALL			92	/* call a function directly */
#define WDB_COMMAND_SEND		93	/* send a command string to */
						/* target agent */

	/* module management */

#define WDB_SYM_LIST_ADD		100	/* add a symbol list */
#define WDB_SYM_GET			101	/* get symbol info. */
#define WDB_MOD_INFO_GET		102	/* get module info. */
#define WDB_MOD_ADD			103	/* add a module */
#define WDB_MOD_REMOVE			104	/* remove a module */

	/* Target commands */

#define	WDB_USR_SVC_CTRL		110	/* Add or Remove a command */
						/* on the target */
#define WDB_USR_SVC_CALL		111	/* call a command on the */
						/* target */

	/* Thread Management */

#define	WDB_THREAD_INFO_SET		120	/* set thread debug info */
#define	WDB_THREAD_INFO_GET		121	/* get thread debug info */

	/* new generic connection request (for support of 64bits, and more) */

#define WDB_TARGET_CONNECT2		122	/* connect to the agent */
#define WDB_TGT_INFO_GET		123	/* request for various info */

/*
 * WDB error codes.
 * Each WDB function returns an error code.
 * If the error code is zero (OK), then the procedure succeded and
 * the reply data is valid.
 * If the error code is nonzero, then the procedure failed. In this case
 * the error code indicates the reason for failure and the reply data
 * is invalid.
 */

#define WDB_OK				OK	/* success */
#define WDB_ERR_INVALID_PARAMS		0x501	/* params invalid */
#define WDB_ERR_MEM_ACCESS	 	0x502	/* memory fault */
#define WDB_ERR_AGENT_MODE		0x503	/* wrong agent mode */
#define WDB_ERR_RT_ERROR 		0x504	/* run-time callout failed */
#define WDB_ERR_INVALID_CONTEXT 	0x505	/* bad context ID */
#define WDB_ERR_INVALID_VIO_CHANNEL	0x506	/* bad virtual I/O channel */
#define WDB_ERR_INVALID_EVENT 		0x507	/* no such event type */
#define WDB_ERR_INVALID_EVENTPOINT 	0x508	/* no such eventpoint */
#define WDB_ERR_GOPHER_FAULT 		0x509	/* gopher fault */
#define WDB_ERR_GOPHER_TRUNCATED 	0x50a	/* gopher tape too large */
#define WDB_ERR_EVENTPOINT_TABLE_FULL	0x50b	/* out of room */
#define WDB_ERR_NO_AGENT_PROC		0x50c	/* agent proc not installed */
#define WDB_ERR_NO_RT_PROC		0x50d	/* run-time callout unavail */
#define WDB_ERR_GOPHER_SYNTAX		0x50e   /* gopher syntax error */
#define WDB_ERR_NOT_FOUND		0x50f	/* object not found */
#define WDB_ERR_HW_REGS_EXHAUSTED	0x510	/* hw breakpoint regs full */
#define WDB_ERR_INVALID_HW_BP		0x511	/* invalid hw breakpoint */
#define WDB_ERR_USR_SVC_EXHAUSTED	0x512	/* no more use service */
						/* available */
#define WDB_ERR_USR_SVC_ALREADY_USED	0x513	/* specified user service id */
						/* is already used */
#define	WDB_ERR_NO_RT_CTX		0x514	/* context not supported by */
						/* run time */
#define	WDB_ERR_NO_PROCESS		0x515	/* no process support */

#define	WDB_ERR_FILE_NOT_FOUND		0x520	/* file not found */
#define	WDB_ERR_FILE_NULL_SIZE		0x521	/* size of file = 0 */
#define	WDB_ERR_FILE_BAD_FORMAT		0x522	/* bad file format */

#define WDB_ERR_PROC_FAILED		0x5ff	/* generic proc failure */

#define WDB_ERR_NO_CONNECTION		0x600	/* not connected */
#define WDB_ERR_CONNECTION_BUSY		0x601	/* someone else connected */
#define WDB_ERR_COMMUNICATION		0x6ff	/* generic comm error */

/*
 * WDB miscelaneous definitions.
 */

	/* agent modes */

#define WDB_MODE_APP	1			/* application mode agent */
#define WDB_MODE_TASK	WDB_MODE_APP		/* task mode agent */
#define WDB_MODE_USER	WDB_MODE_APP		/* user mode agent */
#define WDB_MODE_SYSTEM	2			/* system mode agent */
#define WDB_MODE_EXTERN	WDB_MODE_SYSTEM		/* system mode agent */
#define WDB_MODE_DUAL	(WDB_MODE_TASK | WDB_MODE_EXTERN) /* dual mode */
#define WDB_MODE_BI	WDB_MODE_DUAL

	/* size of WDB headers */

#define WDB_IP_HDR_SZ		20
#define	WDB_UDP_HDR_SZ		8
#define WDB_RPC_REPLY_HDR_SZ	24
#define WDB_RPC_REQUEST_HDR_SZ	40
#define WDB_WRAPPER_HDR_SZ	12

	/* maximum number of words of event data */

#define WDB_MAX_EVT_DATA	20

	/* gopher stream format type codes */

#define GOPHER_UINT32		0
#define GOPHER_STRING		1
#define GOPHER_UINT16		2
#define GOPHER_UINT8            3
#define GOPHER_FLOAT32          4
#define GOPHER_FLOAT64          5
#define GOPHER_FLOAT80          6
#define GOPHER_SEPARATOR	7
#define GOPHER_UINT64		8

	/* option bits for task creation */

#define WDB_UNBREAKABLE	0x0002		/* ignore breakpoints */
#define WDB_FP_RETURN	0x8000		/* return value is a double */

/* set / get / clear the return type in the options word */

#define	WDB_RET_TYPE_SET(x)	(((x) & 0x3) << 15)
#define	WDB_RET_TYPE_GET(x)	(((x) >> 15) & 0x3)
#define	WDB_RET_TYPE_CLEAR(x)	*(x) &= ~(0x3 << 15);

        /* TSFS file type bits */

#define WDB_TSFS_S_IFREG	0x4000 	/* directory */
#define WDB_TSFS_S_IFDIR	0x8000	/* regular file */

        /* TSFS open mode flags */

#define	WDB_TSFS_O_RDONLY	0
#define	WDB_TSFS_O_WRONLY	1
#define	WDB_TSFS_O_RDWR		2
#define	WDB_TSFS_O_CREAT	0x0200
#define	WDB_TSFS_O_TRUNC	0x0400
#define	WDB_TSFS_O_APPEND	0x0008
#define	WDB_TSFS_O_EXCL		0x0800
#define	WDB_TSFS_O_TEXT		0x4000  /* for Win32 hosts; to override default
					   binary translation mode */

	
        /* TSFS portable errno values */

/*
 * POSIX errnos are used by the TSFS but not defined here.  These errno
 * values are portable between the target and host, because they are not
 * supported by one of the host platforms, or the errno is numbered
 * differently on each host platform.
 */

#define WDB_TSFS_ERRNO_ENOTEMPTY	(1 << 16)
#define WDB_TSFS_ERRNO_EDEADLK		(2 << 16)
#define WDB_TSFS_ERRNO_ENOLCK		(3 << 16)
#define WDB_TSFS_ERRNO_EMSGSIZE		(4 << 16)
#define WDB_TSFS_ERRNO_EOPNOTSUPP	(5 << 16)
#define WDB_TSFS_ERRNO_EADDRNOTAVAIL	(6 << 16)
#define WDB_TSFS_ERRNO_ENOTSOCK		(7 << 16)
#define WDB_TSFS_ERRNO_ENETRESET	(8 << 16)
#define WDB_TSFS_ERRNO_ECONNABORTED	(9 << 16)
#define WDB_TSFS_ERRNO_ECONNRESET	(10 << 16)
#define WDB_TSFS_ERRNO_ECONNREFUSED	(11 << 16)
#define WDB_TSFS_ERRNO_ENOBUFS		(12 << 16)
#define WDB_TSFS_ERRNO_ENOTCONN		(13 << 16)
#define WDB_TSFS_ERRNO_ESHUTDOWN	(14 << 16)
#define WDB_TSFS_ERRNO_ETIMEDOUT	(15 << 16)
#define WDB_TSFS_ERRNO_EINPROGRESS	(16 << 16)
#define WDB_TSFS_ERRNO_EWOULDBLOCK	(17 << 16)
#define WDB_TSFS_ERRNO_ENOSR		(18 << 16)
#define WDB_TSFS_ERRNO_ELOOP		(19 << 16)
#define WDB_TSFS_ERRNO_ENAMETOOLONG	(20 << 16)
#define WDB_TSFS_ERRNO_EBADMSG		(21 << 16)

/* session information */

#ifndef HOST
#define WDBPORT 	        wdbPort		/* this variable contains */
                                                /* the UDP port to connect */
#else	/* HOST */
#define WDBPORT			(UINT16) 0x4321	/* UDP port to connect */
#endif	/* HOST */

#define WDBPROG	(UINT32)	0x55555555	/* RPC program number */
#define WDBVERS	(UINT32)	1		/* RPC version number */
#define WDB_VERSION_STR		"5.0"

/* message cores */

#define WDB_EVENT_NOTIFY	0x8000		/* notify bit in errCode */
#define WDB_TO_BE_CONTINUED	0x4000		/* continue bit in errCode */
#define WDB_HOST_ID_MASK	0xffff0000	/* hostId in seqNum */

/* memory options */

#define	WDB_MEM_OPT_UNPROTECT	0x1		/* unprotect memory */
#define	WDB_MEM_OPT_BYTE	0x10000000	/* memory byte access */
#define	WDB_MEM_OPT_SHORT	0x20000000	/* memory short access */
#define	WDB_MEM_OPT_INT		0x40000000	/* memory int access */

#define	WDB_MEM_WIDTH_GET(x)	((x & 0xf0000000) >> 28)
#define	WDB_MEM_WIDTH_SET(x)	((x & 0xf) << 28)

/* symbol table synchronization options */

#define WDB_SYM_SYNC_CORE_FILE	0x1		/* synchronize core file sym. */

/* available agent options */

#define	WDB_AGENT_IS_SCALABLE		0x1	/* agent is scalable */
#define	WDB_AGENT_HAS_MDL_SYM_SYNC	0x2	/* support for module and */
						/* SYNC */

/* connection options */

#define WDB_CONNECT_FORCE	0x00000001	/* disconnect the existing
						   connection when set */

/* keep these values for backward compatibility issues */

#define	WDB_REG_SET_IU		0	/* integer unit register set */
#define	WDB_REG_SET_FPU		1	/* floating point unit register set */
#define	WDB_REG_SET_MMU		2	/* memory management unit reg set */
#define	WDB_REG_SET_CU		3	/* cache unit register set */
#define	WDB_REG_SET_TPU		4	/* timer processor unit register set */
#define	WDB_REG_SET_SYS		5	/* system registers */
#define	WDB_REG_SET_DSP		6	/* dsp registers */
#define	WDB_REG_SET_AV		7	/* altivec registers */
#define WDB_REG_SET_SPE		8	/* SPE registers */

/* typedefs */

/* To handle the cast to/from EVT_DATA opaque structure */

#define WDB_CTX_STATUS_EX	TGT_LONG_T
#define WDB_CTX_TYPE_EX		TGT_LONG_T
#define WDB_EVT_TYPE_EX		TGT_LONG_T
#define WDB_EVT_OPTION_TYPE_EX	TGT_LONG_T
/*
 * WDB enum types.
 */

typedef enum wdb_ctx_type		/* type of context on the target */
    {
    WDB_CTX_SYSTEM	= 0,		/* system mode */
    WDB_CTX_GROUP	= 1,		/* process group (not implemented) */
    WDB_CTX_ANY		= 2,		/* any context (not implemented) */
    WDB_CTX_TASK	= 3,		/* specific task or processes */
    WDB_CTX_ANY_TASK	= 4,		/* any task */
    WDB_CTX_ISR		= 5,		/* specific ISR */
    WDB_CTX_ANY_ISR	= 6,		/* any ISR */
    WDB_CTX_PD		= 7,		/* specific protection domain */
    WDB_CTX_PROCESS	= 8,		/* specific process */
    WDB_CTX_RTP		= 9,		/* specific RTP */
    WDB_CTX_TYPE_NUM			/* type number, must be last field */
    } WDB_CTX_TYPE;

typedef enum wdb_ctx_status		/* status of context on the target */
    {
    WDB_CTX_RUNNING	= 0,		/* context is running */
    WDB_CTX_SUSPENDED	= 1,		/* context is suspended */
    WDB_CTX_STOPPED	= 2		/* context is stopped */
    } WDB_CTX_STATUS;

typedef enum wdb_evt_type		/* type of event on the target */
    {
    WDB_EVT_NONE		= 0,	/* no event */
    WDB_EVT_CTX_START		= 1,	/* context creation */
    WDB_EVT_CTX_EXIT		= 2,	/* context exit */
    WDB_EVT_BP			= 3,	/* breakpoint */
    WDB_EVT_HW_BP		= 4,	/* hardware breakpoint */
    WDB_EVT_WP			= 5,	/* watchpoint */
    WDB_EVT_EXC			= 6,	/* exception */
    WDB_EVT_VIO_WRITE		= 7,	/* virtual I/O write */
    WDB_EVT_CALL_RET		= 8,	/* function call finished */
    WDB_EVT_TSFS_OP		= 9,	/* VIO II operation */
    WDB_EVT_USER		= 10,	/* user event */
    WDB_EVT_MOD_LOAD		= 11,	/* object module load */
    WDB_EVT_MOD_UNLOAD		= 12,	/* object module unload */
    WDB_EVT_SYM_ADD		= 13,	/* symbol add */
    WDB_EVT_TRIGGER		= 14,	/* trigger event */
    WDB_EVT_CTX_UPDATE		= 15,	/* context update */
    WDB_EVT_BP_CREATE		= 16,	/* breakpoint creation */
    WDB_EVT_BP_DELETE		= 17,	/* breakpoint deletion */
    WDB_EVT_MODE		= 18,	/* WDB mode changed */
    WDB_EVT_HW_BP_VALUE		= 19,	/* h/w bp with value check */
    WDB_EVT_CTX_DUMP		= 30	/* context dump */
    } WDB_EVT_TYPE;

typedef enum wdb_action_type		/* what to do when an event occurs */
    {
    WDB_ACTION_CALL		= 1,	/* condition the evtpt via a proc */
    WDB_ACTION_NOTIFY		= 2,	/* notify the host */
    WDB_ACTION_STOP		= 4,	/* stop the task or process */
    WDB_ACTION_SYS_STOP		= 8,	/* stop system */
    WDB_ACTION_ALL_STOP		= 16	/* stop RTP tasks, or kernel tasks */
    } WDB_ACTION_TYPE;

typedef enum wdb_vt_option_type		/* breakpoint options */
    {
    WDB_EVT_BP_PERSISTENT	= 1	/* breakpoint is persistent */
    } WDB_EVT_OPTION_TYPE;

typedef enum wdb_call_ret_type		/* type of return value */
    {
    WDB_CALL_RET_INT	= 0,		/* an integer is returned 	*/
    WDB_CALL_RET_DBL	= 1,		/* a double is returned 	*/
    WDB_CALL_RET_FLT	= 2,		/* a float is returned 		*/
    WDB_CALL_RET_LONG	= 3		/* a long is returned		*/
    } WDB_CALL_RET_TYPE;

typedef enum wdb_rt_type		/* type of run-time system */
    {
    WDB_RT_NULL		= 0,		/* standalone WDB agent */
    WDB_RT_VXWORKS	= 1		/* WDB agent integrated in VxWorks */
    } WDB_RT_TYPE;

typedef enum				/* types of WDB_TGT_INFO_GET request */
    {
    WDB_RUNTIME_INFO	= 0
    /* to be extended */
    } WDB_TGT_INFO_TYPE;

/*
 * WDB primitive data types.
 */

#ifndef	HOST
typedef void * 	TGT_ADDR_T;
typedef long	TGT_LONG_T;
#define TGT_INT_T UINT32

#define TGT_ADDR_T_SIZE	sizeof(TGT_ADDR_T) /* more tricky on the host side */

#else
/* We need to define TGT_ADDR_T to a 64 bits host type */
#define TGT_ADDR_T	UINT64
#define TGT_ADDR_T_SIZE	target_xdr_tgtAddrSize

#endif	/* HOST */

typedef char *	WDB_STRING_T;
typedef char *	WDB_OPQ_DATA_T;
typedef char **	WDB_STRING_ARRAY;


typedef enum 
    {
    WDB_POINTER_16_SIZE = 2,
    WDB_POINTER_32_SIZE = 4,
    WDB_POINTER_64_SIZE = 8
    } WDB_POINTER_SIZE;

typedef struct
    {
    TGT_INT_T		ctx;		/* the memory context id */
    WDB_POINTER_SIZE	pointerSize;
    } WDB_MEM_CTX_T;

/* WDB structure needed for all requests */

typedef struct wdb_core			/* common header WDB structure */
    {
    UINT32		numInts;	/* WDB_CORE length: 3 */
    UINT32		toolId;		/* tool ID */
    WDB_MEM_CTX_T       memCtx;         /* context WDB request sees */
    } WDB_CORE;

typedef struct				/* parameter of WDB_TGT_INFO_GET */
    {
    WDB_TGT_INFO_TYPE	tgtInfoType;	/* what kind of target info */
    } WDB_TGT_INFO_DESC;


typedef struct
    {
    UINT32	 param;			/* the connection parameter */
    WDB_STRING_T connectMsg;		/* a message of the connecting tgtsvr */
    } WDB_CONNECT_PARAMS;

/* Context Structures */

#define	WDB_CTX_MAX_ARGS	2	/* maximum of information in context */

typedef struct wdb_ctx			/* a particular context */
    {
    WDB_CTX_TYPE	contextType;	/* type of context */
    UINT32		numArgs;	/* structure length: it depends on */
    					/* context type and OS */
    TGT_LONG_T *	args;		/* information to characterize */
    					/* a context */
    } WDB_CTX;

typedef struct wdb_ctx_create_desc	/* how to create a context */
    {
    WDB_CTX_TYPE	contextType;	/* task, PD, process or system */
    WDB_STRING_T	name;		/* name */
    TGT_INT_T		redirIn;	/* redirect input file (or 0) */
    TGT_INT_T		redirOut;	/* redirect output file (or 0) */
    TGT_INT_T		redirErr;	/* redirect error output file (or 0) */
    TGT_ADDR_T		baseAddr;	/* virtual base address */

    union
    	{

    	/* the following are used for system contexts */

	struct
	    {
    	    TGT_ADDR_T 	 entry;		/* context entry point */
	    TGT_INT_T	 numArgs;	/* number of arguments */
	    TGT_LONG_T * args;		/* arguments */
	    } sys;

	/* the following are only used for task contexts */

	struct
	    {
	    TGT_ADDR_T   entry;		/* task entry point */
	    TGT_INT_T	 numArgs;	/* number of arguments */
	    TGT_LONG_T * args;		/* arguments */
	    TGT_ADDR_T	 pdId;		/* protection domain father */
    	    TGT_INT_T	 priority;	/* priority */
	    TGT_INT_T	 size;		/* task stack size */
	    TGT_INT_T	 options;	/* context options */
	    } task;

	/* the following are only used for protection domain contexts */

	struct
	    {
	    TGT_INT_T		loPriority;	/* lowest priority of a task */
	    TGT_INT_T		hiPriority;	/* highest priority of a task */
	    TGT_ADDR_T		physAddr;	/* shared data physical addr */
	    WDB_STRING_T	pagePoolList;	/* list page pools */
	    WDB_STRING_T	linkPath;	/* link path */
	    TGT_INT_T		size;		/* heap size */
	    TGT_INT_T		options;	/* context options */
	    } pd;

	/* the following are only used for RTP contexts */

	struct
	    {
	    TGT_INT_T		numArgv;	/* number of argv elements */
	    WDB_STRING_ARRAY	argv;		/* pointer to argv array */
	    TGT_INT_T		numEnvp;	/* number of envp elements */
	    WDB_STRING_ARRAY	envp;		/* pointer to envp array */
	    TGT_INT_T		priority;	/* priority of initial task */
	    TGT_INT_T		stackSize;	/* stack for initial task */
	    TGT_INT_T		options;	/* options passed to RTP */
	    } rtp;

	/* the following are only used for process contexts */

	struct
	    {
	    TGT_INT_T		uid;		/* process user ID */
	    TGT_INT_T		gid;		/* process group ID */
	    TGT_INT_T		numArgv;	/* number of argv elements */
	    WDB_STRING_ARRAY	argv;		/* pointer to argv array */
	    TGT_INT_T		numEnvp;	/* number of envp elements */
	    WDB_STRING_ARRAY	envp;		/* pointer to envp array */
	    TGT_INT_T		priority;	/* priority of initial task */
	    TGT_INT_T		stackSize;	/* stack for initial task */
	    TGT_INT_T		options;	/* options passed to RTP */
	    } process;
	} ctxSpecific;
    } WDB_CTX_CREATE_DESC;

typedef struct wdb_ctx_kill_desc	/* how to kill a context */
    {
    WDB_CTX		context;	/* context to kill */
    TGT_INT_T		options;	/* how to kill a context */
    } WDB_CTX_KILL_DESC;

typedef struct wdb_direct_call_ret	/* direct call return info */
    {
    TGT_LONG_T		returnVal;	/* returned value */
    TGT_LONG_T		errnoVal;	/* errno value */
    } WDB_DIRECT_CALL_RET;

/* Memory Structures */

typedef struct wdb_mem_region		/* a region of target memory */
    {
    TGT_INT_T		options;	/* memory options */
    TGT_ADDR_T		baseAddr;	/* memory region base address */
    TGT_INT_T		numBytes;	/* memory region size */
    TGT_INT_T		param;		/* procedure dependent parameter */
    } WDB_MEM_REGION;

typedef struct wdb_mem_xfer		/* transfer a block of memory */
    {
    TGT_INT_T		options;	/* memory options */
    WDB_OPQ_DATA_T	source;		/* data to transfer */
    TGT_ADDR_T		destination;	/* requested destination */
    TGT_INT_T		numBytes;	/* number of bytes transferred */
    } WDB_MEM_XFER;

typedef struct wdb_mem_scan_desc
    {
    WDB_MEM_REGION	memRegion;	/* region of memory to scan */
    WDB_MEM_XFER	memXfer;	/* pattern to scan for */
    } WDB_MEM_SCAN_DESC;

/* Debug Structures */

typedef struct wdb_ctx_step_desc	/* how to single step a context */
    {
    WDB_CTX		context;	/* context to step */
    TGT_ADDR_T		startAddr;	/* lower bound of step range */
    TGT_ADDR_T 		endAddr;	/* upper bound of step range */
    } WDB_CTX_STEP_DESC;

/* Register Structures */

typedef struct wdb_reg_read_desc	/* register data to read */
    {
    UINT32		regSetType;	/* type of register set to read */
    WDB_CTX		context;	/* context associated with registers */
    WDB_MEM_REGION	memRegion;	/* subregion of the register block */
    } WDB_REG_READ_DESC;

typedef struct wdb_reg_write_desc	/* register data to write */
    {
    UINT32		regSetType;	/* type of register set to write */
    WDB_CTX		context;	/* context associated with registers */
    WDB_MEM_XFER	memXfer;	/* new value of the register set */
    } WDB_REG_WRITE_DESC;

/* Target Structures */


typedef struct wdb_linux_info		/* Linux specific information */
    {
    UINT32	dummy;
    } WDB_LINUX_INFO;

typedef struct wdb_bsd_info		/* BSD/OS specific information */
    {
    UINT32		dummy;
    } WDB_BSD_INFO;

typedef struct wdb_vxworks_info		/* vxWorks specific information */
    {
    TGT_ADDR_T		pdList;		/* pointer on protection domain list */
    } WDB_VXWORKS_INFO;

typedef struct wdb_rt_info		/* info on the run-time system */
    {
    WDB_STRING_T	rtName;		/* runtime name */
    WDB_STRING_T	rtVersion;	/* run time version */
    TGT_INT_T		cpuFamily;	/* target processor family type */
    TGT_INT_T		cpuType;	/* target processor type */
    WDB_STRING_T	cpuVariant;	/* target processor variant */
    WDB_STRING_T	toolName;	/* tool used to build the kernel */
    WDB_STRING_T	bspShortName;	/* BSP short name */
    UINT32		hasCoprocessor;	/* target has a floating point unit */
    BOOL		hasWriteProtect; /* target can write protect memory */
    TGT_INT_T		pageSize;	/* size of a page */
    TGT_INT_T		endian;		/* endianness (0x4321 or 0x1234) */
    WDB_STRING_T	bspName;	/* board support package name */
    WDB_STRING_T	bootline;	/* boot file path or NULL if embedded */
    TGT_ADDR_T		memBase;	/* target main memory base address */
    UINT32		memSize;	/* target main memory size */
    TGT_INT_T		numRegions;	/* number of memory regions */
    WDB_MEM_REGION *	memRegion;	/* memory region descriptor(s) */
    TGT_ADDR_T		hostPoolBase;	/* host-controlled target memory pool */
    UINT32		hostPoolSize;	/* host-controlled memory pool size */
    UINT32		numLongs;	/* RT specific union length */
    union
    	{
	WDB_VXWORKS_INFO vxWorksInfo;	/* vxWorks specific information */
	WDB_BSD_INFO	 bsdInfo;	/* BSD/OS specific information  */
	WDB_LINUX_INFO   linuxInfo;	/* Linux specific information   */
	} rtSpecific;
    } WDB_RT_INFO;


typedef struct wdb_agent_info		/* info on the debug agent */
    {
    WDB_STRING_T	agentVersion;	/* version of the WDB agent */
    TGT_INT_T		mtu;		/* maximum transfer size in bytes */
    TGT_INT_T		mode;		/* available agent modes */
    TGT_INT_T		options;	/* available agent options */
    } WDB_AGENT_INFO;

typedef struct wdb_tgt_info		/* info on the target */
    {
    WDB_AGENT_INFO  	agentInfo;	/* info on the agent */
    WDB_RT_INFO	    	rtInfo;		/* info on the run time system */
    } WDB_TGT_INFO;

typedef struct 				/* reply to WDB_TARGET_CONNECT2 */
    {
    WDB_AGENT_INFO	agentInfo;	/* info about the agent 	*/
    WDB_STRING_T	rtName;		/* runtime name 		*/
    UINT32	 	tgtLongSize;	/* sizeof(long)  on target	*/
    UINT32	 	tgtAddrSize;   	/* sizeof(void*) on target	*/
    UINT32		tgtIntSize;	/* sizeod(int) on target	*/
    } WDB_CONNECT_INFO;

typedef struct 				/* wrapper for info on the target */
    {
    WDB_TGT_INFO_TYPE	infoType;	/* info for type of union */
    union 
    	{
	WDB_RT_INFO	rtInfo;		/* the basic information */
	/* can be extended */
	} info;
    } WDB_TGT_INFO_2;


/* Eventpoint Structures */

typedef struct wdb_action		/* a specific action */
    {
    WDB_ACTION_TYPE	actionType;
    TGT_LONG_T		actionArg;
    TGT_ADDR_T		callRtn;
    TGT_LONG_T		callArg;
    } WDB_ACTION;

typedef struct wdb_evtpt_add_desc	/* how to add an eventpoint */
    {
    WDB_EVT_TYPE	evtType;	/* type of event to detect */
    TGT_INT_T		numArgs;	/* eventType dependent arguments */
    TGT_LONG_T  *  	args;		/* arguments list */
    WDB_CTX		context;	/* context in which event must occur */
    WDB_ACTION 		action;		/* action to perform */
    } WDB_EVTPT_ADD_DESC;

typedef struct wdb_evtpt_del_desc	/* how to delete an eventpoint */
    {
    WDB_EVT_TYPE 	evtType;	/* type of event */
    TGT_LONG_T	 	evtptId;	/* eventpoint ID */
    } WDB_EVTPT_DEL_DESC;

/* Event Structures */

typedef struct wdb_evt_info		/* event info for anything but VIO */
    {
    UINT32		numLongs;	/* number of ints of event data */
    TGT_LONG_T 		info [WDB_MAX_EVT_DATA]; /* event data */
    } WDB_EVT_INFO;

typedef struct wdb_call_ret_info
    {
    UINT32		callId;		/* returned from WDB_FUNC_CALL */
    UINT32		toolId;		/* tool that did the function call */
    WDB_CALL_RET_TYPE	returnType;	/* return type */
    union
	{
	int		returnValInt;
	double		returnValDbl;	/* double return value */
	TGT_LONG_T	returnValLong;	/* integer return value */
	float		returnValFlt;	/* float return value */
	} returnVal;
    TGT_LONG_T		errnoVal;	/* error status */
    } WDB_CALL_RET_INFO;

typedef struct wdb_evt_data		/* reply to a WDB_EVENT_GET */
    {
    WDB_EVT_TYPE 	evtType;	/* event type detected */
    union 				/* eventType specific info */
	{
	WDB_MEM_XFER		vioWriteInfo;	/* vio write event data */
	WDB_CALL_RET_INFO	callRetInfo;	/* call return event data */
	WDB_EVT_INFO		evtInfo;	/* any other event info */
	} eventInfo;
    } WDB_EVT_DATA;

/* The next XXX_info structures are mapped on WDB_EVT_DATA */

typedef struct wdb_exc_info		/* WDB_EVT_INFO for exceptions */
    {
    UINT32	numLongs;	/* structure length */
    TGT_LONG_T	status;		/* status of context */
    TGT_LONG_T	stoppedCtxType; /* context stopped by exception */
    TGT_LONG_T	stoppedArgs [WDB_CTX_MAX_ARGS];
    TGT_LONG_T	excCtxType;	/* context that got exception */
    TGT_LONG_T	excArgs [WDB_CTX_MAX_ARGS];
    TGT_LONG_T	vec;		/* hardware trap number */
    TGT_ADDR_T	pEsf;		/* address of exception stack frame */
    TGT_LONG_T	sigNum;		/* signal number */
    } WDB_EXC_INFO;


typedef struct wdb_bp_info		/* WDB_EVT_INFO for breakpoints */
    {
    UINT32	numLongs;	/* structure length */
    TGT_LONG_T	status;		/* status of stopped context */
    TGT_LONG_T	stoppedCtxType;	/* context stopped on breakpoint */
    TGT_LONG_T	stoppedArgs [WDB_CTX_MAX_ARGS];
    TGT_LONG_T	hitCtxType;	/* context that hit breakpoint */
    TGT_LONG_T	hitArgs [WDB_CTX_MAX_ARGS];
    TGT_ADDR_T	pc;		/* program counter */
    TGT_ADDR_T	fp;		/* frame pointer */
    TGT_ADDR_T	sp;		/* stack pointer */
    TGT_ADDR_T	addr;		/* breakpoint address (watchpoint) */
    } WDB_BP_INFO;

typedef struct wdb_bp_create_info	/* WDB_EVT_INFO for BP creation */
    {
    UINT32	numLongs;	/* structure length */
    TGT_LONG_T	bpId;		/* breakpoint ID */
    TGT_LONG_T	bpType;		/* breakpoint type */
    TGT_ADDR_T	addr;		/* breakpoint address */
    TGT_LONG_T	count;		/* breakpoint count */
    TGT_LONG_T	ctxType;	/* breakpoint context */
    TGT_LONG_T	ctxArgs [WDB_CTX_MAX_ARGS];

    /* The WDB_ACTION fields */
    TGT_LONG_T	actionType;
    TGT_LONG_T	actionArg;
    TGT_ADDR_T	callRtn;
    TGT_LONG_T	callArg;

    TGT_LONG_T	options;	/* breakpoint options */
    TGT_LONG_T	type;		/* hardware breakpoint type */
    } WDB_BP_CREATE_INFO;

typedef struct wdb_bp_delete_info	/* WDB_EVT_INFO for BP deletion */
    {
    UINT32	numLongs;	/* 2 */
    TGT_LONG_T	bpId;		/* breakpoint ID deleted */
    TGT_LONG_T	bpType;		/* breakpoint type */
    } WDB_BP_DELETE_INFO;


typedef struct wdb_mode_info		/* WDB_EVT_INFO for mode change */
    {
    UINT32	numLongs;	/* 1 */
    TGT_LONG_T	mode;		/* new mode */
    } WDB_MODE_INFO;

typedef struct wdb_ctx_exit_info	/* WDB_EVT_INFO for context exit */
    {
    UINT32	numLongs;	/* structure length */
    TGT_LONG_T	ctxType;	/* context that exited */
    TGT_LONG_T	ctxArgs [WDB_CTX_MAX_ARGS];
    TGT_LONG_T	returnVal;	/* context's return value/exit code */
    TGT_LONG_T	errnoVal;	/* context's error status */
    } WDB_CTX_EXIT_INFO;

typedef struct wdb_ctx_update_info	/* WDB_EVT_INFO for context update */
    {
    UINT32	numLongs;	/* structure length */
    TGT_LONG_T	ctxType;	/* context updated */
    TGT_LONG_T	ctxArgs [WDB_CTX_MAX_ARGS];
    } WDB_CTX_UPDATE_INFO;

 
/* symbol table synchronization structures */

#define	WDB_MOD_NUM_SEGS	3	/* Number of segments in a module */

typedef struct wdb_sym_info		/* WDB_SYM_INFO for symbols */
    {
    struct wdb_sym_info *	next;	/* next symbol */
    TGT_INT_T		symId;		/* host or target symbol's ID */
    TGT_ADDR_T		value;		/* symbol value */
    TGT_INT_T		symRef;		/* symbols reference */
    TGT_INT_T		type;		/* symbol type */
    TGT_INT_T		group;		/* symbol group */
    TGT_INT_T		nameLength;	/* symbol name length */
    WDB_STRING_T	name;		/* symbol name */
    } WDB_SYM_INFO;

typedef struct wdb_sym_list		/* symbol list */
    {
    TGT_INT_T		index;		/* request index */
    BOOL		moreData;	/* more data to come ? */
    WDB_SYM_INFO *	pSymInfo;	/* Start of symbol list */
    } WDB_SYM_LIST;

typedef struct wdb_sym_id		/* host and target symbol ID */
    {
    TGT_INT_T		hostSymbolId;	/* host symbol ID */
    TGT_INT_T		tgtSymbolId;	/* target symbol ID */
    } WDB_SYM_ID;

typedef struct wdb_sym_id_list		/* symbols' ID list */
    {
    UINT32		numSym;		/* number of symbols */
    WDB_SYM_ID *	pSymId;		/* symbol ID array */
    } WDB_SYM_ID_LIST;

typedef struct wdb_sect_info		/* section information */
    {
    struct wdb_sect_info *	next;	/* next section */
    TGT_INT_T		nameLength;	/* section name length */
    WDB_STRING_T	name;		/* section name */
    TGT_INT_T		type;		/* section type */
    TGT_ADDR_T		address;	/* section address */
    TGT_INT_T		size;		/* section size */
    TGT_INT_T		flags;		/* section flags */
    TGT_INT_T		cksum;		/* section checksum */
    } WDB_SECT_INFO;

typedef struct wdb_seg_info		/* segment information */
    {
    TGT_INT_T		type;		/* segment type */
    TGT_ADDR_T		address;	/* segment address */
    TGT_INT_T		size;		/* segment size */
    TGT_INT_T		flags;		/* segment flags */
    } WDB_SEG_INFO;

typedef struct wdb_mod_info		/* module information */
    {
    TGT_INT_T		index;		/* request index */
    TGT_INT_T		nameLength;	/* module name length */
    WDB_STRING_T	name;		/* module name */
    TGT_INT_T		flags;		/* load flags */
    TGT_INT_T		group;		/* group number */
    TGT_INT_T		commTotalSize;	/* memory used by common symbols */
    TGT_INT_T		swapNeeded;	/* TRUE if the headers need byte swap */
    TGT_INT_T		undefSymCount;	/* number of undefined symbols */
    TGT_ADDR_T		ctors;		/* static constructors */
    TGT_ADDR_T		dtors;		/* static constructors */
    BOOL		moreData;	/* more data to come ? */
    WDB_SEG_INFO 	segment[WDB_MOD_NUM_SEGS];	/* segments desc. */
    WDB_SECT_INFO *	section;	/* section list */
    } WDB_MOD_INFO;

typedef struct wdb_sym_mod_desc		/* descriptor used to get sym. info */
    {
    TGT_INT_T		moduleId;	/* module to synchronize */
    TGT_INT_T		symbolId;	/* symbol to synchronize */
    TGT_INT_T		options;	/* options */
    } WDB_SYM_MOD_DESC;

/* command structures */

typedef enum wdb_usr_svc_ctrl_opcode	/* opcode for WDB_USR_SVC_CTRL op. */
    {
    WDB_USR_SVC_ADD	= 0,		/* add a new service on the target */
    WDB_USR_SVC_REMOVE	= 1,		/* remove a service on the target */
    WDB_USR_SVC_FIND	= 2		/* find a service on the target */
    } WDB_USR_SVC_CTRL_OPCODE;

typedef struct wdb_usr_svc_add_info	/* extra field for WDB_USR_SVC_ADD */
    {
    TGT_ADDR_T		func;		/* routine associated with service */
    TGT_ADDR_T		initRtn;	/* init routine for the new service */
    TGT_LONG_T		initRtnArg;	/* argument for init routine */
    } WDB_USR_SVC_ADD_INFO;

typedef struct wdb_usr_svc_rem_info	/* extra field for WDB_USR_SVC_REM */
    {
    TGT_ADDR_T		stopRtn;	/* routine to call to stop the */
    					/* command service */
    TGT_LONG_T		stopRtnArg;	/* argument for stop command service */
    } WDB_USR_SVC_REM_INFO;

typedef struct wdb_usr_svc_find_info	/* extra field for WDB_USR_SVC_FIND */
    {
    TGT_ADDR_T		func;		/* routine associated with service */
    } WDB_USR_SVC_FIND_INFO;

typedef struct wdb_usr_svc_ctrl_info	/* user service control information */
    {
    UINT32		numLongs;	/* depends on opcode */
    TGT_LONG_T		svcId;		/* service Id */
    TGT_LONG_T		opcode;		/* operation to perform */
    union
	{
	WDB_USR_SVC_ADD_INFO	add;	/* specific params to add a service */
	WDB_USR_SVC_REM_INFO	remove;	/* specific params to rem. a service */
	WDB_USR_SVC_FIND_INFO	find;	/* specific params to find a service */
	} extra;
    } WDB_USR_SVC_CTRL_INFO;


typedef struct wdb_ctx_start_info	/* WDB_EVT_INFO for context creation */
    {
    UINT32	numLongs;	/* structure length */
    TGT_LONG_T	createdCtxType;	/* created context */
    TGT_LONG_T	createdArgs [WDB_CTX_MAX_ARGS];
    TGT_LONG_T	creationCtxType; /* creation context */
    TGT_LONG_T	creationArgs [WDB_CTX_MAX_ARGS];
    } WDB_CTX_START_INFO;


typedef struct wdb_ctx_dump_info	/* WDB_EVT_INFO for context dump */
    {
    UINT32	numLongs;	/* structure length */
    TGT_LONG_T	dumpedCtxType;	/* dumped context */
    TGT_LONG_T	ctxArgs [WDB_CTX_MAX_ARGS];
    } WDB_CTX_DUMP_INFO;

typedef enum wdb_tsfs_opcode		/* opcode for TSFS operation */
    {
    WDB_TSFS_OPEN	= 0,		/* open target server file */
    WDB_TSFS_READ	= 1,		/* read target server file */
    WDB_TSFS_WRITE	= 2,		/* write target server file */
    WDB_TSFS_CLOSE	= 3,		/* close target server file */
    WDB_TSFS_IOCTL	= 4,		/* ioctl target server file */
    WDB_TSFS_DELETE	= 5		/* delete target server file */
    } WDB_TSFS_OPCODE;

typedef enum wdb_tsfs_ioctl_req		/* portable TSFS ioctl requests */
    {
    WDB_TSFS_IOCTL_FIOSEEK	= 0,	/* set file pointer */
    WDB_TSFS_IOCTL_FIOWHERE	= 1,	/* get file pointer */
    WDB_TSFS_IOCTL_FIONREAD	= 2,	/* # bytes readable */
    WDB_TSFS_IOCTL_FIOFSTATGET	= 3,	/* fill a struct stat for file */
    WDB_TSFS_IOCTL_FIOREADDIR	= 4,	/* read one directory entry */
    WDB_TSFS_IOCTL_FIORENAME	= 5,	/* rename a file or directory */
    WDB_TSFS_IOCTL_SO_SNDURGB	= 6,	/* send urgent-mode byte over sock */
    WDB_TSFS_IOCTL_SO_SETDEBUG	= 7,	/* set SO_DEBUG for sock */
    WDB_TSFS_IOCTL_SO_GETDEBUG	= 8,	/* get SO_DEBUG for sock */
    WDB_TSFS_IOCTL_SO_SETSNDBUF	= 9,	/* set SO_SNDBUF for sock */
    WDB_TSFS_IOCTL_SO_SETRCVBUF	= 11,	/* set SO_RCVBUF for sock */
    WDB_TSFS_IOCTL_SO_SETDONTROUTE = 13,/* set SO_DONTROUT for sock */
    WDB_TSFS_IOCTL_SO_GETDONTROUTE = 14,/* get SO_DONTROUT for sock */
    WDB_TSFS_IOCTL_SO_SETOOBINLINE = 15,/* set SO_OOBINLINE for sock */
    WDB_TSFS_IOCTL_SO_GETOOBINLINE = 16 /* get SO_OOBINLINE for sock */
    } WDB_TSFS_IOCTL_REQ;

typedef struct wdb_tsfs_generic_info	/* base structure for any TSFS op */
    {
    UINT32		numLongs;	/* depends on opcode */
    TGT_LONG_T		channel;	/* TSFS channel for operation */
    TGT_LONG_T		opcode;		/* TSFS operation to perform */
    TGT_ADDR_T		semId;		/* semaphore to "give" when op done */
    TGT_ADDR_T		pResult;	/* where to store result */
    TGT_ADDR_T		pErrno;		/* where to store errno */
    TGT_ADDR_T		semGiveAddr;	/* semGive address to give semId */
    } WDB_TSFS_GENERIC_INFO;

typedef struct wdb_tsfs_open_info	/* extra fields for TSFS open op */
    {
    TGT_ADDR_T		filename;	/* address of filename string on tgt */
    TGT_LONG_T		fnameLen;	/* length of above */
    TGT_LONG_T		mode;		/* mode to open with */
    TGT_LONG_T		perm;		/* permission bits */
    } WDB_TSFS_OPEN_INFO;

typedef struct wdb_tsfs_rw_info		/* extra fields for TSFS read/write */
    {
    TGT_ADDR_T		buffer;		/* buffer of data to write/read */
    TGT_LONG_T		nBytes;		/* length of above */
    } WDB_TSFS_RW_INFO;

typedef struct wdb_tsfs_ioctl_info	/* extra fields for TSFS ioctl */
    {
    TGT_LONG_T		request;	/* of type WDB_TSFS_IOCTL_REQ */
    TGT_LONG_T		arg;
    TGT_ADDR_T		pExtra1;	/* meaning depends on request */
    TGT_ADDR_T		pExtra2;	/* meaning depends on request */
    } WDB_TSFS_IOCTL_INFO;

typedef struct wdb_tsfs_delete_info	/* extra fields for TSFS delete op */
    {
    TGT_ADDR_T		filename;	/* address of filename string on tgt */
    TGT_LONG_T		fnameLen;	/* length of above */
    } WDB_TSFS_DELETE_INFO;

typedef struct wdb_tsfs_info		/* generic TSFS event */
    {
    WDB_TSFS_GENERIC_INFO	info;	/* generic params for all TSFS ops */
    union
	{
	WDB_TSFS_OPEN_INFO	open;	/* specific params for open() */
	WDB_TSFS_DELETE_INFO	remove;	/* specific params for unlink() */
	WDB_TSFS_RW_INFO	rw;	/* specific params for read/write() */
	WDB_TSFS_IOCTL_INFO	ioctl;	/* specific params for ioctl() */
	} extra;
    } WDB_TSFS_INFO;

/* variable declarations */

#ifndef HOST
extern	UINT16 wdbPort;        /* UDP port number to connect */
#endif	/* HOST */

#ifdef __cplusplus
}
#endif

#endif /* __INCwdbh */
