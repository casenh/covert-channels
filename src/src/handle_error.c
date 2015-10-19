//The error handler used by PAPI. Make sure to compile with all code

#include <stdio.h>
#include <stdlib.h>
#include "../../papi-5.3.0/src/papi.h"

void handle_error(int retval){

	printf("PAPI error %d: %s \n", retval, PAPI_strerror(retval));
	exit(1);

}
