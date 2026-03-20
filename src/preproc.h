/*
 * preproc.h - Simple preprocessor for MUD zone files
 *
 * This preprocessor handles the subset of C preprocessor features used by
 * the zone files: #include, #define, and #undef.
 *
 * It preserves multiline strings exactly as-is, which is critical for
 * zone file parsing.
 */

#ifndef _PREPROC_H_
#define _PREPROC_H_

/*
 * preproc_open - Open a file and preprocess it
 *
 * Parameters:
 *   filename    - Path to the zone file to preprocess
 *   include_path - Directory to search for #include files (usually "../include")
 *
 * Returns:
 *   FILE* pointer to a pipe containing the preprocessed output
 *   NULL on error
 *
 * The caller should use preproc_close() or pclose() to close the returned
 * file pointer.
 */
FILE *preproc_open(const char *filename, const char *include_path);

/*
 * preproc_close - Close a preprocessed file stream
 *
 * Parameters:
 *   fp - File pointer returned by preproc_open()
 *
 * Returns:
 *   0 on success, non-zero on error
 */
int preproc_close(FILE *fp);

#endif /* _PREPROC_H_ */
