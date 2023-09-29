#include "d3dUtil.h"

void ReleaseCOM(IUnknown* x) { if (x) { x->Release(); x = 0; } }
