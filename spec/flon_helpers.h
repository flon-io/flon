
//
// helpers
//
// Tue Jan 13 06:17:52 JST 2015
//


/* Expects the last arg to be an int (timeout in seconds).
 * Filetype is either 'f' or 'd'.
 * Returns 0 in case of timeout or if the file is present but not of the
 * expected type. Returns 1 else (success).
 * and present.
 * Returns as soon as the file is present ('d' or 'f').
 */
int hlp_wait_for_file(char filetype, const char *path, ...);

