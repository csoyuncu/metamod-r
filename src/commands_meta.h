#ifndef COMMANDS_META_H
#define COMMANDS_META_H

#include "types_meta.h"			// mBOOL
#include "comp_dep.h"

// Flags to use for meta_cmd_doplug(), to operate on existing plugins; note
// "load" operates on a non-existing plugin thus isn't included here.
typedef enum {
	PC_NULL = 0,
	PC_PAUSE,		// pause the plugin
	PC_UNPAUSE,		// unpause the plugin
	PC_UNLOAD,		// unload the plugin
	PC_RELOAD,		// unload the plugin and load it again
	PC_RETRY,		// retry a failed operation (usually load/attach)
	PC_INFO,		// show all info about the plugin
	PC_CLEAR,		// remove a failed plugin from the list
	PC_FORCE_UNLOAD,	// forcibly unload the plugin
	PC_REQUIRE,		// require that this plugin is loaded/running
} PLUG_CMD;

void DLLINTERNAL meta_register_cmdcvar();

void DLLHIDDEN svr_meta(void); // only hidden because called from outside!

void DLLINTERNAL cmd_meta_usage(void);
void DLLINTERNAL cmd_meta_version(void);
void DLLINTERNAL cmd_meta_gpl(void);

void DLLINTERNAL cmd_meta_game(void);
void DLLINTERNAL cmd_meta_refresh(void);
void DLLINTERNAL cmd_meta_load(void);

void DLLINTERNAL cmd_meta_pluginlist(void);
void DLLINTERNAL cmd_meta_cmdlist(void);
void DLLINTERNAL cmd_meta_cvarlist(void);
void DLLINTERNAL cmd_meta_config(void);

void DLLINTERNAL cmd_doplug(PLUG_CMD pcmd);

void DLLINTERNAL client_meta(edict_t *pEntity);
void DLLINTERNAL client_meta_usage(edict_t *pEntity);
void DLLINTERNAL client_meta_version(edict_t *pEntity);
void DLLINTERNAL client_meta_pluginlist(edict_t *pEntity);
void DLLINTERNAL client_meta_aybabtu(edict_t *pEntity);

#endif /* COMMANDS_META_H */