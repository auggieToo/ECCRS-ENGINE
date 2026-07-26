#include "klm.h"
#include <complex.h>
#include <stdlib.h>
#include <string.h>


#define ATOM_INVALID ((atomId)0xFFFFFFFFu)
#define RULE_NOT_FOUND ((u32)0xFFFFFFFFu)
#define RANK_UNASSIGNED ((u32)0xFFFFFFFFu)


static i32 
atomTableInit(atomTable *t, u32 initialCap)
{
    if (initialCap == 0) initialCap = 16;
    
	t->names = malloc(initialCap * sizeof(u8 *));
    t->ids   = malloc(initialCap * sizeof(atomId));
    
	if (!t->names || !t->ids) 
	{
        free(t->names); free(t->ids);
        t->names = NULL; t->ids = NULL;
        t->count = t->capacity = 0;
        return -1;
    }

    t->count = 0;
    t->capacity = initialCap;
    return 0;
}

atomId 
atomLookup(const atomTable *t, const char *name)
{
    for (u32 i = 0; i < t->count; i++)
        if (strcmp((const char *)t->names[i], name) == 0)
            return t->ids[i];
    return ATOM_INVALID;
}


static i32 
atomTableGrow(atomTable *t)
{
    u32 newCap = t->capacity ? t->capacity * 2 : 16;
    u8 **nn = realloc(t->names, newCap * sizeof(u8 *));
    
	if (!nn) return -1;
    
	t->names = nn;
    atomId *ni = realloc(t->ids, newCap * sizeof(atomId));
    
	if (!ni) return -1;
    t->ids = ni;
    t->capacity = newCap;
    return 0;
}


atomId 
atomIntern(atomTable *t, const char *name)
{
    atomId found = atomLookup(t, name);
    if (found != ATOM_INVALID) return found;
 
    if (t->count == t->capacity && atomTableGrow(t) != 0)
        return ATOM_INVALID;
 
    size_t len = strlen(name) + 1;
    u8 *copy = malloc(len);
    if (!copy) return ATOM_INVALID;
    memcpy(copy, name, len);
 
    atomId newId = t->count;          /* ids are dense: 0..count-1 */
    t->names[t->count] = copy;
    t->ids[t->count] = newId;
    t->count++;
    return newId;
}



static void 
atomTableFree(atomTable *t)
{
    if (!t) return;
    for (u32 i = 0; i < t->count; i++)
        free(t->names[i]);
    free(t->names);
    free(t->ids);
    t->names = NULL;
    t->ids = NULL;
    t->count = t->capacity = 0;
}



i32 
kbInit(knowledgeBase *kb, u32 cap)
{
	if(cap == 0) cap = 16; 
	kb->rules = malloc(cap * sizeof(rule));
	
	if(!kb->rule) 
	{
		kb->count = kb->capacity = 0 ; 
		return -1; 
	}
	kb->count = 0 ; 
	kb->capacity = cap; 
	
}
