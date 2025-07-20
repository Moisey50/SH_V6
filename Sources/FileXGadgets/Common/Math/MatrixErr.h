#ifndef _MATRIX_ERROR_MACROS_INC
#define _MATRIX_ERROR_MACROS_INC

#define MXERR_INVALID_SIZE			"\nMATRIX ERROR: Matrex sizes not proper for the operation\n"
#define MXERR_OUT_OF_RANGE			"\nMATRIX ERROR: Attempt to access element out of matrix ranges\n"
#define	MX_ASSERT(cond, error)		{ if (!cond) TRACE(error); ASSERT(cond); }
#define MX_BREAK(cond, error, EXIT)	{ if (!cond) { TRACE(error); ASSERT(FALSE); EXIT; } }


#endif