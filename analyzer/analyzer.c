#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <json-c/json.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LINES 1000
#define MAX_FUNCTIONS 50
#define MAX_JSON_SIZE 16384

// Define the LogEntry struct
typedef struct {
    int line;
    char* code;
    char* complexity;
    char* explanation;
} LogEntry;

// Define the AnalysisResult struct
typedef struct {
    char* bestTimeComplexity;
    char* averageTimeComplexity;
    char* worstTimeComplexity;
    char* spaceComplexity;
    LogEntry* logs;
    int logCount;
    int maxLogs;
} AnalysisResult;

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

// Forward declarations of all functions
static int is_loop_start(const char* line);
static int is_function_start(const char* line);
static int is_recursive_call(const char* line, const char* func_name);
static int is_malloc_call(const char* line);
static int is_array_declaration(const char* line);
static void detect_classic_patterns(const char* line, struct json_object* analysis);
static int is_constant_loop(const char* line);
static int is_simple_loop(const char* line);
static int is_division_loop(const char* line);
static int is_nested_loop(const char* line, int current_depth);
static int is_linear_nested_loop(const char* line, const char* prev_line);
static const char* determine_loop_complexity(const char* line, int loop_depth, const char* prev_line);
static const char* determine_recursive_complexity(const char* line, int recursive_calls);
static int get_loop_nesting_level(const char* line);
static char* generate_complexity_graph(const char* complexity);
static char* safe_strdup(const char* str);
static void cleanup_static_variables(void);
static void analyze_line(const char* line, struct json_object* analysis, CodeAnalysis* code_analysis);
static int has_classic_pattern(struct json_object* analysis);
static const char* determine_overall_complexity(int max_nesting, int total_loops);

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

static void detect_classic_patterns(const char* line, struct json_object* analysis) {
    // Merge Sort
    if (strstr(line, "merge") || 
        (strstr(line, "L[") && strstr(line, "R[")) || 
        (strstr(line, "n1") && strstr(line, "n2")) ||
        (strstr(line, "left") && strstr(line, "right"))) {
        json_object_object_add(analysis, "merge_sort_pattern", json_object_new_boolean(1));
    }
    
    // Quick Sort
    if (strstr(line, "pivot") || 
        (strstr(line, "partition") && strstr(line, "return")) || 
        (strstr(line, "i+1") && strstr(line, "j-1")) ||
        (strstr(line, "swap") && strstr(line, "pivot"))) {
        json_object_object_add(analysis, "quick_sort_pattern", json_object_new_boolean(1));
    }
    
    // Bubble Sort
    if ((strstr(line, "for") && strstr(line, "j") && strstr(line, "i+1")) ||
        (strstr(line, "swap") && strstr(line, "arr[j]") && strstr(line, "arr[j+1]"))) {
        json_object_object_add(analysis, "bubble_sort_pattern", json_object_new_boolean(1));
    }
    
    // Binary Search
    if ((strstr(line, "return") && ((strstr(line, "mid - 1") || strstr(line, "mid + 1")) || 
        (strstr(line, "high = mid") || strstr(line, "low = mid")))) ||
        (strstr(line, "mid = (low + high) / 2"))) {
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

static int is_constant_loop(const char* line) {
    // First check for explicit constant values
    const char* constant_patterns[] = {
        "i < 5", "i <= 5", "i > 5", "i >= 5",
        "i < 10", "i <= 10", "i > 10", "i >= 10",
        "i < 100", "i <= 100", "i > 100", "i >= 100",
        "i < 3", "i <= 3", "i > 3", "i >= 3",
        "i < 7", "i <= 7", "i > 7", "i >= 7",
        "i < 20", "i <= 20", "i > 20", "i >= 20",
        "i < 50", "i <= 50", "i > 50", "i >= 50",
        "i < 1000", "i <= 1000", "i > 1000", "i >= 1000",
        "i < 8", "i <= 8", "i > 8", "i >= 8",  // Common for bit operations
        "i < 32", "i <= 32", "i > 32", "i >= 32",  // Common for 32-bit operations
        "i < 64", "i <= 64", "i > 64", "i >= 64"   // Common for 64-bit operations
    };
    
    size_t num_patterns = sizeof(constant_patterns)/sizeof(constant_patterns[0]);
    for (size_t i = 0; i < num_patterns; i++) {
        if (strstr(line, constant_patterns[i])) {
            return 1;
        }
    }
    
    // Check for array length based constant loops
    if (strstr(line, "sizeof") || strstr(line, "strlen")) {
        return 1;
    }
    
    // Check for numeric literals in loop conditions
    char* condition = strstr(line, "for");
    if (condition) {
        size_t len = strlen(condition);
        // Look for numeric literals in the condition
        for (size_t i = 0; i < len; i++) {
            if (isdigit(condition[i])) {
                // Check if it's a standalone number (not part of a variable name)
                if (i == 0 || !isalnum(condition[i-1])) {
                    if (i == len-1 || !isalnum(condition[i+1])) {
                        return 1;
                    }
                }
            }
        }
    }
    
    // Check for variable bounds (n, m, etc.)
    if (strstr(line, "i < n") || strstr(line, "i <= n") ||
        strstr(line, "i < m") || strstr(line, "i <= m") ||
        strstr(line, "j < n") || strstr(line, "j <= n") ||
        strstr(line, "j < m") || strstr(line, "j <= m") ||
        strstr(line, "k < n") || strstr(line, "k <= n") ||
        strstr(line, "k < m") || strstr(line, "k <= m")) {
        return 0;  // Not constant if bound by a variable
    }
    
    return 0;
}

static int is_simple_loop(const char* line) {
    // Check for basic increment/decrement loops
    return (strstr(line, "i++") || strstr(line, "i--") || 
            strstr(line, "i += 1") || strstr(line, "i -= 1") ||
            strstr(line, "i += 2") || strstr(line, "i -= 2") ||
            strstr(line, "i += 3") || strstr(line, "i -= 3"));
}

static int is_division_loop(const char* line) {
    // Check for division-based loops (logarithmic)
    return (strstr(line, "i /= 2") || strstr(line, "i /= 2") ||
            strstr(line, "n/2") || strstr(line, "n / 2") ||
            strstr(line, "i /= 3") || strstr(line, "i /= 4") ||
            strstr(line, "i /= 5") || strstr(line, "i /= 10"));
}

static int is_nested_loop(const char* line, int current_depth) {
    // Check if this is a nested loop
    return (strstr(line, "for") || strstr(line, "while")) && current_depth > 0;
}

static int is_linear_nested_loop(const char* line, const char* prev_line) {
    // Check for patterns like:
    // for (i = 0; i < n; i++)
    //     for (j = i; j < n; j++)
    if (!prev_line) return 0;
    
    return (strstr(line, "j = i") || strstr(line, "j=i") ||
            strstr(line, "k = i") || strstr(line, "k=i") ||
            strstr(line, "j = i+1") || strstr(line, "j=i+1"));
}

static const char* determine_loop_complexity(const char* line, int loop_depth, const char* prev_line) {
    // First check if it's a constant-time loop
    if (is_constant_loop(line)) {
        return "O(1)";  // Fixed number of iterations means constant time
    }
    
    // Then check for other patterns
    if (is_division_loop(line)) {
        return "O(log n)";
    } else if (loop_depth > 1) {
        // For nested loops, check if the inner loop is constant
        if (is_constant_loop(line)) {
            return "O(n)";  // If inner loop is constant, overall is linear
        }
        // Check for linear nested loops (like in selection sort)
        if (is_linear_nested_loop(line, prev_line)) {
            return "O(n²/2)";  // More precise than O(n²)
        }
        // For nested loops with variable bounds, use appropriate complexity
        char complexity[20];
        snprintf(complexity, sizeof(complexity), "O(n^%d)", loop_depth);
        return strdup(complexity);
    } else if (is_simple_loop(line)) {
        return "O(n)";
    }
    return "O(1)";
}

static const char* determine_recursive_complexity(const char* line, int recursive_calls) {
    if (recursive_calls > 1) {
        // If there are multiple recursive calls, it's likely exponential
        return "O(2^n)";
    } else if (recursive_calls == 1) {
        // Single recursive call with division (like binary search)
        if (strstr(line, "/2") || strstr(line, "mid")) {
            return "O(log n)";
        }
        // Single recursive call (like linear recursion)
        return "O(n)";
    }
    return "O(1)";
}

static int get_loop_nesting_level(const char* line) {
    static int current_level = 0;
    static int max_level = 0;
    static int in_loop = 0;
    
    const char* p = line;
    while (*p) {
        if (*p == '{') {
            if (in_loop) {
                current_level++;
                if (current_level > max_level) {
                    max_level = current_level;
                }
            }
        } else if (*p == '}') {
            if (in_loop) {
                current_level--;
                if (current_level < 0) current_level = 0;
            }
        } else if (strstr(p, "for") || strstr(p, "while")) {
            in_loop = 1;
        }
        p++;
    }
    return max_level;
}

static char* generate_complexity_graph(const char* complexity) {
    char* graph = malloc(1024);
    if (!graph) return NULL;
    
    // Initialize graph with empty space
    memset(graph, ' ', 1024);
    graph[1023] = '\0';
    
    // Set up graph dimensions
    int width = 50;
    int height = 10;
    
    // Draw axes
    for (int i = 0; i < height; i++) {
        graph[i * (width + 1)] = '|';
    }
    for (int i = 0; i < width; i++) {
        graph[height * (width + 1) + i] = '-';
    }
    
    // Draw complexity curve based on type
    if (strcmp(complexity, "O(1)") == 0) {
        // Constant - horizontal line
        for (int i = 1; i < width; i++) {
            graph[1 * (width + 1) + i] = '-';
        }
    } else if (strcmp(complexity, "O(log n)") == 0) {
        // Logarithmic - curved line
        for (int i = 1; i < width; i++) {
            int y = height - 1 - (int)(log2(i) * (height - 2) / log2(width));
            if (y >= 0 && y < height) {
                graph[y * (width + 1) + i] = '*';
            }
        }
    } else if (strcmp(complexity, "O(n)") == 0) {
        // Linear - diagonal line
        for (int i = 1; i < width; i++) {
            int y = height - 1 - (i * (height - 2) / width);
            if (y >= 0 && y < height) {
                graph[y * (width + 1) + i] = '*';
            }
        }
    } else if (strcmp(complexity, "O(n log n)") == 0) {
        // Linearithmic - curved line
        for (int i = 1; i < width; i++) {
            int y = height - 1 - (int)((i * log2(i)) * (height - 2) / (width * log2(width)));
            if (y >= 0 && y < height) {
                graph[y * (width + 1) + i] = '*';
            }
        }
    } else if (strstr(complexity, "O(n^2)") || strstr(complexity, "O(n²)")) {
        // Quadratic - parabolic curve
        for (int i = 1; i < width; i++) {
            int y = height - 1 - (int)((i * i) * (height - 2) / (width * width));
            if (y >= 0 && y < height) {
                graph[y * (width + 1) + i] = '*';
            }
        }
    } else if (strstr(complexity, "O(2^n)")) {
        // Exponential - steep curve
        for (int i = 1; i < width/2; i++) {
            int y = height - 1 - (int)(pow(2, i) * (height - 2) / pow(2, width/2));
            if (y >= 0 && y < height) {
                graph[y * (width + 1) + i] = '*';
            }
        }
    }
    
    // Add labels
    char* label = malloc(100);
    snprintf(label, 100, "\nTime Complexity: %s\n", complexity);
    strcat(graph, label);
    free(label);
    
    return graph;
}

static char* safe_strdup(const char* str) {
    if (!str) return NULL;
    char* new_str = malloc(strlen(str) + 1);
    if (!new_str) return NULL;
    strcpy(new_str, str);
    return new_str;
}

static void cleanup_static_variables(void) {
    static char* prev_line = NULL;
    if (prev_line) {
        free(prev_line);
        prev_line = NULL;
    }
}

static void analyze_line(const char* line, struct json_object* analysis, CodeAnalysis* code_analysis) {
    static char* prev_line = NULL;
    static int max_nesting_level = 0;
    static int loop_count = 0;
    static int in_loop = 0;
    
    if (!line || line[0] == '\0' || line[0] == '/' || line[0] == '*') {
        return;
    }

    // Create a copy of the line for analysis
    char* trimmed_line = safe_strdup(line);
    if (!trimmed_line) {
        return;
    }

    // Remove leading/trailing whitespace
    char* p = trimmed_line;
    while (isspace(*p)) p++;
    char* end = p + strlen(p) - 1;
    while (end > p && isspace(*end)) *end-- = '\0';

    // Check for loops
    if (is_loop_start(trimmed_line)) {
        code_analysis->total_loops++;
        loop_count++;
        in_loop = 1;
        
        // Update nesting level
        int current_level = get_loop_nesting_level(trimmed_line);
        if (current_level > max_nesting_level) {
            max_nesting_level = current_level;
            code_analysis->max_nested_loops = current_level;
        }
    }

    // Reset loop tracking when we exit a loop
    if (strstr(trimmed_line, "}")) {
        if (in_loop) {
            loop_count--;
            if (loop_count <= 0) {
                in_loop = 0;
                loop_count = 0;
            }
        }
    }

    // Update previous line
    if (prev_line) {
        free(prev_line);
    }
    prev_line = safe_strdup(trimmed_line);

    // Check for function start
    if (is_function_start(trimmed_line)) {
        char func_name[50] = {0};
        sscanf(trimmed_line, "%*s %s", func_name);
        char* paren = strchr(func_name, '(');
        if (paren) *paren = '\0';
        
        if (code_analysis->total_functions < MAX_FUNCTIONS) {
            strncpy(code_analysis->functions[code_analysis->total_functions].name, 
                   func_name, sizeof(func_name) - 1);
            code_analysis->functions[code_analysis->total_functions].is_recursive = 0;
            code_analysis->functions[code_analysis->total_functions].loop_depth = 0;
            code_analysis->functions[code_analysis->total_functions].has_recursion = 0;
            code_analysis->total_functions++;
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
                
                const char* complexity = determine_recursive_complexity(trimmed_line, 
                    code_analysis->recursive_functions);
                if (complexity) {
                    json_object_object_add(analysis, "recursive_complexity", 
                        json_object_new_string(complexity));
                }
            }
        }
    }

    // Check for malloc and array declarations
    if (is_malloc_call(trimmed_line)) {
        code_analysis->malloc_calls++;
    }
    if (is_array_declaration(trimmed_line)) {
        code_analysis->array_declarations++;
    }

    detect_classic_patterns(trimmed_line, analysis);

    free(trimmed_line);
}

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

static const char* determine_overall_complexity(int max_nesting, int total_loops) {
    if (max_nesting > 1) {
        // For nested loops, use the nesting level to determine complexity
        char* complexity = malloc(20);
        if (complexity) {
            snprintf(complexity, 20, "O(n^%d)", max_nesting);
            return complexity;
        }
    } else if (total_loops > 0) {
        return "O(n)";
    }
    return "O(1)";
}

__attribute__((visibility("default"))) char* analyze_complexity(const char* code) {
    if (!code) {
        return NULL;
    }
    
    struct json_object* analysis = json_object_new_object();
    if (!analysis) {
        return NULL;
    }

    CodeAnalysis code_analysis = {0};
    
    char* code_copy = safe_strdup(code);
    if (!code_copy) {
        json_object_put(analysis);
        return NULL;
    }

    char* line = strtok(code_copy, "\n");
    while (line) {
        analyze_line(line, analysis, &code_analysis);
        line = strtok(NULL, "\n");
    }

    free(code_copy);
    
    // First check for classic algorithm patterns
    if (has_classic_pattern(analysis)) {
        const char* best_time = "O(1)", *avg_time = "O(1)", *worst_time = "O(1)";
        const char* best_space = "O(1)", *avg_space = "O(1)", *worst_space = "O(1)";
        const char* pattern_name = "Unknown";
        
        if (json_object_object_get_ex(analysis, "merge_sort_pattern", NULL)) {
            best_time = avg_time = worst_time = "O(n log n)";
            best_space = avg_space = worst_space = "O(n)";
            pattern_name = "Merge Sort";
        } else if (json_object_object_get_ex(analysis, "quick_sort_pattern", NULL)) {
            best_time = avg_time = "O(n log n)"; worst_time = "O(n²)";
            best_space = avg_space = "O(log n)"; worst_space = "O(n)";
            pattern_name = "Quick Sort";
        } else if (json_object_object_get_ex(analysis, "bubble_sort_pattern", NULL)) {
            best_time = "O(n)"; avg_time = worst_time = "O(n²)";
            pattern_name = "Bubble Sort";
        } else if (json_object_object_get_ex(analysis, "binary_search_pattern", NULL)) {
            best_time = "O(1)"; avg_time = worst_time = "O(log n)";
            pattern_name = "Binary Search";
        }
        
        json_object_object_add(analysis, "best_time", json_object_new_string(best_time));
        json_object_object_add(analysis, "average_time", json_object_new_string(avg_time));
        json_object_object_add(analysis, "worst_time", json_object_new_string(worst_time));
        json_object_object_add(analysis, "best_space", json_object_new_string(best_space));
        json_object_object_add(analysis, "average_space", json_object_new_string(avg_space));
        json_object_object_add(analysis, "worst_space", json_object_new_string(worst_space));
        
        char* details = malloc(1024);
        if (details) {
            snprintf(details, 1024, 
                "Detected %s pattern. Time complexity: %s (best), %s (avg), %s (worst). "
                "Space complexity: %s (best), %s (avg), %s (worst). "
                "The code contains %d functions, %d loops (max nesting: %d), %d recursive functions, "
                "%d dynamic memory allocations, and %d array declarations.",
                pattern_name, best_time, avg_time, worst_time,
                best_space, avg_space, worst_space,
                code_analysis.total_functions, code_analysis.total_loops, code_analysis.max_nested_loops,
                code_analysis.recursive_functions, code_analysis.malloc_calls, code_analysis.array_declarations);
            json_object_object_add(analysis, "implementation_details", json_object_new_string(details));
            free(details);
        }
    } else {
        // If no pattern detected, use nesting level for complexity
        const char* complexity = determine_overall_complexity(code_analysis.max_nested_loops, code_analysis.total_loops);
        
        // Set time complexities based on nesting level
        json_object_object_add(analysis, "best_time", json_object_new_string(complexity));
        json_object_object_add(analysis, "average_time", json_object_new_string(complexity));
        json_object_object_add(analysis, "worst_time", json_object_new_string(complexity));
        
        // Set space complexity
        const char* space_complexity = "O(1)";
        if (code_analysis.malloc_calls > 0 || code_analysis.array_declarations > 0) {
            space_complexity = "O(n)";
        }
        json_object_object_add(analysis, "best_space", json_object_new_string(space_complexity));
        json_object_object_add(analysis, "average_space", json_object_new_string(space_complexity));
        json_object_object_add(analysis, "worst_space", json_object_new_string(space_complexity));
        
        // Add implementation details
        char* details = malloc(1024);
        if (details) {
            snprintf(details, 1024, 
                "The code contains %d functions, %d loops (max nesting: %d), %d recursive functions, "
                "%d dynamic memory allocations, and %d array declarations. "
                "Time complexity is %s due to %d levels of nested loops. "
                "Space complexity is %s.",
                code_analysis.total_functions, code_analysis.total_loops, code_analysis.max_nested_loops,
                code_analysis.recursive_functions, code_analysis.malloc_calls, code_analysis.array_declarations,
                complexity, code_analysis.max_nested_loops, space_complexity);
            json_object_object_add(analysis, "implementation_details", json_object_new_string(details));
            free(details);
        }
        
        if (complexity && strcmp(complexity, "O(1)") != 0 && strcmp(complexity, "O(n)") != 0) {
            free((void*)complexity);
        }
    }
    
    // Convert analysis to string
    const char* analysis_str = json_object_to_json_string(analysis);
    char* result = safe_strdup(analysis_str);
    
    // Clean up
    json_object_put(analysis);
    
    // Clean up static variables
    cleanup_static_variables();
    
    return result;
}

void free_analysis_result(AnalysisResult* result) {
    if (!result) return;

    if (result->bestTimeComplexity) free(result->bestTimeComplexity);
    if (result->averageTimeComplexity) free(result->averageTimeComplexity);
    if (result->worstTimeComplexity) free(result->worstTimeComplexity);
    if (result->spaceComplexity) free(result->spaceComplexity);

    if (result->logs) {
        for (int i = 0; i < result->logCount; i++) {
            if (result->logs[i].code) free(result->logs[i].code);
            if (result->logs[i].complexity) free(result->logs[i].complexity);
            if (result->logs[i].explanation) free(result->logs[i].explanation);
        }
        free(result->logs);
    }
} 
