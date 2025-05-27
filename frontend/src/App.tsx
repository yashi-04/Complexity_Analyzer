import React, { useState } from 'react';
import ComplexityChart from './components/ComplexityChart';
import ComplexityLog from './components/ComplexityLog';
import AnalysisResults from './components/AnalysisResults';

const BACKEND_URL = 'http://localhost:5001'; // Updated port to match backend

function App() {
  const [code, setCode] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [analysis, setAnalysis] = useState<{
    line_analysis: Array<{
      line: number;
      code: string;
      complexity: string | null;
      explanation: string | null;
    }>;
    best_time: string;
    average_time: string;
    worst_time: string;
    best_space: string;
    average_space: string;
    worst_space: string;
    implementation_details: string;
  } | null>(null);

  const analyzeCode = async () => {
    if (!code.trim()) {
      setError('Please enter some code to analyze');
      return;
    }

    try {
      setLoading(true);
      setError(null);
      
      const response = await fetch(`${BACKEND_URL}/analyze`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ code }),
      });

      if (!response.ok) {
        const errorData = await response.json().catch(() => null);
        throw new Error(errorData?.message || `Server error: ${response.status}`);
      }

      const data = await response.json();
      setAnalysis({
        line_analysis: data.logs.map((log: any) => ({
          line: log.line,
          code: log.code || '',
          complexity: log.complexity || null,
          explanation: log.explanation || null
        })),
        best_time: data.bestTimeComplexity,
        average_time: data.averageTimeComplexity,
        worst_time: data.worstTimeComplexity,
        best_space: data.spaceComplexity,
        average_space: data.spaceComplexity,
        worst_space: data.spaceComplexity,
        implementation_details: 'Implementation details not available'
      });
    } catch (error) {
      console.error('Error analyzing code:', error);
      if (error instanceof TypeError && error.message === 'Failed to fetch') {
        setError('Unable to connect to the server. Please make sure the backend is running.');
      } else {
        setError(error instanceof Error ? error.message : 'An error occurred');
      }
    } finally {
      setLoading(false);
    }
  };

  const getComplexityDescription = (complexity: string) => {
    switch (complexity) {
      case 'O(1)': return 'Constant time';
      case 'O(log n)': return 'Logarithmic time';
      case 'O(n)': return 'Linear time';
      case 'O(n log n)': return 'Linearithmic time';
      case 'O(n²)': return 'Quadratic time';
      case 'O(n³)': return 'Cubic time';
      case 'O(n!)': return 'Factorial time';
      case 'O(sqrt(n))': return 'Square root time';
      case 'O(2^n)': return 'Exponential time';
      default: return '';
    }
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-indigo-50 via-purple-50 to-pink-50 py-8">
      <div className="container mx-auto px-4 py-8">
        <h1 className="text-6xl font-bold text-center mb-8 title-text title-gradient">
          CompO-Meter
        </h1>
        <div className="text-center mb-12">
          <p className="text-gray-600 text-lg">
            Analyze your code's time and space complexity with precision
          </p>
        </div>
        
        <div className="mb-8 bg-white/80 backdrop-blur-sm rounded-2xl shadow-xl p-6 transform transition-all duration-300 hover:shadow-2xl border border-white/20">
          <textarea
            className="w-full h-64 p-4 border-2 border-indigo-100 rounded-xl focus:ring-2 focus:ring-indigo-500 focus:border-transparent font-mono text-sm transition-all duration-300 bg-white/50"
            value={code}
            onChange={(e) => setCode(e.target.value)}
            placeholder="Enter your C code here..."
          />
          <div className="mt-4 flex items-center space-x-4">
            <button
              className="px-8 py-3 bg-gradient-to-r from-indigo-600 via-purple-600 to-pink-600 text-white rounded-xl hover:from-indigo-700 hover:via-purple-700 hover:to-pink-700 transition-all duration-300 transform hover:scale-105 disabled:opacity-50 disabled:transform-none font-semibold shadow-lg hover:shadow-xl"
              onClick={analyzeCode}
              disabled={loading}
            >
              {loading ? (
                <span className="flex items-center">
                  <svg className="animate-spin -ml-1 mr-3 h-5 w-5 text-white" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                    <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4"></circle>
                    <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                  </svg>
                  Analyzing...
                </span>
              ) : 'Analyze Code'}
            </button>
            {error && (
              <div className="flex items-center text-red-500 bg-red-50/80 backdrop-blur-sm px-4 py-2 rounded-lg border border-red-100">
                <svg className="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                  <path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                </svg>
                {error}
              </div>
            )}
          </div>
        </div>

        {loading && (
          <div className="text-center py-12">
            <div className="inline-block animate-spin rounded-full h-12 w-12 border-4 border-indigo-500 border-t-transparent"></div>
            <p className="mt-4 text-gray-600 text-lg">Analyzing your code...</p>
          </div>
        )}

        {analysis && !loading && (
          <div className="space-y-8 animate-fade-in">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-8">
              <div className="bg-white/80 backdrop-blur-sm p-8 rounded-2xl shadow-xl border-l-4 border-indigo-500 transform transition-all duration-300 hover:shadow-2xl border border-white/20">
                <h2 className="text-2xl font-bold mb-6 text-gray-800">Time Complexity</h2>
                <div className="space-y-6">
                  <div className="p-4 bg-gradient-to-r from-green-50 to-emerald-50 rounded-xl border border-green-100">
                    <h3 className="text-sm font-semibold text-green-700 mb-2">Best Case</h3>
                    <p className="text-3xl font-mono text-green-600">
                      {analysis.best_time}
                    </p>
                    <p className="text-sm text-green-600 mt-2">
                      {getComplexityDescription(analysis.best_time)}
                    </p>
                  </div>
                  <div className="p-4 bg-gradient-to-r from-blue-50 to-indigo-50 rounded-xl border border-blue-100">
                    <h3 className="text-sm font-semibold text-blue-700 mb-2">Average Case</h3>
                    <p className="text-3xl font-mono text-blue-600">
                      {analysis.average_time}
                    </p>
                    <p className="text-sm text-blue-600 mt-2">
                      {getComplexityDescription(analysis.average_time)}
                    </p>
                  </div>
                  <div className="p-4 bg-gradient-to-r from-red-50 to-pink-50 rounded-xl border border-red-100">
                    <h3 className="text-sm font-semibold text-red-700 mb-2">Worst Case</h3>
                    <p className="text-3xl font-mono text-red-600">
                      {analysis.worst_time}
                    </p>
                    <p className="text-sm text-red-600 mt-2">
                      {getComplexityDescription(analysis.worst_time)}
                    </p>
                  </div>
                </div>
              </div>
              <div className="bg-white/80 backdrop-blur-sm p-8 rounded-2xl shadow-xl border-l-4 border-green-500 transform transition-all duration-300 hover:shadow-2xl border border-white/20">
                <h2 className="text-2xl font-bold mb-4 text-gray-800">Space Complexity</h2>
                <div className="space-y-6">
                  <div className="p-4 bg-gradient-to-r from-green-50 to-emerald-50 rounded-xl border border-green-100">
                    <h3 className="text-sm font-semibold text-green-700 mb-2">Best Case</h3>
                    <p className="text-3xl font-mono text-green-600">
                      {analysis.best_space}
                    </p>
                    <p className="text-sm text-green-600 mt-2">
                      {getComplexityDescription(analysis.best_space)}
                    </p>
                  </div>
                  <div className="p-4 bg-gradient-to-r from-blue-50 to-indigo-50 rounded-xl border border-blue-100">
                    <h3 className="text-sm font-semibold text-blue-700 mb-2">Average Case</h3>
                    <p className="text-3xl font-mono text-blue-600">
                      {analysis.average_space}
                    </p>
                    <p className="text-sm text-blue-600 mt-2">
                      {getComplexityDescription(analysis.average_space)}
                    </p>
                  </div>
                  <div className="p-4 bg-gradient-to-r from-red-50 to-pink-50 rounded-xl border border-red-100">
                    <h3 className="text-sm font-semibold text-red-700 mb-2">Worst Case</h3>
                    <p className="text-3xl font-mono text-red-600">
                      {analysis.worst_space}
                    </p>
                    <p className="text-sm text-red-600 mt-2">
                      {getComplexityDescription(analysis.worst_space)}
                    </p>
                  </div>
                </div>
              </div>
            </div>

            <div className="space-y-8">
              <div className="bg-white/80 backdrop-blur-sm p-8 rounded-2xl shadow-xl transform transition-all duration-300 hover:shadow-2xl border border-white/20">
                <h2 className="text-2xl font-bold mb-6 text-gray-800">Complexity Visualization</h2>
                <ComplexityChart
                  bestTime={analysis.best_time}
                  averageTime={analysis.average_time}
                  worstTime={analysis.worst_time}
                  bestSpace={analysis.best_space}
                  averageSpace={analysis.average_space}
                  worstSpace={analysis.worst_space}
                />
              </div>

              <div className="bg-white/80 backdrop-blur-sm p-8 rounded-2xl shadow-xl transform transition-all duration-300 hover:shadow-2xl border border-white/20">
                <h2 className="text-2xl font-bold mb-6 text-gray-800">Line-by-Line Analysis</h2>
                <ComplexityLog logs={analysis.line_analysis} />
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

export default App;
