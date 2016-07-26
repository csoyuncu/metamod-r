#include "precompiled.h"

// Reads metamod.dll and game.dll function export tables and combines theim to
// single table that replaces metamod.dll's original table.
typedef struct sort_names_s {
	unsigned long name;
	unsigned short nameOrdinal;
} sort_names_t;

#define rva_to_va(base, rva) ((unsigned long)base + (unsigned long)rva)			// relative virtual address to virtual address
#define va_to_rva(base, va) ((unsigned long)va - (unsigned long)base)			// virtual address to relative virtual address

// Checks module signatures and return ntheaders pointer for valid module
IMAGE_NT_HEADERS *get_ntheaders(HMODULE module)
{
	union {
		unsigned long mem;
		IMAGE_DOS_HEADER *dos;
		IMAGE_NT_HEADERS *pe;
	} mem;

	// Check if valid dos header
	mem.mem = (unsigned long)module;
	if (IsBadReadPtr(mem.dos, sizeof(*mem.dos)) || mem.dos->e_magic != IMAGE_DOS_SIGNATURE)
		return nullptr;

	// Get and check pe header
	mem.mem = rva_to_va(module, mem.dos->e_lfanew);
	if (IsBadReadPtr(mem.pe, sizeof(*mem.pe)) || mem.pe->Signature != IMAGE_NT_SIGNATURE)
		return nullptr;

	return mem.pe;
}

// Returns export table for valid module
IMAGE_EXPORT_DIRECTORY *get_export_table(HMODULE module)
{
	union {
		unsigned long mem;
		void *pvoid;
		IMAGE_DOS_HEADER *dos;
		IMAGE_NT_HEADERS *pe;
		IMAGE_EXPORT_DIRECTORY *export_dir;
	} mem;

	// check module
	mem.pe = get_ntheaders(module);
	if (!mem.pe)
		return nullptr;

	// check for exports
	if (!mem.pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress)
		return nullptr;

	mem.mem = rva_to_va(module, mem.pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	if (IsBadReadPtr(mem.export_dir, sizeof(*mem.export_dir)))
		return nullptr;

	return mem.export_dir;
}

// Sort function for qsort
int sort_names_list(const sort_names_t *A, const sort_names_t *B)
{
	const char *str_A = (const char *)A->name;
	const char *str_B = (const char *)B->name;

	return Q_strcmp(str_A, str_B);
}

// Combines moduleMM and moduleGame export tables and replaces moduleMM table with new one
int combine_module_export_tables(HMODULE moduleMM, HMODULE moduleGame)
{
	IMAGE_EXPORT_DIRECTORY *exportMM;
	IMAGE_EXPORT_DIRECTORY *exportGame;

	unsigned long newNumberOfFunctions;
	unsigned long newNumberOfNames;
	unsigned long *newFunctions;
	unsigned long *newNames;
	unsigned short *newNameOrdinals;
	sort_names_t *newSort;

	unsigned long i;
	unsigned long u;
	unsigned long funcCount;
	unsigned long nameCount;
	unsigned long listFix;

	// get export tables
	exportMM = get_export_table(moduleMM);
	exportGame = get_export_table(moduleGame);
	if (!exportMM || !exportGame)
	{
		META_ERROR("Couldn't initialize dynamic linkents, exportMM: %i, exportGame: %i.  Exiting...", exportMM, exportGame);
		return 0;
	}

	// setup new export table
	newNumberOfFunctions = exportMM->NumberOfFunctions + exportGame->NumberOfFunctions;
	newNumberOfNames = exportMM->NumberOfNames + exportGame->NumberOfNames;

	// alloc lists
	*(void**)&newFunctions = calloc(1, newNumberOfFunctions * sizeof(*newFunctions));
	*(void**)&newSort = calloc(1, newNumberOfNames * sizeof(*newSort));

	// copy moduleMM to new export
	for (funcCount = 0; funcCount < exportMM->NumberOfFunctions; funcCount++)
		newFunctions[funcCount] = rva_to_va(moduleMM, ((unsigned long*)rva_to_va(moduleMM, exportMM->AddressOfFunctions))[funcCount]);
	for (nameCount = 0; nameCount < exportMM->NumberOfNames; nameCount++)
	{
		//fix name address
		newSort[nameCount].name = rva_to_va(moduleMM, ((unsigned long*)rva_to_va(moduleMM, exportMM->AddressOfNames))[nameCount]);
		//ordinal is index to function list
		newSort[nameCount].nameOrdinal = ((unsigned short *)rva_to_va(moduleMM, exportMM->AddressOfNameOrdinals))[nameCount];
	}

	// copy moduleGame to new export
	for (i = 0; i < exportGame->NumberOfFunctions; i++)
		newFunctions[funcCount + i] = rva_to_va(moduleGame, ((unsigned long*)rva_to_va(moduleGame, exportGame->AddressOfFunctions))[i]);
	for (i = 0, listFix = 0; i < exportGame->NumberOfNames; i++)
	{
		const char *name = (const char *)rva_to_va(moduleGame, ((unsigned long*)rva_to_va(moduleGame, exportGame->AddressOfNames))[i]);
		// check if name already in the list
		for (u = 0; u < nameCount; u++)
		{
			if (!strcasecmp(name, (const char*)newSort[u].name))
			{
				listFix -= 1;
				break;
			}
		}

		// already in the list.. skip
		if (u < nameCount)
			continue;

		newSort[nameCount + i + listFix].name = (unsigned long)name;
		newSort[nameCount + i + listFix].nameOrdinal = (unsigned short)funcCount + ((unsigned short *)rva_to_va(moduleGame, exportGame->AddressOfNameOrdinals))[i];
	}

	// set new number
	newNumberOfNames = nameCount + i + listFix;

	// sort names list
	qsort(newSort, newNumberOfNames, sizeof(*newSort), (int(*)(const void*, const void*))&sort_names_list);

	// make newNames and newNameOrdinals lists (VirtualAlloc so we dont waste heap memory to stuff that isn't freed)
	*(void**)&newNames = VirtualAlloc(0, newNumberOfNames * sizeof(*newNames), MEM_COMMIT, PAGE_READWRITE);
	*(void**)&newNameOrdinals = VirtualAlloc(0, newNumberOfNames * sizeof(*newNameOrdinals), MEM_COMMIT, PAGE_READWRITE);

	for (i = 0; i < newNumberOfNames; i++)
	{
		newNames[i] = newSort[i].name;
		newNameOrdinals[i] = newSort[i].nameOrdinal;
	}

	free(newSort);

	//translate VAs to RVAs
	for (i = 0; i < newNumberOfFunctions; i++)
		newFunctions[i] = va_to_rva(moduleMM, newFunctions[i]);

	for (i = 0; i < newNumberOfNames; i++)
	{
		newNames[i] = va_to_rva(moduleMM, newNames[i]);
		newNameOrdinals[i] = (unsigned short)va_to_rva(moduleMM, newNameOrdinals[i]);
	}

	DWORD OldProtect;
	if (!VirtualProtect(exportMM, sizeof(*exportMM), PAGE_READWRITE, &OldProtect))
	{
		META_ERROR("Couldn't initialize dynamic linkents, VirtualProtect failed: %i.  Exiting...", GetLastError());
		return 0;
	}

	exportMM->Base = 1;
	exportMM->NumberOfFunctions = newNumberOfFunctions;
	exportMM->NumberOfNames = newNumberOfNames;
	*(unsigned long*)&(exportMM->AddressOfFunctions) = va_to_rva(moduleMM, newFunctions);
	*(unsigned long*)&(exportMM->AddressOfNames) = va_to_rva(moduleMM, newNames);
	*(unsigned long*)&(exportMM->AddressOfNameOrdinals) = va_to_rva(moduleMM, newNameOrdinals);

	VirtualProtect(exportMM, sizeof(*exportMM), OldProtect, &OldProtect);
	return 1;
}

int init_linkent_replacement(DLHANDLE moduleMetamod, DLHANDLE moduleGame)
{
	return combine_module_export_tables(moduleMetamod, moduleGame);
}