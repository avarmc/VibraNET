#pragma once
#include "VIVarTag.h"



typedef struct tagVI_VAR
{
	float		fv1;
	float		fv2;
	int			iv1;
	int			iv2;
} VI_VAR;

enum VI_VAR_SUB
{
	VI_VAR_SUB_I1,VI_VAR_SUB_I2,VI_VAR_SUB_F1,VI_VAR_SUB_F2,
	VI_VAR_SUB_STRING,
	VI_VAR_SUB_BOOL1, VI_VAR_SUB_BOOL2,
	VI_VAR_SUB_SELECT1, VI_VAR_SUB_SELECT2
};

