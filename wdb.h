/* wdb.h - WDB protocol definition header */

/* Copyright 1995-2001 Wind River Systems, Inc. */

/*
modification history
--------------------
01y,20sep01,jhw  Fixed comment about endianness (SPR 5638).
01x,09may01,dtr  Removing hasAltivec and hasFpp to replace with
                 hasCoprocessor.
01w,19mar01,tpw  Resolve SH/altivec conflict; ALTIVEC->AV; add WDB_REG_SET_LAST
		 so a stable name for the count of WDB_REG_SET's is available.
01v,29jan01,dtr  Adding altivec register support.
01v,07mar00,zl   merged SH support from T1
01u,24feb99,dbt  removed useless definition of BOOL type (SPR #25225).
01t,09feb99,dbt  doc: indicate which elements of WDB_CTX_TYPE are implemented.
01s,28jan99,jmp  added semGiveAddr field to WDB_TSFS_GENERIC_INFO structure,
                 for TSFS_BOOT facility (SPR# 24466).
01r,25aug98,cth  added WDB_TSFfffffffffffffffffS_ERRNO_ECONNREFUSED
01q,27may98,cth  removed TSFS_IOCTL_GET_SND/RCVBUF defs
01p,16apr98,cth  added portable errnos for TSFS
01o,03apr98,cjtc i960 port. Added #pragma align 1 for TSFS target side
01n,19mar98,cth  added TSFS ioctl commands for sockets
01m,25feb98,cth  changed values of WDB_TSFS_S_IFDIR/_IFREG, added WDB_TSFS_
		 O_APPEND, O_EXCL, and O_TEXT
01p,04jun98,dbt  merged wdb.h and wdbP.h (SPR #21217)
01o,24mar98,dbt  added WDB_CONTEXT_STATUS_GET service.
01n,10mar98,dbt  added support for user events and context start notification.
01m,12jan98,dbt  added hardware breakpoints support
01l,06dec96,dgp  doc: correct spelling
01k,18nov96,dbt  changed the name of field delete in WDB_TSFS_INFO struct for
                 C++ compatibility, it is now named remove.
01j,12nov96,c_s  add perm member to TSFS open params
01i,21oct96,c_s  add target server file system support
01h,02oct96,elp  changed TGT_ADDR_T and TGT_INT_T from char to UINT32 and
		 int to INT32.
01g,04jul96,p_m  added WDB_TARGET_MODE_GET (SPR# 6200).
01f,01jul96,dgp  doc: API Guide, fix spelling of comment,
                     wdb_ctx_step_desc
01e,10jun95,pad  removed #include <rpc/xdr.h> and moved #include <rpc/types.h>
		 into #else statement.
01d,07jun95,ms   added WDB_ERR_NOT_FOUND 
01c,01jun95,c_s  added some gopher constants
01b,17may95,tpr  added #include <rpc/xdr.h>.
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
All tasks on the target must share a common address space.

INTERFACE

errCode = wtxBeCall (HBackend, procNum, &params, &reply);

DATA TYPES

	UINT32		/@ 32 bit unsigned integer (defined in host.h)  @/
	BOOL		/@ TRUE or FALSE @/
	STATUS		/@ OK or ERROR @/
	TGT_ADDR_T	/@ target address  (defined in host.h) @/
	TGT_INT_T	/@ target integer  (defined in host.h) @/
	WDB_STRING_T	/@ pointer to a string (a local address) @/
	WDB_OPQ_DATA_T	/@ pointer to a block of memory (a local address) @/

Each side must also provide the following XDR filters:

	xdr_UINT32	/@ encode/decode a 32 bit integer @/
*/

/* includes */

#ifdef HOST
#include "host.h"
#include <rpc/rpc.h>
#else
#include "types/vxANSI.h"
#include <rpc/types.h>
#endif

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
#define WDB_MEM_WRITE_MANY		18	/* scatter write */
#define WDB_MEM_WRITE_MANY_INT		19	/* scatter write of ints */

	/* Context Control */

#define	WDB_CONTEXT_CREATE		30	/* create a new context */
#define	WDB_CONTEXT_KILL		31	/* remove a context */
#define	WDB_CONTEXT_SUSPEND		32	/* suspend a context */
#define WDB_CONTEXT_RESUME		33	/* resume a context */
#define WDB_CONTEXT_STATUS_GET		34	/* get the context status */

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

#define WDB_CONTEXT_CONT		80	/* XXX - same as resume? */
#define	WDB_CONTEXT_STEP		81	/* continue a context */

	/* Miscelaneous */

#define	WDB_FUNC_CALL			90	/* spawn a function */
#define WDB_EVALUATE_GOPHER		91	/* evaluate a gopher tape */
#define WDB_DIRECT_CALL			92	/* call a function directly */

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
#define WDB_ERR_MEM_ACCES	 	0x502	/* memory fault */
#define WDB_ERR_AGENT_MODE		0x503	/* wrong agent mode */
#define WDB_ERR_RT_ERROR 		0x504	/* run-time callout failed */
#define WDB_ERR_INVALID_CONTEXT 	0x505	/* bad task ID */
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
#define WDB_ERR_PROC_FAILED		0x5ff	/* generic proc failure */

#define WDB_ERR_NO_CONNECTION		0x600	/* not connected */
#define WDB_ERR_CONNECTION_BUSY		0x601	/* someone else connected */
#define WDB_ERR_COMMUNICATION		0x6ff	/* generic comm error */

/*
 * WDB miscelaneous definitions.
 */

	/* agent modes */

#define WDB_MODE_TASK	1			/* task mode agent */
#define WDB_MODE_EXTERN	2			/* system mode agent */
#define WDB_MODE_DUAL	(WDB_MODE_TASK | WDB_MODE_EXTERN) /* dual mode */
#define WDB_MODE_BI	WDB_MODE_DUAL

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

	/* option bits for task creation */

#define WDB_UNBREAKABLE	0x0002		/* ignore breakpoints */
#define WDB_FP_TASK	0x0008		/* task uses floating point */
#define WDB_FP_RETURN	0x8000		/* return value is a double */

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


	/* context status */
    
#define WDB_CTX_RUNNING		0	/* context is running */
#define WDB_CTX_SUSPENDED	1	/* context is suspended */

/* session information */

#define WDBPORT (u_short)	0x4321		/* UDP port to connect */
#define WDBPROG	(u_long)	0x55555555	/* RPC program number */
#define WDBVERS	(u_long)	1		/* RPC version number */
#define WDB_VERSION_STR		"2.0"

/* message cores */

#define WDB_EVENT_NOTIFY	0x8000		/* notify bit in errCode */
#define WDB_TO_BE_CONTINUED	0x4000		/* continue bit in errCode */
#define WDB_HOST_ID_MASK	0xffff0000	/* hostId in seqNum */

/*
 * WDB primitive data types.
 */

#ifndef	HOST
typedef UINT32  TGT_ADDR_T;
typedef INT32	TGT_INT_T;
#endif

typedef char *	WDB_STRING_T;
typedef char *	WDB_OPQ_DATA_T;

/*
 * WDB compound data types.
 */

typedef struct wdb_mem_region		/* a region of target memory */
    {
    TGT_ADDR_T		baseAddr;	/* memory region base address */
    TGT_INT_T		numBytes;	/* memory region size */
    UINT32		param;		/* proc dependent parameter */
    } WDB_MEM_REGION;

typedef struct wdb_mem_xfer		/* transfer a block of memory */
    {
    WDB_OPQ_DATA_T	source;		/* data to transfer */
    TGT_ADDR_T		destination;	/* requested destination */
    TGT_INT_T		numBytes;	/* number of bytes transferred */
    } WDB_MEM_XFER;

typedef struct wdb_mem_scan_desc
    {
    WDB_MEM_REGION	memRegion;	/* region of memory to scan */
    WDB_MEM_XFER	memXfer;	/* pattern to scan for */
    } WDB_MEM_SCAN_DESC;

typedef enum wdb_ctx_type		/* type of context on the target */
    {
    WDB_CTX_SYSTEM	= 0,		/* system mode */
    WDB_CTX_GROUP	= 1,		/* process group (not implemented) */
    WDB_CTX_ANY		= 2,		/* any context (not implemented) */
    WDB_CTX_TASK	= 3,		/* specific task or processes */
    WDB_CTX_ANY_TASK	= 4,		/* any task */
    WDB_CTX_ISR		= 5,		/* specific ISR (not implemented) */
    WDB_CTX_ANY_ISR	= 6		/* any ISR (not implemented) */
    } WDB_CTX_TYPE;

typedef struct wdb_ctx			/* a particular context */
    {
    WDB_CTX_TYPE	contextType;	/* type of context */
    UINT32		contextId;	/* context ID */
    } WDB_CTX;

typedef struct wdb_ctx_step_desc	/* how to single step a context */
    {
    WDB_CTX		context;	/* context to step */
    TGT_ADDR_T		startAddr;	/* lower bound of step range */
    TGT_ADDR_T 		endAddr;	/* upper bound of step range */
    } WDB_CTX_STEP_DESC;

typedef struct wdb_ctx_create_desc	/* how to create a context */
    {
    WDB_CTX_TYPE	contextType;	/* task or system context */

    /* the following are used for task and system contexts */

    TGT_ADDR_T		stackBase;	/* bottom of stack (NULL = malloc) */
    UINT32		stackSize;	/* stack size */
    TGT_ADDR_T 		entry;		/* context entry point */
    TGT_INT_T		args[10];	/* arguments */

    /* the following are only used for task contexts */

    WDB_STRING_T	name;		/* name */
    TGT_INT_T		priority;	/* priority */
    TGT_INT_T		options;	/* options */
    TGT_INT_T		redirIn;	/* redirect input file (or 0) */
    TGT_INT_T		redirOut;	/* redirect output file (or 0) */
    TGT_INT_T		redirErr;	/* redirect error output file (or 0) */
    } WDB_CTX_CREATE_DESC;

typedef enum wdb_reg_set_type		/* a type of register set */
    {
    WDB_REG_SET_IU	= 0,            /* integer unit register set */
    WDB_REG_SET_FPU	= 1,            /* floating point unit register set */
    WDB_REG_SET_MMU	= 2,            /* memory management unit reg set */
    WDB_REG_SET_CU	= 3,            /* cache unit register set */
    WDB_REG_SET_TPU	= 4,            /* timer processor unit register set */
    WDB_REG_SET_SYS	= 5,		/* system registers */
    WDB_REG_SET_DSP	= 6,		/* dsp registers */
    WDB_REG_SET_AV	= 7,		/* altivec registers */
    WDB_REG_SET_LAST	= 8		/* 1 + last used REG_SET number */
    } WDB_REG_SET_TYPE;

typedef struct wdb_reg_read_desc	/* register data to read */
    {
    WDB_REG_SET_TYPE regSetType;	/* type of register set to read */
    WDB_CTX	     context;		/* context associated with registers */
    WDB_MEM_REGION   memRegion;		/* subregion of the register block */
    } WDB_REG_READ_DESC;

typedef struct wdb_reg_write_desc	/* register data to write */
    {
    WDB_REG_SET_TYPE regSetType;	/* type of register set to write */
    WDB_CTX	     context;		/* context associated with registers */
    WDB_MEM_XFER     memXfer;		/* new value of the register set */
    } WDB_REG_WRITE_DESC;

typedef enum wdb_rt_type		/* type of run-time system */
    {
    WDB_RT_NULL		= 0,		/* standalone WDB agent */
    WDB_RT_VXWORKS	= 1		/* WDB agent integrated in VxWorks */
    } WDB_RT_TYPE;
/* These are used to bit shift result for wdbTgtHasCoprocessors */
typedef enum wdb_coprocessor_type
    {
    WDB_CO_PROC_FPP	= 0,
    WDB_CO_PROC_ALTIVEC = 1,
    WDB_CO_PROC_DSP	= 2,
    WDB_CO_PROC_LAST	= 16  /* DON'T USE 15BIT SHIFT MAX */
    } WDB_COPROCESSOR_TYPE;

typedef struct wdb_rt_info		/* info on the run-time system */
    {
    WDB_RT_TYPE		rtType;		/* runtime type */
    WDB_STRING_T	rtVersion;	/* run time version */
    TGT_INT_T		cpuType;	/* target processor type */
    UINT32		hasCoprocessor;	/* target has a floating point unit */
    BOOL		hasWriteProtect; /* target can write protect memory */
    TGT_INT_T		pageSize;	/* size of a page */
    TGT_INT_T		endian;		/* endianness (4321 or 1234) */
    WDB_STRING_T	bspName;	/* board support package name */
    WDB_STRING_T	bootline;	/* boot file path or NULL if embedded */
    TGT_ADDR_T		memBase;	/* target main memory base address */
    UINT32		memSize;	/* target main memory size */
    TGT_INT_T		numRegions;	/* number of memory regions */
    WDB_MEM_REGION *	memRegion;	/* memory region descriptor(s) */
    TGT_ADDR_T		hostPoolBase;	/* host-controlled tgt memory pool */
    UINT32		hostPoolSize;	/* host-controlled memory pool size */
    } WDB_RT_INFO;

typedef struct wdb_agent_info		/* info on the debug agent */
    {
    WDB_STRING_T	agentVersion;	/* version of the WDB agent */
    TGT_INT_T		mtu;		/* maximum transfer size in bytes */
    TGT_INT_T		mode;		/* available agent modes */
    } WDB_AGENT_INFO;

typedef struct wdb_tgt_info		/* info on the target */
    {
    WDB_AGENT_INFO  	agentInfo;	/* info on the agent */
    WDB_RT_INFO	    	rtInfo;		/* info on the run time system */
    } WDB_TGT_INFO;

typedef enum wdb_evt_type		/* type of event on the target */
    {
    WDB_EVT_NONE	= 0,		/* no event */
    WDB_EVT_CTX_START	= 1,		/* context creation */
    WDB_EVT_CTX_EXIT	= 2,		/* context exit */
    WDB_EVT_BP		= 3,		/* breakpoint */
    WDB_EVT_HW_BP	= 4,		/* hardware breakpoint */
    WDB_EVT_WP		= 5,		/* watchpoint */
    WDB_EVT_EXC		= 6,		/* exception */
    WDB_EVT_VIO_WRITE	= 7,		/* virtual I/O write */
    WDB_EVT_CALL_RET	= 8,		/* function call finished */
    WDB_EVT_TSFS_OP	= 9,		/* VIO II operation */
    WDB_EVT_USER	= 10		/* user event */
    } WDB_EVT_TYPE;

typedef enum wdb_action_type		/* what to do when an event occurs */
    {
    WDB_ACTION_CALL	= 1,		/* condition the evtpt via a proc */
    WDB_ACTION_NOTIFY	= 2,		/* notify the host */
    WDB_ACTION_STOP	= 4		/* stop the context */
    } WDB_ACTION_TYPE;

typedef struct wdb_action		/* a specific action */
    {
    WDB_ACTION_TYPE	actionType;
    UINT32		actionArg;
    TGT_ADDR_T		callRtn;
    TGT_INT_T		callArg;
    } WDB_ACTION;

typedef struct wdb_evtpt_add_desc	/* how to add an eventpt */
    {
    WDB_EVT_TYPE	evtType;	/* type of event to detect */
    TGT_INT_T		numArgs;	/* eventType dependent arguments */
    UINT32  *  		args;		/* arg list */
    WDB_CTX		context;	/* context in which event must occur */
    WDB_ACTION 		action;		/* action to perform */
    } WDB_EVTPT_ADD_DESC;

typedef struct wdb_evtpt_del_desc	/* how to delete an eventpoint */
    {
    WDB_EVT_TYPE 	evtType;	/* type of event */
    TGT_ADDR_T	 	evtptId;	/* eventpoint ID */
    } WDB_EVTPT_DEL_DESC;

typedef struct wdb_evt_info		/* event info for anything but VIO */
    {
    TGT_INT_T		numInts;	/* number of ints of event data */
    UINT32 		info [WDB_MAX_EVT_DATA]; /* event data */
    } WDB_EVT_INFO;

typedef enum wdb_call_ret_type		/* type of return value */
    {
    WDB_CALL_RET_INT	= 0,
    WDB_CALL_RET_DBL	= 1
    } WDB_CALL_RET_TYPE;

typedef struct wdb_call_return_info
    {
    UINT32		callId;		/* returned from WDB_FUNC_CALL */
    WDB_CALL_RET_TYPE	returnType;	/* return type */
    union
	{
	double		returnValDbl;	/* double return value */
	TGT_INT_T	returnValInt;	/* integer return value */
	} returnVal;
    TGT_INT_T		errnoVal;	/* error status */
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

typedef struct wdb_exc_info		/* WDB_EVT_INFO for exceptions */
    {
    TGT_INT_T		numInts;	/* 4 */
    WDB_CTX		context;	/* context that incurred the exception */
    TGT_INT_T		vec;		/* hardware trap number */
    TGT_ADDR_T		pEsf;		/* address of the exception stack frame */
    } WDB_EXC_INFO;

typedef struct wdb_bp_info		/* WDB_EVT_INFO for breakpoints */
    {
    TGT_INT_T		numInts;	/* 5 */
    WDB_CTX		context;	/* context when the breakpoint */
					/* was hitten */
    TGT_ADDR_T		pc;		/* program counter */
    TGT_ADDR_T		fp;		/* frame pointer */
    TGT_ADDR_T		sp;		/* stack pointer */
    TGT_ADDR_T		addr;		/* breakpoint address */
    } WDB_BP_INFO;

typedef struct wdb_ctx_exit_info	/* WDB_EVT_INFO for context exit */
    {
    TGT_INT_T		numInts;	/* 4 */
    WDB_CTX		context;	/* context that exited */
    TGT_INT_T		returnVal;	/* context's return value/exit code */
    TGT_INT_T		errnoVal;	/* context's error status */
    } WDB_CTX_EXIT_INFO;


/*
 * #pragma align 1 for i960 will prevent gcc960 from aligning all structures
 * on a 16 byte boundary.
 */

#ifndef HOST
#if (defined (CPU_FAMILY) && (CPU_FAMILY==I960) && (defined __GNUC__))
#pragma align 1                 /* tell gcc960 not to optimize alignments */
#endif  /* CPU_FAMILY==I960 */
#endif  /* HOST */

typedef struct wdb_ctx_start_info	/* WDB_EVT_INFO for context creation */
    {
    TGT_INT_T		numInts;	/* 4 */
    WDB_CTX		createdCtx;	/* created context */
    WDB_CTX		creationCtx;	/* creation context */
    } WDB_CTX_START_INFO;

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
    TGT_INT_T		numInts;	/* depends on opcode */
    TGT_INT_T		channel;	/* TSFS channel for operation */
    TGT_INT_T		opcode;		/* TSFS operation to perform */
    TGT_ADDR_T		semId;		/* semaphore to "give" when op done */
    TGT_ADDR_T		pResult;	/* where to store result */
    TGT_ADDR_T		pErrno;		/* where to store errno */
    TGT_ADDR_T		semGiveAddr;	/* semGive address to give semId */
    } WDB_TSFS_GENERIC_INFO;

typedef struct wdb_tsfs_open_info	/* extra fields for TSFS open op */
    {
    TGT_ADDR_T		filename;	/* address of filename string on tgt */
    TGT_INT_T		fnameLen;	/* length of above */
    TGT_INT_T		mode;		/* mode to open with */
    TGT_INT_T		perm;		/* permission bits */
    } WDB_TSFS_OPEN_INFO;

typedef struct wdb_tsfs_rw_info		/* extra fields for TSFS read/write */
    {
    TGT_ADDR_T		buffer;		/* buffer of data to write/read */
    TGT_INT_T		nBytes;		/* length of above */
    } WDB_TSFS_RW_INFO;

typedef struct wdb_tsfs_ioctl_info	/* extra fields for TSFS ioctl */
    {
    WDB_TSFS_IOCTL_REQ	request;
    TGT_INT_T		arg;
    TGT_ADDR_T		pExtra1;	/* meaning depends on request */
    TGT_ADDR_T		pExtra2;	/* meaning depends on request */
    } WDB_TSFS_IOCTL_INFO;

typedef struct wdb_tsfs_delete_info	/* extra fields for TSFS delete op */
    {
    TGT_ADDR_T		filename;	/* address of filename string on tgt */
    TGT_INT_T		fnameLen;	/* length of above */
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

#ifndef HOST
#if (defined (CPU_FAMILY) && (CPU_FAMILY==I960) && (defined __GNUC__))
#pragma align 0				/* turn off alignment requirement */
#endif  /* CPU_FAMILY==I960 */
#endif  /* HOST */

typedef struct wdb_param_wrapper
    {
    void *	pParams;	/* real parameters */
    xdrproc_t	xdr;		/* XDR filter for the real params */
    UINT32	seqNum;		/* sequence number */
    } WDB_PARAM_WRAPPER;

typedef struct wdb_reply_wrapper
    {
    void *	pReply;		/* real reply */
    xdrproc_t	xdr;		/* XDR filter for the real reply */
    UINT32	errCode;	/* error status */
    } WDB_REPLY_WRAPPER;


/* function prototypes */

#if defined(__STDC__) || defined(__cplusplus) || defined(WIN32_COMPILER)

extern BOOL	xdr_UINT32		(XDR *xdrs, UINT32 *objp);
extern BOOL	xdr_TGT_ADDR_T		(XDR *xdrs, TGT_ADDR_T *objp);
extern BOOL	xdr_TGT_INT_T		(XDR *xdrs, TGT_INT_T *objp);
extern BOOL	xdr_WDB_STRING_T	(XDR *xdrs, WDB_STRING_T *objp);
extern BOOL	xdr_WDB_OPQ_DATA_T	(XDR *xdrs, WDB_OPQ_DATA_T *objp,
					 UINT32 len);
extern BOOL	xdr_ARRAY		(XDR *xdrs, char **, TGT_INT_T *,
					 TGT_INT_T, TGT_INT_T,
					 xdrproc_t elproc);
extern BOOL	xdr_CHECKSUM		(XDR *xdrs, UINT32 xdrCksumVal,
					 UINT32 xdrStreamSize,
					 UINT32 xdrCksumValPos,
					 UINT32 xdrStreamSizePos);

extern BOOL	xdr_WDB_PARAM_WRAPPER	(XDR *xdrs, WDB_PARAM_WRAPPER *objp);
extern BOOL	xdr_WDB_REPLY_WRAPPER	(XDR *xdrs, WDB_REPLY_WRAPPER *objp);
extern BOOL	xdr_WDB_TGT_INFO 	(XDR *xdrs, WDB_TGT_INFO *objp);
extern BOOL	xdr_WDB_MEM_REGION	(XDR *xdrs, WDB_MEM_REGION *objp);
extern BOOL	xdr_WDB_MEM_XFER	(XDR *xdrs, WDB_MEM_XFER *objp);
extern BOOL	xdr_WDB_MEM_SCAN_DESC	(XDR *xdrs, WDB_MEM_SCAN_DESC *objp);
extern BOOL	xdr_WDB_CTX		(XDR *xdrs, WDB_CTX *objp);
extern BOOL	xdr_WDB_CTX_CREATE_DESC	(XDR *xdrs, WDB_CTX_CREATE_DESC *objp);
extern BOOL	xdr_WDB_CTX_STEP_DESC	(XDR *xdrs, WDB_CTX_STEP_DESC *objp);
extern BOOL	xdr_WDB_REG_READ_DESC	(XDR *xdrs, WDB_REG_READ_DESC *objp);
extern BOOL	xdr_WDB_REG_WRITE_DESC	(XDR *xdrs, WDB_REG_WRITE_DESC *objp);
extern BOOL	xdr_WDB_EVTPT_ADD_DESC	(XDR *xdrs, WDB_EVTPT_ADD_DESC *objp);
extern BOOL	xdr_WDB_EVTPT_DEL_DESC	(XDR *xdrs, WDB_EVTPT_DEL_DESC *objp);
extern BOOL	xdr_WDB_EVT_DATA	(XDR *xdrs, WDB_EVT_DATA *objp);

#else	/* __STDC__ */

extern BOOL	xdr_UINT32		();
extern BOOL	xdr_BOOL		();
extern BOOL	xdr_TGT_ADDR_T		();
extern BOOL	xdr_TGT_INT_T		();
extern BOOL	xdr_WDB_STRING_T	();
extern BOOL	xdr_WDB_OPQ_DATA_T	();
extern BOOL	xdr_ARRAY		();
extern BOOL	xdr_CHECKSUM		();

extern BOOL	xdr_PARAM_WRAPPER	();
extern BOOL	xdr_REPLY_WRAPPER	();
extern BOOL	xdr_WDB_TGT_INFO 	();
extern BOOL	xdr_WDB_MEM_REGION	();
extern BOOL	xdr_WDB_MEM_XFER	();
extern BOOL	xdr_WDB_MEM_SCAN_DESC	();
extern BOOL	xdr_WDB_CTX		();
extern BOOL	xdr_WDB_CTX_CREATE_DESC	();
extern BOOL	xdr_WDB_CTX_STEP_DESC	();
extern BOOL	xdr_WDB_REG_READ_DESC	();
extern BOOL	xdr_WDB_REG_WRITE_DESC	();
extern BOOL	xdr_WDB_EVTPT_ADD_DESC	();
extern BOOL	xdr_WDB_EVTPT_DEL_DESC	();
extern BOOL	xdr_WDB_EVT_DATA	();

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif /* __INCwdbh */

