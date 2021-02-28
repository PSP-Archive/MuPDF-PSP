
#ifndef FZINPUTSTREAM_H
#define FZINPUTSTREAM_H

#include "fzrefcount.h"

/**
 * Base interface for input streams.
 */
class FZInputStream : public FZRefCounted {
protected:
	FZInputStream() { }
	virtual ~FZInputStream() { }
public:
	/**
	 * Check for end of stream.
	 */
	virtual bool eos() = 0;
	/**
	 * Get a byte from the stream.
	 */
	virtual char get() = 0;
	/**
	 * Get a block of bytes byte from the stream.
	 * Returns the number of bytes read.
	 */
	virtual int getBlock(char* where, int size) = 0;
};

#endif

