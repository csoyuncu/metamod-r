#ifndef MUTIL_H
#define MUTIL_H

#include "comp_dep.h"
#include "plinfo.h"		// plugin_info_t, etc
#include "sdk_util.h"	// hudtextparms_t, etc

// max buffer size for printed messages
#define MAX_LOGMSG_LEN  1024

// For GetGameInfo:
typedef enum {
	GINFO_NAME = 0,
	GINFO_DESC,
	GINFO_GAMEDIR,
	GINFO_DLL_FULLPATH,
	GINFO_DLL_FILENAME,
	GINFO_REALDLL_FULLPATH,
} ginfo_t;

// Meta Utility Function table type.
typedef struct meta_util_funcs_s {
	void		(*pfnLogConsole)		(plid_t plid, const char *fmt, ...);
	void		(*pfnLogMessage)		(plid_t plid, const char *fmt, ...);
	void		(*pfnLogError)			(plid_t plid, const char *fmt, ...);
	void		(*pfnLogDeveloper)		(plid_t plid, const char *fmt, ...);
	void		(*pfnCenterSay)			(plid_t plid, const char *fmt, ...);
	void		(*pfnCenterSayParms)	(plid_t plid, hudtextparms_t tparms, const char *fmt, ...);
	void		(*pfnCenterSayVarargs)	(plid_t plid, hudtextparms_t tparms, const char *fmt, va_list ap);
	qboolean	(*pfnCallGameEntity)	(plid_t plid, const char *entStr,									entvars_t *pev);
	int			(*pfnGetUserMsgID)		(plid_t plid, const char *msgname, int *size);
	const char *(*pfnGetUserMsgName)	(plid_t plid, int msgid, int *size);
	const char *(*pfnGetPluginPath)		(plid_t plid);
	const char *(*pfnGetGameInfo)		(plid_t plid, ginfo_t tag);
	int (*pfnLoadPlugin)(plid_t plid, const char *cmdline, PLUG_LOADTIME now, void **plugin_handle);
	int (*pfnUnloadPlugin)(plid_t plid, const char *cmdline, PLUG_LOADTIME now, PL_UNLOAD_REASON reason);
	int (*pfnUnloadPluginByHandle)(plid_t plid, void *plugin_handle, PLUG_LOADTIME now, PL_UNLOAD_REASON reason);
	const char *(*pfnIsQueryingClientCvar)	(plid_t plid, const edict_t *player);
	int (*pfnMakeRequestID)	(plid_t plid);
	void            (*pfnGetHookTables)             (plid_t plid, enginefuncs_t **peng, DLL_FUNCTIONS **pdll, NEW_DLL_FUNCTIONS **pnewdll);
} mutil_funcs_t;

extern mutil_funcs_t MetaUtilFunctions DLLHIDDEN;

// Convenience macros for MetaUtil functions
#define LOG_CONSOLE			(*gpMetaUtilFuncs->pfnLogConsole)
#define LOG_MESSAGE			(*gpMetaUtilFuncs->pfnLogMessage)
#define LOG_ERROR			(*gpMetaUtilFuncs->pfnLogError)
#define LOG_DEVELOPER		(*gpMetaUtilFuncs->pfnLogDeveloper)
#define CENTER_SAY			(*gpMetaUtilFuncs->pfnCenterSay)
#define CENTER_SAY_PARMS	(*gpMetaUtilFuncs->pfnCenterSayParms)
#define CENTER_SAY_VARARGS	(*gpMetaUtilFuncs->pfnCenterSayVarargs)
#define CALL_GAME_ENTITY	(*gpMetaUtilFuncs->pfnCallGameEntity)
#define GET_USER_MSG_ID		(*gpMetaUtilFuncs->pfnGetUserMsgID)
#define GET_USER_MSG_NAME	(*gpMetaUtilFuncs->pfnGetUserMsgName)
#define GET_PLUGIN_PATH		(*gpMetaUtilFuncs->pfnGetPluginPath)
#define GET_GAME_INFO		(*gpMetaUtilFuncs->pfnGetGameInfo)
#define LOAD_PLUGIN		(*gpMetaUtilFuncs->pfnLoadPlugin)
#define UNLOAD_PLUGIN		(*gpMetaUtilFuncs->pfnUnloadPlugin)
#define UNLOAD_PLUGIN_BY_HANDLE	(*gpMetaUtilFuncs->pfnUnloadPluginByHandle)
#define IS_QUERYING_CLIENT_CVAR (*gpMetaUtilFuncs->pfnIsQueryingClientCvar)
#define MAKE_REQUESTID		(*gpMetaUtilFuncs->pfnMakeRequestID)
#define GET_HOOK_TABLES         (*gpMetaUtilFuncs->pfnGetHookTables)

#endif /* MUTIL_H */