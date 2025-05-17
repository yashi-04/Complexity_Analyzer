#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <json-c/json.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LINES 1000
#define MAX_FUNCTIONS 50
#define MAX_JSON_SIZE 16384

typedef struct {
    char name[50];
    int is_recursive;
    int loop_depth;
    int has_recursion;
} FunctionInfo;

typedef struct {
    int total_loops;
    int max_nested_loops;
    int recursive_functions;
    int total_functions;
    int malloc_calls;
    int array_declarations;
    FunctionInfo functions[MAX_FUNCTIONS];
} CodeAnalysis;

// Helper function declarations
static int is_loop_start(const char* line);
static int is_function_start(const char* line);
static int is_recursive_call(const char* line, const char* func_name);
static int is_malloc_call(const char* line);
static int is_array_declaration(const char* line);

// Helper function implementations
static int is_loop_start(const char* line) {
    return strstr(line, "for(") || strstr(line, "for (") ||
           strstr(line, "while(") || strstr(line, "while (") ||
           strstr(line, "do{") || strstr(line, "do {");
}

static int is_function_start(const char* line) {
    char* open_paren = strchr(line, '(');
    if (!open_paren) return 0;
    
    for (const char* p = line; p < open_paren; p++) {
        if (isspace(*p)) return 1;
    }
    return 0;
}

static int is_recursive_call(const char* line, const char* func_name) {
    if (!func_name || strlen(func_name) == 0) return 0;
    
    if (strstr(line, func_name) && strchr(line, '{')) {
        return 0;
    }
    
    char* call_pos = strstr(line, func_name);
    if (!call_pos) return 0;
    
    char* after_name = call_pos + strlen(func_name);
    while (*after_name && isspace(*after_name)) after_name++;
    
    return *after_name == '(';
}

static int is_malloc_call(const char* line) {
    return strstr(line, "malloc(") || strstr(line, "malloc (");
}

static int is_array_declaration(const char* line) {
    char* open_bracket = strchr(line, '[');
    if (!open_bracket) return 0;
    
    char* close_bracket = strchr(open_bracket, ']');
    if (!close_bracket) return 0;
    
    for (char* p = open_bracket + 1; p < close_bracket; p++) {
        if (!isspace(*p)) return 1;
    }
    
    return 0;
}

// Add classic algorithm pattern detection helper
static void detect_classic_patterns(const char* line, struct json_object* analysis) {
    // Bubble Sort
    if (strstr(line, "for") && strstr(line, "j") && strstr(line, "i+1")) {
        json_object_object_add(analysis, "bubble_sort_pattern", json_object_new_boolean(1));
    }
    // Merge Sort
    if (strstr(line, "merge") || (strstr(line, "L[") && strstr(line, "R[")) || (strstr(line, "n1") && strstr(line, "n2"))) {
        json_object_object_add(analysis, "merge_sort_pattern", json_object_new_boolean(1));
    }
    // Quick Sort
    if (strstr(line, "pivot") || (strstr(line, "partition") && strstr(line, "return")) || (strstr(line, "i+1") && strstr(line, "j-1"))) {
        json_object_object_add(analysis, "quick_sort_pattern", json_object_new_boolean(1));
    }
    // Binary Search
    if (strstr(line, "return") && ((strstr(line, "mid - 1") || strstr(line, "mid + 1")) || (strstr(line, "high = mid") || strstr(line, "low = mid")))) {
        json_object_object_add(analysis, "binary_search_pattern", json_object_new_boolean(1));
    }
    // Fibonacci DP
    if ((strstr(line, "fib") || strstr(line, "dp")) && (strstr(line, "[") && strstr(line, "]"))) {
        json_object_object_add(analysis, "fibonacci_dp_pattern", json_object_new_boolean(1));
    }
    // Dijkstra's Algorithm
    if (strstr(line, "dist") && (strstr(line, "priority_queue") || strstr(line, "min_heap"))) {
        json_object_object_add(analysis, "dijkstra_pattern", json_object_new_boolean(1));
    }
    // Knapsack
    if (strstr(line, "weight") && strstr(line, "value") && (strstr(line, "[") && strstr(line, "]"))) {
        json_object_object_add(analysis, "knapsack_pattern", json_object_new_boolean(1));
    }
    // Sieve of Eratosthenes
    if (strstr(line, "prime") && strstr(line, "for") && (strstr(line, "i*i") || strstr(line, "i+i"))) {
        json_object_object_add(analysis, "sieve_pattern", json_object_new_boolean(1));
    }
    // BFS
    if (strstr(line, "queue") && (strstr(line, "visited") || strstr(line, "level"))) {
        json_object_object_add(analysis, "bfs_pattern", json_object_new_boolean(1));
    }
    // Power/Exponentiation
    if (strstr(line, "return") && ((strstr(line, "e % 2") || strstr(line, "e/2")) || (strstr(line, "e == 0") || strstr(line, "e == 1")))) {
        json_object_object_add(analysis, "power_pattern", json_object_new_boolean(1));
    }
}

// Function to analyze a single line of code
static void analyze_line(const char* line, struct json_object* analysis, CodeAnalysis* code_analysis) {
    // Skip empty lines and comments
    if (!line || line[0] == '\0' || line[0] == '/' || line[0] == '*') {
        return;
    }

    // Remove leading/trailing whitespace
    char* trimmed_line = strdup(line);
    char* p = trimmed_line;
    while (isspace(*p)) p++;
    char* end = p + strlen(p) - 1;
    while (end > p && isspace(*end)) *end-- = '\0';

    // Check for function start
    if (is_function_start(trimmed_line)) {
        char func_name[50];
        sscanf(trimmed_line, "%*s %s", func_name);
        char* paren = strchr(func_name, '(');
        if (paren) *paren = '\0';
        
        if (code_analysis->total_functions < MAX_FUNCTIONS) {
            strcpy(code_analysis->functions[code_analysis->total_functions].name, func_name);
            code_analysis->functions[code_analysis->total_functions].is_recursive = 0;
            code_analysis->functions[code_analysis->total_functions].loop_depth = 0;
            code_analysis->functions[code_analysis->total_functions].has_recursion = 0;
            code_analysis->total_functions++;
        }
    }

    // Check for loops
    if (is_loop_start(trimmed_line)) {
        code_analysis->total_loops++;
        if (code_analysis->total_functions > 0) {
            code_analysis->functions[code_analysis->total_functions-1].loop_depth++;
            if (code_analysis->functions[code_analysis->total_functions-1].loop_depth > code_analysis->max_nested_loops) {
                code_analysis->max_nested_loops = code_analysis->functions[code_analysis->total_functions-1].loop_depth;
            }
        }
    }

    // Check for recursion
    if (code_analysis->total_functions > 0) {
        const char* current_func = code_analysis->functions[code_analysis->total_functions-1].name;
        if (is_recursive_call(trimmed_line, current_func)) {
            code_analysis->functions[code_analysis->total_functions-1].has_recursion = 1;
            if (!code_analysis->functions[code_analysis->total_functions-1].is_recursive) {
                code_analysis->recursive_functions++;
                code_analysis->functions[code_analysis->total_functions-1].is_recursive = 1;
            }
        }
    }

    // Check for malloc calls
    if (is_malloc_call(trimmed_line)) {
        code_analysis->malloc_calls++;
    }

    // Check for array declarations
    if (is_array_declaration(trimmed_line)) {
        code_analysis->array_declarations++;
    }

    detect_classic_patterns(trimmed_line, analysis);

    free(trimmed_line);
}

// Add a function to check if any classic pattern was detected
static int has_classic_pattern(struct json_object* analysis) {
    struct json_object* pattern;
    const char* keys[] = {"bubble_sort_pattern", "merge_sort_pattern", "quick_sort_pattern", "binary_search_pattern", "fibonacci_dp_pattern", "dijkstra_pattern", "knapsack_pattern", "sieve_pattern", "bfs_pattern", "power_pattern"};
    for (int i = 0; i < 10; i++) {
        if (json_object_object_get_ex(analysis, keys[i], &pattern) && json_object_get_boolean(pattern)) {
            return 1;
        }
    }
    return 0;
}

// Main analysis function
__attribute__((visibility("default"))) char* analyze_complexity(const char* code) {
    if (!code) {
        return NULL;
    }
    
    // Initialize analysis structures
    struct json_object* analysis = json_object_new_object();
    CodeAnalysis code_analysis = {0};
    
    // Split code into lines and analyze each line
    char* code_copy = strdup(code);
    char* line = strtok(code_copy, "\n");
    while (line) {
        analyze_line(line, analysis, &code_analysis);
        line = strtok(NULL, "\n");
    }
    free(code_copy);
    
    // If classic pattern detected, use classic complexity
    if (has_classic_pattern(analysis)) {
        const char* best_time = "O(1)", *avg_time = "O(1)", *worst_time = "O(1)";
        const char* best_space = "O(1)", *avg_space = "O(1)", *worst_space = "O(1)";
        if (json_object_object_get_ex(analysis, "bubble_sort_pattern", NULL)) {
            best_time = "O(n)"; avg_time = worst_time = "O(n²)";
        } else if (json_object_object_get_ex(analysis, "merge_sort_pattern", NULL)) {
            best_time = avg_time = worst_time = "O(n log n)";
            best_space = avg_space = worst_space = "O(n)";
        } else if (json_object_object_get_ex(analysis, "quick_sort_pattern", NULL)) {
            best_time = avg_time = "O(n log n)"; worst_time = "O(n²)";
            best_space = avg_space = "O(log n)"; worst_space = "O(n)";
        } else if (json_object_object_get_ex(analysis, "binary_search_pattern", NULL)) {
            best_time = "O(1)"; avg_time = worst_time = "O(log n)";
        } else if (json_object_object_get_ex(analysis, "fibonacci_dp_pattern", NULL)) {
            best_time = avg_time = worst_time = "O(n)";
            best_space = avg_space = worst_space = "O(n)";
        } else if (json_object_object_get_ex(analysis, "dijkstra_pattern", NULL)) {
            best_time = avg_time = worst_time = "O((V + E) log V)";
            best_space = avg_space = worst_space = "O(V)";
        } else if (json_object_object_get_ex(analysis, "knapsack_pattern", NULL)) {
            best_time = avg_time = worst_time = "O(nW)";
            best_space = avg_space = worst_space = "O(nW)";
        } else if (json_object_object_get_ex(analysis, "sieve_pattern", NULL)) {
            best_time = avg_time = worst_time = "O(n log log n)";
            best_space = avg_space = worst_space = "O(n)";
        } else if (json_object_object_get_ex(analysis, "bfs_pattern", NULL)) {
            best_time = avg_time = worst_time = "O(V + E)";
            best_space = avg_space = worst_space = "O(V)";
        } else if (json_object_object_get_ex(analysis, "power_pattern", NULL)) {
            best_time = avg_time = worst_time = "O(log n)";
        }
        json_object_object_add(analysis, "best_time", json_object_new_string(best_time));
        json_object_object_add(analysis, "average_time", json_object_new_string(avg_time));
        json_object_object_add(analysis, "worst_time", json_object_new_string(worst_time));
        json_object_object_add(analysis, "best_space", json_object_new_string(best_space));
        json_object_object_add(analysis, "average_space", json_object_new_string(avg_space));
        json_object_object_add(analysis, "worst_space", json_object_new_string(worst_space));
        char* details = malloc(1024);
        snprintf(details, 1024, "Classic algorithm detected. Time: %s (best), %s (avg), %s (worst). Space: %s (best), %s (avg), %s (worst).", best_time, avg_time, worst_time, best_space, avg_space, worst_space);
        json_object_object_add(analysis, "implementation_details", json_object_new_string(details));
        free(details);
    } else {
        // Determine time complexity
        const char* best_time;
        const char* avg_time;
        const char* worst_time;
        
        if (code_analysis.recursive_functions > 0) {
            best_time = "O(n)";
            avg_time = "O(n log n)";
            worst_time = "O(2^n)";
        } else if (code_analysis.max_nested_loops >= 3) {
            char complexity[20];
            snprintf(complexity, sizeof(complexity), "O(n^%d)", code_analysis.max_nested_loops);
            best_time = strdup(complexity);
            avg_time = strdup(complexity);
            worst_time = strdup(complexity);
        } else if (code_analysis.max_nested_loops == 2) {
            best_time = "O(n²)";
            avg_time = "O(n²)";
            worst_time = "O(n²)";
        } else if (code_analysis.total_loops > 0) {
            best_time = "O(n)";
            avg_time = "O(n)";
            worst_time = "O(n)";
        } else {
            best_time = "O(1)";
            avg_time = "O(1)";
            worst_time = "O(1)";
        }
        
        // Determine space complexity
        const char* best_space;
        const char* avg_space;
        const char* worst_space;
        
        if (code_analysis.malloc_calls > 0 || code_analysis.array_declarations > 0) {
            if (code_analysis.recursive_functions > 0) {
                best_space = "O(n)";
                avg_space = "O(n)";
                worst_space = "O(n)";
            } else {
                best_space = "O(n)";
                avg_space = "O(n)";
                worst_space = "O(n)";
            }
        } else {
            best_space = "O(1)";
            avg_space = "O(1)";
            worst_space = "O(1)";
        }
        
        // Add complexity information to analysis
        json_object_object_add(analysis, "best_time", json_object_new_string(best_time));
        json_object_object_add(analysis, "average_time", json_object_new_string(avg_time));
        json_object_object_add(analysis, "worst_time", json_object_new_string(worst_time));
        
        json_object_object_add(analysis, "best_space", json_object_new_string(best_space));
        json_object_object_add(analysis, "average_space", json_object_new_string(avg_space));
        json_object_object_add(analysis, "worst_space", json_object_new_string(worst_space));
        
        // Add implementation details
        char* details = malloc(1024);
        snprintf(details, 1024, 
            "The code contains %d functions, %d loops (max nesting: %d), %d recursive functions, "
            "%d dynamic memory allocations, and %d array declarations. "
            "Time complexity ranges from %s (best) to %s (worst), with an average case of %s. "
            "Space complexity ranges from %s (best) to %s (worst), with an average case of %s.",
            code_analysis.total_functions, code_analysis.total_loops, code_analysis.max_nested_loops,
            code_analysis.recursive_functions, code_analysis.malloc_calls, code_analysis.array_declarations,
            best_time, worst_time, avg_time,
            best_space, worst_space, avg_space);
        json_object_object_add(analysis, "implementation_details", json_object_new_string(details));
        free(details);
    }
    
    // Convert analysis to string
    const char* analysis_str = json_object_to_json_string(analysis);
    char* result = strdup(analysis_str);
    
    // Clean up
    json_object_put(analysis);
    
    return result;
} 
