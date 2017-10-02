#include <stdlib.h>
#include <stdio.h>


#include "macros.h"
#include "record.h"

int main(int argc, const char **argv)
{
	struct record new_record;

	if (record_init(&new_record)) {
		ERROR("Couldn't initialize a record!");
		return EXIT_FAILURE;
	}

	/* Some debug code be guarded in ifdef's
	 * Enable it compiling with `make debug`
	 */
#ifdef DEBUG
	LOG("Content of the record after initialization:");
	LOG("\tType= %d", record_get_type(&new_record));
	if (record_has_footer(&new_record))
		LOG("\tUUID= %d", record_get_uuid(&new_record));
	else
		LOG("\tNo footer");
	LOG("\tLength= %d", record_get_length(&new_record));
#endif

	/* Do something with the record, e.g. write it in a file, load one from
	 * a file, ...
	 * You should test your implementation here.
	 * Think on using the program arguments
	 * as an easy way to script a lot of tests.
	 */

	 FILE *read = fopen("./output","r");
	 if (!read) return EXIT_FAILURE;

	 FILE *write = fopen("./output_lo","w");
	 if (!write) return EXIT_FAILURE;

	 while (record_read(&new_record, read) != 0) {
	 	record_write(&new_record, write);
	 }

	 fclose(read);
	 fclose(write);

	return EXIT_SUCCESS;
}
