import os
import json
import ctypes
from flask import Flask, request, jsonify
from flask_cors import CORS
from reportlab.pdfgen import canvas
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.platypus import Table, TableStyle
import tempfile
import sys
import traceback
import math
import re

app = Flask(__name__)
CORS(app, resources={r"/*": {"origins": "*"}})

# Get the absolute path to the analyzer library
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIBRARY_PATH = os.path.join(BASE_DIR, 'analyzer', 'libanalyzer.dylib')

# Define the function signature for the C analyzer
analyze_complexity_func = None

# Load the C analyzer library
try:
    print("Attempting to load analyzer library...")
    print(f"Current working directory: {os.getcwd()}")
    print(f"Library path: {LIBRARY_PATH}")
    analyzer_lib = ctypes.CDLL(LIBRARY_PATH)
    
    # Set up the function signature
    analyze_complexity_func = analyzer_lib.analyze_complexity
    analyze_complexity_func.argtypes = [ctypes.c_char_p]
    analyze_complexity_func.restype = ctypes.c_char_p
    
    print("Successfully loaded analyzer library")
except Exception as e:
    print(f"Error loading analyzer library: {e}", file=sys.stderr)
    print(traceback.format_exc())
    analyzer_lib = None

@app.route('/analyze', methods=['POST'])
def analyze_code():
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'error': 'No code provided'}), 400

        code = data['code']
        print(f"Received code for analysis: {code[:100]}...")
        
        # Call the C analyzer
        if analyzer_lib and analyze_complexity_func:
            print("Calling C analyzer...")
            try:
                # Convert Python string to C string
                c_code = ctypes.c_char_p(code.encode('utf-8'))
                # Call the analyze function from the C library
                result = analyze_complexity_func(c_code)
                if not result:
                    raise Exception("C analyzer returned NULL")
                # Convert result back to Python string
                result_str = ctypes.string_at(result).decode('utf-8')
                print(f"Analysis result: {result_str}")
                analysis_result = json.loads(result_str)
                
                # Process code lines and skip whitespace
                code_lines = [line.strip() for line in code.split('\n')]
                non_empty_lines = [(i + 1, line) for i, line in enumerate(code_lines) if line.strip()]
                analysis_result['code_lines'] = code_lines
                
                # Check for classic algorithm patterns
                has_quick_sort = any('quickSort' in line or 'partition' in line for line in code_lines)
                has_merge_sort = any('mergeSort' in line or 'merge(' in line for line in code_lines)
                has_bubble_sort = any('bubbleSort' in line for line in code_lines)
                has_binary_search = any('binarySearch' in line for line in code_lines)
                has_division_loop = any('/2' in line or '/ 2' in line for line in code_lines if 'for' in line)
                has_recursion = any('(' in line and ')' in line and not any(keyword in line for keyword in ['if', 'for', 'while']) for line in code_lines)
                has_tree_recursion = any('+' in line and '(' in line and ')' in line and not any(keyword in line for keyword in ['if', 'for', 'while']) for line in code_lines)
                
                # Determine best, average, and worst case complexities
                if has_quick_sort:
                    # Quick Sort pattern
                    worst_time = 'O(n²)'    # Worst case when array is already sorted
                    best_time = 'O(n log n)' # Best case with good pivot selection
                    avg_time = 'O(n log n)'  # Average case
                    worst_space = 'O(n)'     # Worst case call stack depth
                    best_space = 'O(log n)'  # Best case call stack depth
                    avg_space = 'O(log n)'   # Average case call stack depth
                elif has_merge_sort:
                    # Merge Sort pattern
                    worst_time = 'O(n log n)' # All cases same for Merge Sort
                    best_time = 'O(n log n)'
                    avg_time = 'O(n log n)'
                    worst_space = 'O(n)'     # Space for temporary array
                    best_space = 'O(n)'
                    avg_space = 'O(n)'
                elif has_bubble_sort:
                    # Bubble Sort pattern
                    worst_time = 'O(n²)'    # All cases same for Bubble Sort
                    best_time = 'O(n²)'
                    avg_time = 'O(n²)'
                    worst_space = 'O(1)'     # In-place sorting
                    best_space = 'O(1)'
                    avg_space = 'O(1)'
                elif has_binary_search:
                    # Binary Search pattern
                    worst_time = 'O(log n)'  # All cases same for Binary Search
                    best_time = 'O(1)'       # Best case when element is at middle
                    avg_time = 'O(log n)'
                    worst_space = 'O(1)'     # Iterative implementation
                    best_space = 'O(1)'
                    avg_space = 'O(1)'
                elif has_tree_recursion:
                    # For tree recursion (like Fibonacci)
                    worst_time = 'O(2^n)'  # Exponential growth due to multiple recursive calls
                    best_time = 'O(1)'     # Best case when n ≤ 1 (no recursion)
                    avg_time = 'O(2^n)'    # Average case still exponential
                    worst_space = 'O(n)'   # Call stack depth is n
                    best_space = 'O(1)'    # Best case when n ≤ 1
                    avg_space = 'O(n)'     # Average case
                elif has_recursion:
                    # For simple linear recursion (like factorial)
                    worst_time = 'O(n)'  # Each recursive call does O(1) work, n calls total
                    best_time = 'O(n)'   # Same path for all inputs
                    avg_time = 'O(n)'    # Same path for all inputs
                    worst_space = 'O(n)' # Call stack depth is n
                    best_space = 'O(n)'  # Same for all inputs
                    avg_space = 'O(n)'   # Same for all inputs
                elif has_division_loop:
                    worst_time = 'O(n/2)'  # Will be simplified to O(n) in Big O notation
                    best_time = 'O(1)'     # Best case when n ≤ 0 (loop doesn't execute)
                    avg_time = 'O(n/2)'    # Average case
                    worst_space = 'O(1)'   # No extra space needed
                    best_space = 'O(1)'
                    avg_space = 'O(1)'
                else:
                    worst_time = analysis_result.get('worst_time', 'O(1)')
                    best_time = analysis_result.get('best_time', worst_time)
                    avg_time = analysis_result.get('average_time', worst_time)
                    worst_space = analysis_result.get('worst_space', 'O(1)')
                    best_space = analysis_result.get('best_space', worst_space)
                    avg_space = analysis_result.get('average_space', worst_space)
                
                # Calculate estimated execution times for different input sizes
                input_sizes = [10, 100, 1000, 10000]
                time_estimates = {
                    'best': {str(n): format_time(estimate_execution_time(best_time, n)) for n in input_sizes},
                    'average': {str(n): format_time(estimate_execution_time(avg_time, n)) for n in input_sizes},
                    'worst': {str(n): format_time(estimate_execution_time(worst_time, n)) for n in input_sizes}
                }
                
                # Format the response to match the frontend expectations
                return jsonify({
                    'timeComplexity': worst_time,  # Default to worst case for overall
                    'spaceComplexity': worst_space,
                    'bestTimeComplexity': best_time,
                    'averageTimeComplexity': avg_time,
                    'worstTimeComplexity': worst_time,
                    'bestSpaceComplexity': best_space,
                    'averageSpaceComplexity': avg_space,
                    'worstSpaceComplexity': worst_space,
                    'timeEstimates': time_estimates,
                    'logs': [
                        {
                            'line': line_num,
                            'code': line,
                            'complexity': get_line_complexity(line, analysis_result),
                            'explanation': get_line_explanation(line, analysis_result)
                        }
                        for line_num, line in non_empty_lines
                    ]
                })
                
            except Exception as e:
                print(f"Error in C analyzer: {e}", file=sys.stderr)
                print(traceback.format_exc())
                return jsonify({'error': f'Analysis failed: {str(e)}'}), 500
        else:
            print("Using fallback analysis (analyzer library not loaded)")
            return jsonify({'error': 'Analyzer library not loaded'}), 500

    except Exception as e:
        print(f"Error in analyze_code: {str(e)}", file=sys.stderr)
        print(traceback.format_exc())
        return jsonify({'error': str(e)}), 500

def get_line_complexity(line, analysis_result):
    """Determine the complexity of a single line of code."""
    line = line.strip()
    
    # Skip empty lines, comments, and brackets
    if (not line or 
        line.startswith('//') or 
        line.startswith('/*') or 
        line.startswith('*/') or
        line in ['{', '}', ';']):
        return None
    
    # Skip function declarations and variable declarations
    if line.startswith('void ') or line.startswith('int ') or line.startswith('#include'):
        return None
    
    # Check for classic algorithm patterns
    if 'quickSort' in line or 'partition' in line:
        if 'quickSort' in line:
            return 'O(n log n)'  # Best/average case for Quick Sort
        return 'O(n)'  # Partition operation
    
    if 'mergeSort' in line or 'merge(' in line:
        if 'mergeSort' in line:
            return 'O(n log n)'  # Merge Sort complexity
        return 'O(n)'  # Merge operation
    
    if 'bubbleSort' in line:
        return 'O(n²)'  # Bubble Sort complexity
    
    if 'binarySearch' in line:
        return 'O(log n)'  # Binary Search complexity
    
    # Check for recursive calls
    if '(' in line and ')' in line and not any(keyword in line for keyword in ['if', 'for', 'while']):
        # Check if it's a recursive call (function calling itself)
        func_match = re.search(r'(\w+)\s*\(', line)
        if func_match:
            func_name = func_match.group(1)
            # Look for function declaration in previous lines
            for prev_line in analysis_result.get('code_lines', []):
                if prev_line.startswith(('int ', 'void ')) and func_name in prev_line:
                    # Check for tree recursion (multiple recursive calls)
                    if '+' in line and func_name in line.split('+')[1]:
                        return 'O(2^n)'  # Tree recursion like Fibonacci
                    return 'O(n)'  # Basic linear recursion
    
    # Check for loop patterns
    if 'for' in line:
        # Check for division in loop condition
        if '/2' in line or '/ 2' in line:
            return 'O(n/2)'  # This will be simplified to O(n) in the overall analysis
        if 'i++' in line or 'i--' in line:
            return 'O(n)'
        elif 'i *= 2' in line or 'i /= 2' in line:
            return 'O(log n)'
    
    # Check for nested loops
    if 'for' in line and 'for' in line.split('{')[0]:
        return 'O(n²)'
    
    # Default for simple operations
    return 'O(1)'

def get_line_explanation(line, analysis_result):
    """Generate an explanation for the complexity of a line."""
    line = line.strip()
    
    # Skip empty lines, comments, and brackets
    if (not line or 
        line.startswith('//') or 
        line.startswith('/*') or 
        line.startswith('*/') or
        line in ['{', '}', ';']):
        return None
    
    # Skip function declarations and variable declarations
    if line.startswith('void ') or line.startswith('int ') or line.startswith('#include'):
        return None
    
    # Generate explanations based on the line content
    if 'quickSort' in line or 'partition' in line:
        if 'quickSort' in line:
            return "Quick Sort recursive call (O(n log n) best/average case, O(n²) worst case)"
        return "Partition operation (O(n) time complexity)"
    
    if 'mergeSort' in line or 'merge(' in line:
        if 'mergeSort' in line:
            return "Merge Sort recursive call (O(n log n) time complexity)"
        return "Merge operation (O(n) time complexity)"
    
    if 'bubbleSort' in line:
        return "Bubble Sort operation (O(n²) time complexity)"
    
    if 'binarySearch' in line:
        return "Binary Search operation (O(log n) time complexity)"
    
    if 'for' in line:
        if '/2' in line or '/ 2' in line:
            return "Linear iteration over n/2 elements (simplified to O(n) in Big O notation)"
        if 'i++' in line or 'i--' in line:
            return "Linear iteration over n elements"
        elif 'i *= 2' in line or 'i /= 2' in line:
            return "Logarithmic iteration (halving/doubling)"
        elif 'for' in line.split('{')[0]:
            return "Nested loop iteration"
    
    if 'while' in line:
        return "Loop iteration with condition"
    
    # Check for recursive calls
    if '(' in line and ')' in line and not any(keyword in line for keyword in ['if', 'for', 'while']):
        func_match = re.search(r'(\w+)\s*\(', line)
        if func_match:
            func_name = func_match.group(1)
            # Look for function declaration in previous lines
            for prev_line in analysis_result.get('code_lines', []):
                if prev_line.startswith(('int ', 'void ')) and func_name in prev_line:
                    # Check for tree recursion (multiple recursive calls)
                    if '+' in line and func_name in line.split('+')[1]:
                        return f"Tree recursive call to {func_name} (exponential growth, O(2^n) time complexity)"
                    return f"Recursive call to {func_name} (adds O(n) time and space complexity)"
        return "Function call"
    
    # Default for simple operations
    return "Simple operation (constant time)"

def generate_complexity_summary(analysis_result):
    """Generate a human-readable summary of the complexity analysis."""
    summary_parts = []
    
    # Add time complexity explanation
    time_best = analysis_result.get('best_time', 'O(1)')
    time_avg = analysis_result.get('average_time', 'O(1)')
    time_worst = analysis_result.get('worst_time', 'O(1)')
    
    summary_parts.append(f"Time Complexity: The function has a best case of {time_best}, average case of {time_avg}, and worst case of {time_worst}.")
    
    # Add space complexity explanation
    space_best = analysis_result.get('best_space', 'O(1)')
    space_avg = analysis_result.get('average_space', 'O(1)')
    space_worst = analysis_result.get('worst_space', 'O(1)')
    
    summary_parts.append(f"Space Complexity: The function uses {space_best} space in the best case, {space_avg} in the average case, and {space_worst} in the worst case.")
    
    # Add specific implementation details if available
    if 'implementation_details' in analysis_result:
        summary_parts.append(f"\nImplementation Details: {analysis_result['implementation_details']}")
    
    # Add optimization suggestions if available
    if 'optimization_suggestions' in analysis_result:
        summary_parts.append(f"\nOptimization Suggestions: {analysis_result['optimization_suggestions']}")
    
    return " ".join(summary_parts)

def estimate_execution_time(complexity, n=1000):
    """Estimate execution time based on complexity and input size."""
    # Assuming each basic operation takes 1 nanosecond
    base_time = 1e-9  # 1 nanosecond
    
    if complexity == 'O(1)':
        return base_time
    elif complexity == 'O(log n)':
        return base_time * math.log2(n)
    elif complexity == 'O(n)':
        return base_time * n
    elif complexity == 'O(n log n)':
        return base_time * n * math.log2(n)
    elif complexity == 'O(n²)':
        return base_time * n * n
    elif complexity == 'O(2^n)':
        return base_time * math.pow(2, min(n, 10))  # Cap exponential growth
    else:
        return base_time * n

def format_time(seconds):
    """Format time in appropriate units."""
    if seconds < 1e-9:
        return f"{seconds * 1e12:.2f} picoseconds"
    elif seconds < 1e-6:
        return f"{seconds * 1e9:.2f} nanoseconds"
    elif seconds < 1e-3:
        return f"{seconds * 1e6:.2f} microseconds"
    elif seconds < 1:
        return f"{seconds * 1e3:.2f} milliseconds"
    elif seconds < 60:
        return f"{seconds:.2f} seconds"
    elif seconds < 3600:
        return f"{seconds / 60:.2f} minutes"
    else:
        return f"{seconds / 3600:.2f} hours"

if __name__ == '__main__':
    app.run(port=5001, debug=True) 
