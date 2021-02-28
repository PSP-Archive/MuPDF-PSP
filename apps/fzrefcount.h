
#ifndef FZREFCOUNT_H
#define FZREFCOUNT_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

/**
 * Simple pure reference counted allocation cycle objects.
 * All the objects inheriting from FZRefCounted must be created by factories.
 * Initial reference count after creation must be always 1. Subclasses must
 * implement creation/destruction behaviour on constructors/destroyers.
 * FZRefCounted::retain() and FZRefCounted::release() are NOT overridable for
 * this reason.
 */
class FZRefCounted {
	int references;
	void tidy();

	protected:
	// Force factory construction
	FZRefCounted();
	// Force destruction by release
	virtual ~FZRefCounted();

	public:
	/**
	 * Increase reference count.
	 */
	void retain();
	/**
	 * Decrease reference count.
	 * It's only when calling this method that the object may get deleted.
	 */
	void release();
};

#endif

