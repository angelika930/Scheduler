#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

void rr_admit(thread new);
void rr_remove(thread victim);
thread rr_next();
int rr_qlen();

