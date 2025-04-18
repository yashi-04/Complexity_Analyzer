#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256
#define MAX_FUNCTIONS 50

// Structure to store function information
typedef struct {
    char name[50];
    int is_recursive;
    int loop_depth;
    int has_recursion;
} FunctionInfo;

// Structure to store analysis results
typedef struct {
    int total_loops;
    int max_nested_loops;
    int recursive_functions;
    int total_functions;
    int malloc_calls;
    int array_declarations;
    FunctionInfo functions[MAX_FUNCTIONS];
} CodeAnalysis;

// Function prototypes
void analyze_file(const char *filename, CodeAnalysis *analysis);
void print_analysis_report(CodeAnalysis *analysis);
void estimate_complexity(CodeAnalysis *analysis);
int is_loop_start(const char *line);
int is_function_start(const char *line);
int is_recursive_call(const char *line, const char *func_name);
int is_malloc_call(const char *line);
int is_array_declaration(const char *line);

void analyze_file(const char *filename, CodeAnalysis *analysis) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    char current_function[50] = "";
    int in_function = 0;
    int brace_count = 0;
    int loop_stack_depth = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove leading/trailing whitespace
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;
        
        // Skip empty lines and comments
        if (strlen(trimmed_line) == 0 || strstr(trimmed_line, "//") == trimmed_line) {
            continue;
        }

        // Check for function start
        // Inside analyze_file(), modify the function detection part:
        if (!in_function && is_function_start(trimmed_line)) {
            sscanf(trimmed_line, "%*s %s", current_function);
            strtok(current_function, "("); // Remove parameters
            strcpy(analysis->functions[analysis->total_functions].name, current_function);
            analysis->functions[analysis->total_functions].is_recursive = 0;
            analysis->functions[analysis->total_functions].loop_depth = 0;
            analysis->functions[analysis->total_functions].has_recursion = 0;  // Initialize to 0
            in_function = 1;
            analysis->total_functions++;
        }

        // Track braces to know when we exit a function
        if (in_function) {
            char *ptr = trimmed_line;
            while (*ptr) {
                if (*ptr == '{') brace_count++;
                if (*ptr == '}') brace_count--;
                ptr++;
            }

            if (brace_count == 0) {
                in_function = 0;
                current_function[0] = '\0';
                loop_stack_depth = 0;
            }
        }

        // Check for loops
        if (in_function && is_loop_start(trimmed_line)) {
            analysis->total_loops++;
            loop_stack_depth++;
            if (loop_stack_depth > analysis->max_nested_loops) {
                analysis->max_nested_loops = loop_stack_depth;
            }
            
            // Update current function's max loop depth
            if (loop_stack_depth > analysis->functions[analysis->total_functions-1].loop_depth) {
                analysis->functions[analysis->total_functions-1].loop_depth = loop_stack_depth;
            }
        }

        // Check for loop end (simplified - looks for closing brace)
        if (in_function && strstr(trimmed_line, "}") && loop_stack_depth > 0) {
            loop_stack_depth--;
        }

        // Check for recursion
        // Inside analyze_file(), modify the recursion check:
if (in_function && is_recursive_call(trimmed_line, current_function)) {
    analysis->functions[analysis->total_functions-1].has_recursion = 1;
    
    // Only count as recursive function if we haven't already
        if (!analysis->functions[analysis->total_functions-1].is_recursive) {
            analysis->recursive_functions++;
            analysis->functions[analysis->total_functions-1].is_recursive = 1;
        }
}

        // Check for malloc calls
        if (is_malloc_call(trimmed_line)) {
            analysis->malloc_calls++;
        }

        // Check for array declarations
        if (is_array_declaration(trimmed_line)) {
            analysis->array_declarations++;
        }
    }

    fclose(file);
}

int is_loop_start(const char *line) {
    return strstr(line, "for(") || strstr(line, "for (") ||
           strstr(line, "while(") || strstr(line, "while (") ||
           strstr(line, "do{") || strstr(line, "do {");
}

int is_function_start(const char *line) {
    // Simple check - looks for pattern "type name("
    char *open_paren = strchr(line, '(');
    if (!open_paren) return 0;
    
    // Check if there's a return type before the name
    for (const char *p = line; p < open_paren; p++) {
        if (isspace(*p)) return 1;
    }
    
    return 0;
}

int is_recursive_call(const char *line, const char *func_name) {
    if (!func_name || strlen(func_name) == 0) return 0;
    
    // Skip lines that look like function declarations
    if (strstr(line, func_name) && strchr(line, '{')) {
        return 0;
    }
    
    char *call_pos = strstr(line, func_name);
    if (!call_pos) return 0;
    
    // Check if it's a function call (has parentheses after the name)
    char *after_name = call_pos + strlen(func_name);
    while (*after_name && isspace(*after_name)) after_name++;
    
    return *after_name == '(';
}

int is_malloc_call(const char *line) {
    return strstr(line, "malloc(") || strstr(line, "malloc (");
}

int is_array_declaration(const char *line) {
    // Look for pattern "type name[...]" or "type name[ ]"
    char *open_bracket = strchr(line, '[');
    if (!open_bracket) return 0;
    
    char *close_bracket = strchr(open_bracket, ']');
    if (!close_bracket) return 0;
    
    // Check there's something between the brackets (or spaces)
    for (char *p = open_bracket + 1; p < close_bracket; p++) {
        if (!isspace(*p)) return 1;
    }
    
    return 0;
}
void print_analysis_report(CodeAnalysis *analysis) {
    printf("\n=== Code Complexity Analysis Report ===\n\n");
    
    printf("Total functions: %d\n", analysis->total_functions);
    printf("Total loops: %d\n", analysis->total_loops);
    printf("Maximum nested loops: %d\n", analysis->max_nested_loops);
    printf("Recursive functions: %d\n", analysis->recursive_functions);
    printf("Memory allocations (malloc): %d\n", analysis->malloc_calls);
    printf("Array declarations: %d\n\n", analysis->array_declarations);
    
    printf("Function Details:\n");
    for (int i = 0; i < analysis->total_functions; i++) {
        printf("  %s(): ", analysis->functions[i].name);
        if (analysis->functions[i].has_recursion) {
            printf("Recursive, ");
        }
        printf("Max loop depth: %d\n", analysis->functions[i].loop_depth);
    }
    
    printf("\n");
    estimate_complexity(analysis);
}

void estimate_complexity(CodeAnalysis *analysis) {
    printf("Estimated Time Complexity:\n");
    
    if (analysis->recursive_functions > 0) {
        printf("- Exponential (O(2^n)) or worse detected due to recursion\n");
    }
    
    if (analysis->max_nested_loops >= 3) {
        printf("- Polynomial (O(n^%d)) detected due to %d nested loops\n", 
               analysis->max_nested_loops, analysis->max_nested_loops);
    } else if (analysis->max_nested_loops == 2) {
        printf("- Quadratic (O(n^2)) detected due to nested loops\n");
    } else if (analysis->total_loops > 0) {
        printf("- Linear (O(n)) detected due to loops\n");
    } else {
        printf("- Constant (O(1)) - no loops found\n");
    }
    
    printf("\nEstimated Space Complexity:\n");
    if (analysis->malloc_calls > 0 || analysis->array_declarations > 0) {
        if (analysis->recursive_functions > 0) {
            printf("- O(n) or worse (dynamic allocations and/or recursion)\n");
        } else {
            printf("- O(n) (dynamic allocations detected)\n");
        }
    } else {
        printf("- O(1) (no significant dynamic allocations)\n");
    }
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename.c>\n", argv[0]);
        return 1;
    }

    CodeAnalysis analysis = {0};
    analyze_file(argv[1], &analysis);
    print_analysis_report(&analysis);
    
    return 0;
}
