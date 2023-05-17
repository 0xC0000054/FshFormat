/* -----------------------------------------------------------------------------

	Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the 
	"Software"), to	deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to 
	permit persons to whom the Software is furnished to do so, subject to 
	the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	
   -------------------------------------------------------------------------- */

#include "squish.h"
#include "squishinterface.h"

extern "C"
{
	void SquishInitialize( void )
	{
		// This function doesn't do anything. It exists solely to ensure that the Squish DLL is loaded
		// and mapped into memory at an earlier point in time. That way the error checking can be
		// a bit simpler for something like the DDS Paint.NET plugin.
	}

	void SquishCompressImage( char* rgba, int width, int height, void* blocks, int flags)
	{
		squish::CompressImage( ( const squish::u8* )rgba, width, height, blocks, flags);
	}

	void SquishDecompressImage( char* rgba, int width, int height, void* blocks, int flags)
	{
		squish::DecompressImage( ( squish::u8* ) rgba, width, height, ( void const* )blocks, flags);
	}
};