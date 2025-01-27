#include <stdio.h>
#include <stdlib.h>
#include "lwp.h"

void rr_admit(thread newThread);
void rr_remove(thread victim);
thread rr_next();
int rr_qlen();

