
#include "fzrefcount.h"

//#define DEBUG_REFCOUNT

#ifdef DEBUG_REFCOUNT
#include <pspdebug.h>
#define printf pspDebugScreenPrintf
#endif

void FZRefCounted::tidy() {
	if (references < 0) {
#ifdef DEBUG_REFCOUNT
		printf("Instance %p: FZRefCounted::tidy() called with references = %d\n", this, references);
#endif
	} else if (references == 0) {
#ifdef DEBUG_REFCOUNT
		printf("Instance %p: delete() call chain begins...\n", this);
#endif
		delete this;
#ifdef DEBUG_REFCOUNT
		printf("Instance %p: delete() call chain ends\n", this);
#endif
	}
}

FZRefCounted::FZRefCounted() : references(1) {
}

FZRefCounted::~FZRefCounted() {
}

void FZRefCounted::retain() {
	++references;
#ifdef DEBUG_REFCOUNT
	printf("Instance %p: retained, now references = %d\n", this, references);
#endif
}

void FZRefCounted::release() {
	--references;
#ifdef DEBUG_REFCOUNT
	printf("Instance %p: released, now references = %d\n", this, references);
#endif
	tidy();
}

