/*
 * Runtime.h
 *
 *  Created on: Apr 10, 2018
 *      Author: Ignacio Laguna, ilaguna@llnl.gov
 */

#ifndef SRC_RUNTIME_H_
#define SRC_RUNTIME_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <limits.h>

/// *** Warning ***
/// Changing this file: Runtime.h
/// When a global __device__ function is added in the this file, some actions
/// must be taken:
/// (1) We need to find this function when initializing Instrumentation pass.
/// (2) Linkage of the function must be set to LinkOnceODRLinkage.
/// (3) We need to add this function to the list of unwanted functions, i.e.,
///     functions we do not instrument in the pass.

#ifdef __CUDA_ARCH__
/// Symbol used to determine whether we are compiling device code or not
__device__ void _FPC_DEVICE_CODE_FUNC_(){};
#endif

__device__ static void	_FPC_INTERRUPT_(int loc);
__device__ static int	_FPC_FP32_IS_SUBNORMAL(float x);
__device__ static int 	_FPC_FP32_IS_ALMOST_OVERFLOW(float x);
__device__ static int 	_FPC_FP32_IS_ALMOST_SUBNORMAL(float x);
__device__ void 	_FPC_PRINT_ERRORS_();
__device__ void 	_FPC_FP32_CHECK_ADD_(float x, float y, float z, int loc);
__device__ void 	_FPC_FP32_CHECK_SUB_(float x, float y, float z, int loc);
__device__ void 	_FPC_FP32_CHECK_MUL_(float x, float y, float z, int loc);
__device__ void 	_FPC_FP32_CHECK_DIV_(float x, float y, float z, int loc);
__device__ static int	_FPC_FP64_IS_SUBNORMAL(double x);
__device__ static int 	_FPC_FP64_IS_ALMOST_OVERFLOW(double x);
__device__ static int 	_FPC_FP64_IS_ALMOST_SUBNORMAL(double x);
__device__ void 	_FPC_FP64_CHECK_ADD_(float x, float y, float z, int loc);
__device__ void 	_FPC_FP64_CHECK_SUB_(float x, float y, float z, int loc);
__device__ void 	_FPC_FP64_CHECK_MUL_(float x, float y, float z, int loc);
__device__ void 	_FPC_FP64_CHECK_DIV_(float x, float y, float z, int loc);

void _FPC_PRINT_AT_MAIN_();

#define REPORT_LINE_SIZE 80
#define REPORT_COL1_SIZE 15
#define REPORT_COL2_SIZE REPORT_LINE_SIZE-REPORT_COL1_SIZE-1

#ifdef FPC_DANGER_ZONE_PERCENT
#define DANGER_ZONE_PERCENTAGE FPC_DANGER_ZONE_PERCENT
#else
#define DANGER_ZONE_PERCENTAGE 0.10
#endif

/* ----------------------------- Global Data ------------------------------- */

/// We store the file name and directory in this variable
//char *_FPC_LOCATIONS_TABLE_[100];// = {"NONE1"};
__device__ static char *_FPC_FILE_NAME_[1];

/// Lock to print from one thread only
__device__ static int lock_state = 0;

__device__ static int errors_array_size = 10;
__device__ static int errors_per_line_array[10]; // not used in practice

/* ------------------------ Generic Functions ------------------------------ */

/// Report format
/// +---------------- FPChecker Error Report ------------------+
///  Error     : NaN
///  Operation : DIV
///  File      : /usr/file/pathForceCalculation.cc
///  Line      : 23
///  Thread ID : 1024
/// +----------------------------------------------------------+

__device__
static int _FPC_LEN_(const char *s)
{
	int maxLen = 1024; // to check correctness and avid infinite loop
	int i = 0;
	while(s[i] != '\0' && i < maxLen)
		i++;
	return i;
}

__device__
static void _FPC_CPY_(char *d, const char *s)
{
	int len = _FPC_LEN_(s);
	int i=0;
	for (i=0; i < len; ++i)
		d[i] = s[i];
	d[i] = '\0';
}

__device__
static void _FPC_CAT_(char *d, const char *s)
{
	int lenS = _FPC_LEN_(s);
	int lenD = _FPC_LEN_(d);
	int i=0;
	for (i=0; i < lenS; ++i)
		d[i+lenD] = s[i];
	d[i+lenD] = '\0';
}

__device__
static void _FPC_PRINT_REPORT_LINE_(const char border)
{
	printf("%c",border);
	for (int i=0; i < REPORT_LINE_SIZE-2; ++i)
		printf("-");
	printf("%c\n",border);
}

__device__
static void _FPC_PRINT_REPORT_HEADER_(int type)
{
	//_FPC_PRINT_REPORT_LINE_('.');

	char msg[255];
	msg[0] = '\0';
	if (type == 0)
		_FPC_CPY_(msg," FPChecker Error Report ");
	else
		_FPC_CPY_(msg," FPChecker Warning Report ");

	int l = _FPC_LEN_(msg);
	l = REPORT_LINE_SIZE-l-2;
	char line[255];
	line[0] = '\0';
	_FPC_CAT_(line,"+");
	for (int i=0; i < l/2; ++i)
		_FPC_CAT_(line,"-");
	if (l%2)
		_FPC_CAT_(line,"-");
	_FPC_CAT_(line,msg);
	for (int i=0; i < l/2; ++i)
		_FPC_CAT_(line,"-");
	_FPC_CAT_(line,"+");
	printf("%s\n",line);
}

__device__
static void _FPC_PRINT_REPORT_ROW_(const char *val, int space, int last, char lastChar)
{
	char msg[255];
	msg[0] = '\0';
	_FPC_CPY_(msg," ");
	_FPC_CAT_(msg, val);
	int rem = _FPC_LEN_(msg);
	for (int i=0; i < space-rem; ++i)
		_FPC_CAT_(msg," ");
	printf("%s",msg);

	if (last==0)
		printf("%c", lastChar);
	else
		printf("\n");
}

__device__
static void _FPC_PRINT_REPORT_ROW_(int val, int space, int last)
{
	int numChars = floor(log10 (abs (val))) + 1;
	printf(" %d", val);

	char msg[255];
	msg[0] = '\0';
	int rem = numChars + 1;
	for (int i=0; i < space-rem; ++i)
		_FPC_CAT_(msg," ");
	printf("%s",msg);

	if (last==0)
		printf(":");
	else
		printf("\n");
}

__device__
static void _FPC_PRINT_REPORT_ROW_(float val, int space, int last)
{
	int numChars = 18;
	printf("(%1.9e)", val);

	char msg[255];
	msg[0] = '\0';
	int rem = numChars + 1;
	for (int i=0; i < space-rem; ++i)
		_FPC_CAT_(msg," ");
	printf("%s",msg);

	//if (last==0)
	//	printf("");
	//else
	if (last!=0)
		printf("\n");
}

__device__
static void _FPC_PRINT_REPORT_ROW_(double val, int space, int last)
{
	int numChars = 18;
	printf("(%1.9e)", val);

	char msg[255];
	msg[0] = '\0';
	int rem = numChars + 1;
	for (int i=0; i < space-rem; ++i)
		_FPC_CAT_(msg," ");
	printf("%s",msg);

	//if (last==0)
	//	printf("");
	//else
	if (last!=0)
		printf("\n");
}

/// errorType: 0:NaN, 1:INF, 2:Underflow
/// op: 0:ADD, 1:SUB, 2:MUL, 3:DIV
__device__
__attribute__((noinline))  static void _FPC_INTERRUPT_(int errorType, int op, int loc, float fp32_val, double fp64_val)
{
/*#ifdef FPC_ERRORS_DONT_ABORT
	volatile int x=3;
	while(x != 0)
		x--;
	x = errorType + op + loc + (int)fp32_val + (int)fp64_val;
	asm ("");
#else*/

#ifdef FPC_ERRORS_DONT_ABORT
	volatile bool blocked = false;
#else
	bool blocked = true;
#endif
	while(blocked) {
			if(0 == atomicCAS(&lock_state, 0, 1)) {

				char e[64]; e[0] = '\0';
				char o[64]; o[0] = '\0';

				if 			(errorType == 0) _FPC_CPY_(e, "NaN");
				else if	(errorType == 1) _FPC_CPY_(e, "INF");
				else if	(errorType == 2) _FPC_CPY_(e, "Underflow");
				else _FPC_CPY_(e, "NONE");

				if 			(op == 0) _FPC_CPY_(o, "ADD");
				else if	(op == 1) _FPC_CPY_(o, "SUB");
				else if	(op == 2) _FPC_CPY_(o, "MUL");
				else if	(op == 3) _FPC_CPY_(o, "DIV");
				else _FPC_CPY_(o, "NONE");

				_FPC_PRINT_REPORT_HEADER_(0);
				_FPC_PRINT_REPORT_ROW_("Error", REPORT_COL1_SIZE, 0, ':');
				_FPC_PRINT_REPORT_ROW_(e, REPORT_COL2_SIZE, 1, ' ');
				_FPC_PRINT_REPORT_ROW_("Operation", REPORT_COL1_SIZE, 0, ':');
				if (errorType == 2)
				{
					_FPC_PRINT_REPORT_ROW_(o, 4, 0, ' ');
					if (fp32_val != 0)
						_FPC_PRINT_REPORT_ROW_(fp32_val, REPORT_COL2_SIZE, 1);
					else
						_FPC_PRINT_REPORT_ROW_(fp64_val, REPORT_COL2_SIZE, 1);
				}
				else
				{
					_FPC_PRINT_REPORT_ROW_(o, REPORT_COL2_SIZE, 1, ' ');
				}
				_FPC_PRINT_REPORT_ROW_("File", REPORT_COL1_SIZE, 0, ':');
				_FPC_PRINT_REPORT_ROW_(_FPC_FILE_NAME_[0], REPORT_COL2_SIZE, 1, ' ');
				_FPC_PRINT_REPORT_ROW_("Line", REPORT_COL1_SIZE, 0, ':');
				//_FPC_PRINT_REPORT_ROW_(l, REPORT_COL2_SIZE, 1);
				_FPC_PRINT_REPORT_ROW_(loc, REPORT_COL2_SIZE, 1);
				_FPC_PRINT_REPORT_LINE_('+');

				asm("trap;");
		}
	}
}

__device__
static void _FPC_WARNING_(int errorType, int op, int loc, float fp32_val, double fp64_val)
{
#ifdef FPC_ERRORS_DONT_ABORT
	volatile bool blocked = false;
#else
	bool blocked = true;
#endif
  	while(blocked) {
			if(0 == atomicCAS(&lock_state, 0, 1)) {

				char e[64]; e[0] = '\0';
				char o[64]; o[0] = '\0';

				if 			(errorType == 0) _FPC_CPY_(e, "NaN");
				else if	(errorType == 1) _FPC_CPY_(e, "INF");
				else if	(errorType == 2) _FPC_CPY_(e, "Underflow");
				else _FPC_CPY_(e, "NONE");

				if 			(op == 0) _FPC_CPY_(o, "ADD");
				else if	(op == 1) _FPC_CPY_(o, "SUB");
				else if	(op == 2) _FPC_CPY_(o, "MUL");
				else if	(op == 3) _FPC_CPY_(o, "DIV");
				else _FPC_CPY_(o, "NONE");

				_FPC_PRINT_REPORT_HEADER_(1);
				_FPC_PRINT_REPORT_ROW_("Error", REPORT_COL1_SIZE, 0, ':');
				_FPC_PRINT_REPORT_ROW_(e, REPORT_COL2_SIZE, 1, ' ');
				_FPC_PRINT_REPORT_ROW_("Operation", REPORT_COL1_SIZE, 0, ':');
				if (errorType == 1 || errorType == 2)
				{
					_FPC_PRINT_REPORT_ROW_(o, 4, 0, ' ');
					if (fp32_val != 0)
						_FPC_PRINT_REPORT_ROW_(fp32_val, REPORT_COL2_SIZE, 1);
					else
						_FPC_PRINT_REPORT_ROW_(fp64_val, REPORT_COL2_SIZE, 1);
				}
				else
				{
					_FPC_PRINT_REPORT_ROW_(o, REPORT_COL2_SIZE, 1, ' ');
				}
				_FPC_PRINT_REPORT_ROW_("File", REPORT_COL1_SIZE, 0, ':');
				_FPC_PRINT_REPORT_ROW_(_FPC_FILE_NAME_[0], REPORT_COL2_SIZE, 1, ' ');
				_FPC_PRINT_REPORT_ROW_("Line", REPORT_COL1_SIZE, 0, ':');
				//_FPC_PRINT_REPORT_ROW_(l, REPORT_COL2_SIZE, 1);
				_FPC_PRINT_REPORT_ROW_(loc, REPORT_COL2_SIZE, 1);
				_FPC_PRINT_REPORT_LINE_('+');

				asm("trap;");
		}
	}
}

void _FPC_PRINT_AT_MAIN_()
{
	printf("\n");
	printf("========================================\n");
	printf(" FPChecker (v0.0.1, %s)\n", __DATE__);
	printf("========================================\n");
	printf("\n");
}

/* -------------------------------------- */
/*         Warning: DO NOT MODIFY!        */
/* It will be instrumented */
__device__
__attribute__((noinline))  int _FPC_READ_GLOBAL_ERRORS_ARRAY_(int i)
{
	asm ("");
	return errors_per_line_array[i];
}
__device__
__attribute__((noinline)) void _FPC_WRITE_GLOBAL_ERRORS_ARRAY_(int i, int val)
{
	asm ("");
	errors_per_line_array[i] = val;
}
/* -------------------------------------- */

__device__
static int getGlobalIdx_3D_3D()
{
	int blockId = blockIdx.x + blockIdx.y * gridDim.x
		+ gridDim.x * gridDim.y * blockIdx.z;
	int threadId = blockId * (blockDim.x * blockDim.y * blockDim.z)
 		+ (threadIdx.z * (blockDim.x * blockDim.y))
 		+ (threadIdx.y * blockDim.x) + threadIdx.x;
	return threadId;
}

__device__
void _FPC_PRINT_ERRORS_()
{
	int id = getGlobalIdx_3D_3D();
	if (id == 0)
	{
		for (int i=0; i < errors_array_size; ++i)
		{
			int errors = _FPC_READ_GLOBAL_ERRORS_ARRAY_(i);
			if (errors > 0)
			{
				printf("\n#FPCHECKER: Errors at %s:%d (#%d, tid:%d)\n", _FPC_FILE_NAME_[0], i, errors, id);
				_FPC_WRITE_GLOBAL_ERRORS_ARRAY_(i, INT_MIN);
			}
			else if (errors < 0)
			{
				// This is so we do not report them multiple times
				_FPC_WRITE_GLOBAL_ERRORS_ARRAY_(i, INT_MIN);
			}
		}
	}

}

/* ------------------------ FP32 Functions --------------------------------- */

__device__
static void _FPC_FP32_CHECK_OPERATION_(float x, float y, float z, int loc, int op)
{
	if (isinf(x))
	{
		_FPC_INTERRUPT_(1, op, loc, x, 0);
	}
	else if (isnan(x))
	{
		_FPC_INTERRUPT_(0, op, loc, x, 0);
	}
	else /// subnormals check
	{
		if (_FPC_FP32_IS_SUBNORMAL(x))
		{
			_FPC_INTERRUPT_(2, op, loc, x, 0);
		}
		else if (_FPC_FP32_IS_ALMOST_SUBNORMAL(x))
		{
			_FPC_WARNING_(2, op, loc, x, 0);
		}
		else if (_FPC_FP32_IS_ALMOST_OVERFLOW(x))
		{
			_FPC_WARNING_(1, op, loc, x, 0);
		}
	}
}

//// Returns non-zero value if FP argument is a sub-normal
__device__
static int _FPC_FP32_IS_SUBNORMAL(float x)
{
	int ret = 0;
	uint32_t val;
  memcpy((void *) &val, (void *) &x, sizeof(val));
  val = val << 1; 	// get rid of sign bit
  val = val >> 24; 	// get rid of the mantissa bits
  if (x != 0.0 && x != -0.0)
  {
  	if (val == 0)
  		ret = 1;
  }
  return ret;
}

__device__
static int _FPC_FP32_IS_ALMOST_OVERFLOW(float x)
{
	int ret = 0;
	uint32_t val;
  memcpy((void *) &val, (void *) &x, sizeof(val));
  val = val << 1; 	// get rid of sign bit
  val = val >> 24; 	// get rid of the mantissa bits
  if (x != 0.0 && x != -0.0)
  {
  	int maxVal = 256 - (int)(DANGER_ZONE_PERCENTAGE*256.0);
  	if (val >= maxVal)
  		ret = 1;
  }
  return ret;
}

__device__
static int _FPC_FP32_IS_ALMOST_SUBNORMAL(float x)
{
	int ret = 0;
	uint32_t val;
  memcpy((void *) &val, (void *) &x, sizeof(val));
  val = val << 1; 	// get rid of sign bit
  val = val >> 24; 	// get rid of the mantissa bits
  if (x != 0.0 && x != -0.0)
  {
  	int minVal = (int)(DANGER_ZONE_PERCENTAGE*256.0);
  	if (val <= minVal)
  		ret = 1;
  }
  return ret;
}

__device__
void _FPC_FP32_CHECK_ADD_(float x, float y, float z, int loc)
{
	_FPC_FP32_CHECK_OPERATION_(x, y, z, loc, 0);
}

__device__
void _FPC_FP32_CHECK_SUB_(float x, float y, float z, int loc)
{
	_FPC_FP32_CHECK_OPERATION_(x, y, z, loc, 1);
}

__device__
void _FPC_FP32_CHECK_MUL_(float x, float y, float z, int loc)
{
	_FPC_FP32_CHECK_OPERATION_(x, y, z, loc, 2);
}

__device__
void _FPC_FP32_CHECK_DIV_(float x, float y, float z, int loc)
{
	_FPC_FP32_CHECK_OPERATION_(x, y, z, loc, 3);
}

/* ------------------------ FP64 Functions --------------------------------- */

__device__
static void _FPC_FP64_CHECK_OPERATION_(double x, double y, double z, int loc, int op)
{
	if (isinf(x))
	{
		_FPC_INTERRUPT_(1, op, loc, 0, x);
	}
	else if (isnan(x))
	{
		_FPC_INTERRUPT_(0, op, loc, 0, x);
	}
	else /// subnormals check
	{
		if (_FPC_FP64_IS_SUBNORMAL(x))
		{
			_FPC_INTERRUPT_(2, op, loc, 0, x);
		}
		else if (_FPC_FP64_IS_ALMOST_SUBNORMAL(x))
		{
			_FPC_WARNING_(2, op, loc, 0, x);
		}
		else if (_FPC_FP64_IS_ALMOST_OVERFLOW(x))
		{
			_FPC_WARNING_(1, op, loc, 0, x);
		}
	}
}

/// Returns non-zero value if FP argument is a sub-normal.
/// Check that the exponent bits are zero.
__device__
static int _FPC_FP64_IS_SUBNORMAL(double x)
{
	int ret = 0;
	uint64_t val;
  memcpy((void *) &val, (void *) &x, sizeof(val));
  val = val << 1; 	// get rid of sign bit
  val = val >> 53; 	// get rid of the mantissa bits
  if (x != 0.0 && x != -0.0)
  {
  	if (val == 0)
  		ret = 1;
  }
  return ret;
}

__device__
static int _FPC_FP64_IS_ALMOST_OVERFLOW(double x)
{
	int ret = 0;
	uint64_t val;
  memcpy((void *) &val, (void *) &x, sizeof(val));
  val = val << 1; 	// get rid of sign bit
  val = val >> 53; 	// get rid of the mantissa bits
  if (x != 0.0 && x != -0.0)
  {
  	int maxVal = 2048 - (int)(DANGER_ZONE_PERCENTAGE*2048.0);
  	if (val >= maxVal)
  		ret = 1;
  }
  return ret;
}

__device__
static int _FPC_FP64_IS_ALMOST_SUBNORMAL(double x)
{
	int ret = 0;
	uint64_t val;
  memcpy((void *) &val, (void *) &x, sizeof(val));
  val = val << 1; 	// get rid of sign bit
  val = val >> 53; 	// get rid of the mantissa bits
  if (x != 0.0 && x != -0.0)
  {
  	int minVal = (int)(DANGER_ZONE_PERCENTAGE*2048.0);
  	if (val <= minVal)
  		ret = 1;
  }
  return ret;
}

__device__
void _FPC_FP64_CHECK_ADD_(double x, double y, double z, int loc)
{
	_FPC_FP64_CHECK_OPERATION_(x, y, z, loc, 0);
}

__device__
void _FPC_FP64_CHECK_SUB_(double x, double y, double z, int loc)
{
	_FPC_FP64_CHECK_OPERATION_(x, y, z, loc, 1);
}

__device__
void _FPC_FP64_CHECK_MUL_(double x, double y, double z, int loc)
{
	_FPC_FP64_CHECK_OPERATION_(x, y, z, loc, 2);
}

__device__
void _FPC_FP64_CHECK_DIV_(double x, double y, double z, int loc)
{
	_FPC_FP64_CHECK_OPERATION_(x, y, z, loc, 3);
}

#endif /* SRC_RUNTIME_H_ */
