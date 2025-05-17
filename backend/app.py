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

app = Flask(__name__)
CORS(app)

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
                
                # Extract complexity information from the analysis
                time_complexity = {
                    'best': analysis_result.get('best_time', 'O(1)'),
                    'average': analysis_result.get('average_time', 'O(1)'),
                    'worst': analysis_result.get('worst_time', 'O(1)')
                }
                
                space_complexity = {
                    'best': analysis_result.get('best_space', 'O(1)'),
                    'average': analysis_result.get('average_space', 'O(1)'),
                    'worst': analysis_result.get('worst_space', 'O(1)')
                }
                
                # Generate summary based on the analysis
                summary = generate_complexity_summary(analysis_result)
                
                return jsonify({
                    'time_complexity': time_complexity,
                    'space_complexity': space_complexity,
                    'summary': summary
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

@app.route('/generate-report', methods=['POST'])
def generate_report():
    try:
        data = request.get_json()
        if not data or 'analysis' not in data:
            return jsonify({'error': 'No analysis data provided'}), 400

        analysis = data['analysis']
        
        with tempfile.NamedTemporaryFile(delete=False, suffix='.pdf') as tmp:
            c = canvas.Canvas(tmp.name, pagesize=letter)
            width, height = letter

            c.setFont("Helvetica-Bold", 16)
            c.drawString(50, height - 50, "Code Complexity Analysis Report")

            c.setFont("Helvetica-Bold", 14)
            y = height - 100
            c.drawString(50, y, "Overall Complexity Analysis")
            y -= 30
            data = [
                ["Case", "Time Complexity", "Space Complexity"],
                ["Best Case", analysis['time_complexity']['best'], analysis['space_complexity']['best']],
                ["Average Case", analysis['time_complexity']['average'], analysis['space_complexity']['average']],
                ["Worst Case", analysis['time_complexity']['worst'], analysis['space_complexity']['worst']]
            ]
            table = Table(data, colWidths=[100, 150, 150])
            table.setStyle(TableStyle([
                ('BACKGROUND', (0, 0), (-1, 0), colors.grey),
                ('TEXTCOLOR', (0, 0), (-1, 0), colors.whitesmoke),
                ('ALIGN', (0, 0), (-1, -1), 'CENTER'),
                ('FONTNAME', (0, 0), (-1, 0), 'Helvetica-Bold'),
                ('FONTSIZE', (0, 0), (-1, 0), 12),
                ('BOTTOMPADDING', (0, 0), (-1, 0), 12),
                ('BACKGROUND', (0, 1), (-1, -1), colors.beige),
                ('TEXTCOLOR', (0, 1), (-1, -1), colors.black),
                ('FONTNAME', (0, 1), (-1, -1), 'Helvetica'),
                ('FONTSIZE', (0, 1), (-1, -1), 10),
                ('GRID', (0, 0), (-1, -1), 1, colors.black)
            ]))
            table.wrapOn(c, width - 100, height)
            table.drawOn(c, 50, y - 100)

            y -= 150
            c.setFont("Helvetica-Bold", 14)
            c.drawString(50, y, "Summary Explanation")
            y -= 30
            c.setFont("Helvetica", 12)
            c.drawString(50, y, analysis['summary'])

            c.save()
            with open(tmp.name, 'rb') as pdf_file:
                pdf_data = pdf_file.read()
            os.unlink(tmp.name)
            return jsonify({
                'pdf_data': pdf_data.hex(),
                'filename': 'complexity_analysis.pdf'
            })
    except Exception as e:
        print(f"Error in generate_report: {str(e)}", file=sys.stderr)
        print(traceback.format_exc())
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5001) 
