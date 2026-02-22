#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dsconv.h"
#include "parser.h"
#include "generator.h"

static char *strip_brackets(const char *s) {
    char *dup = strdup(s);
    if (dup[0] == '[') memmove(dup, dup + 1, strlen(dup));
    size_t len = strlen(dup);
    if (len > 0 && dup[len - 1] == ']') dup[len - 1] = '\0';
    return dup;
}

static char **read_file_list(const char *filename, int *count) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;
    char **list = NULL;
    *count = 0;
    // Get directory of filename
    char *dir = NULL;
    const char *last_slash = strrchr(filename, '\\');
    if (!last_slash) last_slash = strrchr(filename, '/');
    if (last_slash) {
        size_t dir_len = last_slash - filename + 1;
        dir = (char*)malloc(dir_len + 1);
        memcpy(dir, filename, dir_len);
        dir[dir_len] = '\0';
    }
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        // remove newline
        line[strcspn(line, "\n")] = 0;
        char *stripped = strip_brackets(line);
        if (strlen(stripped) > 0) {
            // Split by spaces
            char *token = strtok(stripped, " ");
            while (token) {
                char *full_path = NULL;
                if (dir) {
                    full_path = (char*)malloc(strlen(dir) + strlen(token) + 1);
                    strcpy(full_path, dir);
                    strcat(full_path, token);
                } else {
                    full_path = strdup(token);
                }
                list = (char**)realloc(list, (*count + 1) * sizeof(char*));
                list[(*count)++] = full_path;
                token = strtok(NULL, " ");
            }
            free(stripped);
        } else {
            free(stripped);
        }
    }
    if (dir) free(dir);
    fclose(f);
    return list;
}

static void print_usage(const char *prog) {
	fprintf(stderr,
		"%s - C Data Structure Converter\n\n"
		"USAGE: %s [flags] target\n\n"
		"PREFIXES: (prior to the programs name)\n\n"
		" <                  batch operator for user input redirection (e.g., dsconv < input.txt).\n\n"
		"SWITCHES:\n\n"
		"  /s                Silent mode.\n\n"
		"FLAGS:\n\n"
		"  -esm / -ism       External/Internal struct members.\n"
		"  -em / -im         External/Internal all members.\n"
		"  -eu / -iu         External/Internal union members.\n"
		"  -sf               Type suffixes in output (f, LL, ULL, etc.).\n"
		"  -sn [name]        Instance name (auto-detected if flag is omitted).\n"
		"  -olang [lang]     Output language(s).\n"
		"  -i [string]       Input code string (alternative to file).\n"
		"  -o [file]         Output file (default: stdout).\n"
		"  -p [file]         Log metadata file.\n"
		"  -a                Output as array.\n"
		"  -s                Output as struct.\n"
		"  -u                Output as union.\n"
		"  -e                Output as enum.\n"
		"  -etc              Explicit type casting.\n"
		"  -itc              Implicit type casting.\n"
		"  -fwd              Forward declarations in output.\n"
		"  -nrd              Normal declarations in output.\n"
		"  -g                Globally declared data structure(s).\n"
		"  -l                Locally declared data structure(s).\n"
		"  -l:FunctionName   Locally declared data structure(s) within a function.\n"
		"  [ ]               Grouping operators for input and output files.\n"
		"  .txt              Files containing lists of input files (one per line).\n"
		"  -? / -h           Show this help.\n\n"
		"OPERATORS:\n\n"
		"  > [file]          Redirects STDOUT (standard output) into a file.\n"
		"  >> [file]         Redirects and appends STDOUT (standard output) into a file.\n"
		"  | [prog]          Takes STDOUT from the left program and feeds it into STDIN of the right program.\n",
		prog, prog);
}

int main(int argc, char **argv) {
	Options opts;
	memset(&opts, 0, sizeof(opts));
	opts.name_policy = NAME_POLICY_PRESERVE;
	opts.assign_style = ASSIGN_BOTH;
	opts.expand_mode = EXPAND_MEMBERS;
	opts.silent = 0;
	opts.enable_suffixes = 0;

	if (argc <= 1) {
		print_usage(argv[0]);
		return 1;
	}

	// Collect input files (for grouped [] support) and parse flags
	const char **input_files = NULL;
	int input_count = 0;

	// Parse flags and arguments
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '/' && strcmp(argv[i], "/s") == 0) {
			opts.silent = 1;
			continue;
		}
		int flag_type = 0;
		if (strcmp(argv[i], "-esm") == 0) flag_type = 1;
		else if (strcmp(argv[i], "-ism") == 0) flag_type = 2;
		else if (strcmp(argv[i], "-em") == 0) flag_type = 3;
		else if (strcmp(argv[i], "-im") == 0) flag_type = 4;
		else if (strcmp(argv[i], "-eu") == 0) flag_type = 5;
		else if (strcmp(argv[i], "-iu") == 0) flag_type = 6;
		else if (strcmp(argv[i], "-st") == 0) flag_type = 7;
		else if (strcmp(argv[i], "-stw") == 0) flag_type = 8;
		else if (strcmp(argv[i], "-iv") == 0) flag_type = 9;
		else if (strcmp(argv[i], "-ev") == 0) flag_type = 10;
		else if (strcmp(argv[i], "-sf") == 0) flag_type = 11;
		else if (strcmp(argv[i], "-sn") == 0) flag_type = 12;
		else if (strcmp(argv[i], "-olang") == 0) flag_type = 13;
		else if (strcmp(argv[i], "-i") == 0) flag_type = 14;
		else if (strcmp(argv[i], "-o") == 0) flag_type = 15;
		else if (strcmp(argv[i], "-p") == 0) flag_type = 16;
		else if (strcmp(argv[i], "-s") == 0) flag_type = 17;
		else if (strcmp(argv[i], "-a") == 0) flag_type = 18;
		else if (strcmp(argv[i], "-etc") == 0) flag_type = 19;
		else if (strcmp(argv[i], "-itc") == 0) flag_type = 20;
		else if (strcmp(argv[i], "-u") == 0) flag_type = 21;
		else if (strcmp(argv[i], "-e") == 0) flag_type = 22;
		else if (strcmp(argv[i], "-fwd") == 0) flag_type = 23;
		else if (strcmp(argv[i], "-nrd") == 0) flag_type = 24;
		else if (strcmp(argv[i], "-g") == 0) flag_type = 25;
		else if (strcmp(argv[i], "-l") == 0) flag_type = 26;
		else if (strcmp(argv[i], "-?") == 0) flag_type = 27;
		else if (strcmp(argv[i], "-h") == 0) flag_type = 28;
		if (flag_type > 0) {
			switch (flag_type) {
				case 1: // -esm
					// expand struct members
					break;
				case 2: // -ism
					// internal struct members
					break;
				case 3: // -em
					// expand all members
					break;
				case 4: // -im
					// internal all members
					break;
				case 5: // -eu
					// expand union members
					break;
				case 6: // -iu
					// internal union members
					break;
				case 7: // -st
					opts.expand_mode = EXPAND_MEMBERS;
					break;
				case 8: // -stw
					opts.expand_mode = WRAP_AS_ARRAY;
					break;
				case 9: // -iv
					opts.assign_style = ASSIGN_INTERNAL;
					break;
				case 10: // -ev
					opts.assign_style = ASSIGN_EXTERNAL;
					break;
				case 11: // -sf
					opts.enable_suffixes = 1;
					break;
				case 12: // -sn
					if (i + 1 < argc) {
						opts.instance_name = argv[++i];
					}
					break;
				case 13: // -olang
					if (i + 1 < argc) {
						opts.targets = argv[++i];
					}
					break;
				case 14: // -i
					if (i + 1 < argc) {
						opts.input_string = argv[++i];
					}
					break;
				case 15: // -o
					if (i + 1 < argc) {
						opts.output_file = argv[++i];
					}
					break;
				case 16: // -p
					if (i + 1 < argc) {
						opts.metadata_file = argv[++i];
					}
					break;
				case 17: // -s
					opts.output_as_struct = 1;
					break;
				case 18: // -a
					opts.array_output = 1;
					break;
				case 19: // -etc
					opts.explicit_cast = 1;
					break;
				case 20: // -itc
					opts.explicit_cast = 0;
					break;
				case 21: // -u
					opts.output_as_union = 1;
					break;
				case 22: // -e
					opts.output_as_enum = 1;
					break;
				case 23: // -fwd
					opts.forward_declarations = 1;
					break;
				case 24: // -nrd
					opts.normal_declarations = 1;
					break;
				case 25: // -g
					opts.global_scope = 1;
					break;
				case 26: // -l
					opts.local_scope = 1;
					if (i + 1 < argc && argv[i+1][0] == ':') {
						opts.local_function = argv[++i] + 1;
					}
					break;
				case 27: // -?
				case 28: // -h
					print_usage(argv[0]);
					return 0;
			}
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "Unknown flag: %s\n", argv[i]);
			print_usage(argv[0]);
			return 1;
		} else {
			// Collect input files (strip brackets if present)
			char *stripped = strip_brackets(argv[i]);
			if (strlen(stripped) > 0) {
				// Check if it's a .txt file containing a list of files
				if (strstr(stripped, ".txt")) {
					int sub_count;
					char **sub_list = read_file_list(stripped, &sub_count);
					if (sub_list) {
						for (int j = 0; j < sub_count; ++j) {
							input_files = (const char**)realloc((void*)input_files, (input_count + 1) * sizeof(const char*));
							input_files[input_count++] = sub_list[j];
						}
						free(sub_list);
					} else {
						fprintf(stderr, "Failed to read file list: %s\n", stripped);
						free(stripped);
						return 1;
					}
				} else {
					input_files = (const char**)realloc((void*)input_files, (input_count + 1) * sizeof(const char*));
					input_files[input_count++] = stripped;
				}
			} else {
				free(stripped);
			}
		}
	}

	// Check for CON with multiple inputs
	if (opts.output_file && strcmp(opts.output_file, "CON") == 0 && input_count > 1) {
		fprintf(stderr, "Cannot output multiple inputs to console\n");
		for (int i = 0; i < input_count; ++i) free((char*)input_files[i]);
		free(input_files);
		return 1;
	}

	// Use first input file if no -i string provided
	if (!opts.input_string && input_count > 0) {
		opts.input_file = input_files[0];
	}

	if (!opts.input_file && !opts.input_string) {
		fprintf(stderr, "Missing target file or -i string.\n");
		print_usage(argv[0]);
		free(input_files);
		return 1;
	}

	// Detect source language from file extension if file input
	if (opts.input_file) {
		if (strstr(opts.input_file, ".c") || strstr(opts.input_file, ".h")) {
			opts.src_lang = "c";
		} else if (strstr(opts.input_file, ".bas")) {
			opts.src_lang = "freebasic";
		} else if (strstr(opts.input_file, ".pas")) {
			opts.src_lang = "freepascal";
		} else {
			opts.src_lang = "c"; // default
		}
	} else {
		// For string input, assume C
		opts.src_lang = "c";
	}
	// Targets default to "c" if not set
	if (!opts.targets) opts.targets = "c";

	// Validation: output file cannot be same as input file
	if (opts.output_file && opts.input_file && strcmp(opts.output_file, opts.input_file) == 0) {
		fprintf(stderr, "Output file cannot be the same as input file.\n");
		free(input_files);
		return 1;
	}

	// Parse input(s) and merge ASTs
	ASTRoot *ast = NULL;
	if (opts.input_string) {
		if (!opts.silent) {
			printf("DSConv: parsing input string\n");
		}
		ast = parse_string(opts.input_string, &opts);
		if (!ast) {
			fprintf(stderr, "Parsing failed.\n");
			for (int i = 0; i < input_count; ++i) free((char*)input_files[i]);
			free(input_files);
			return 1;
		}
	} else if (input_count >= 1) {
		// Multiple files/strings - merge ASTs
		if (!opts.silent) {
			printf("DSConv: parsing and merging %d inputs\n", input_count);
		}
		ast = (ASTRoot*)calloc(1, sizeof(ASTRoot));
		ast->first = NULL;
		for (int i = 0; i < input_count; ++i) {
			if (!opts.silent) {
				printf("  [%d/%d] %s\n", i + 1, input_count, input_files[i]);
			}
			ASTRoot *partial = NULL;
			// Check if it's a file or string
			FILE *f = fopen(input_files[i], "r");
			if (f) {
				fclose(f);
				partial = parse_file(input_files[i], &opts);
			} else {
				partial = parse_string(input_files[i], &opts);
			}
			if (!partial) {
				fprintf(stderr, "Parsing failed for %s.\n", input_files[i]);
				for (int j = 0; j < input_count; ++j) free((char*)input_files[j]);
				free(input_files);
				return 1;
			}
			// Merge: append partial->first to ast
			if (!ast->first) {
				ast->first = partial->first;
			} else {
				ASTNode *last = ast->first;
				while (last->next) last = last->next;
				last->next = partial->first;
			}
			free(partial);
		}
	} else {
		fprintf(stderr, "No input files provided.\n");
		for (int i = 0; i < input_count; ++i) free((char*)input_files[i]);
		free(input_files);
		return 1;
	}
	if (!opts.silent && (!opts.output_file || strcmp(opts.output_file, "CON") == 0)) {
		printf("---------------------------------------\n");
	}
	int rc = generate_for_targets(ast, &opts);
	(void)rc;
	for (int i = 0; i < input_count; ++i) free((char*)input_files[i]);
	free(input_files);
	return 0;
}
