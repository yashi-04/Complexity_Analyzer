import React from 'react';

interface LogEntry {
  line: number;
  code: string;
  complexity: string | null;
  explanation: string | null;
}

interface ComplexityLogProps {
  logs: LogEntry[];
}

const ComplexityLog: React.FC<ComplexityLogProps> = ({ logs = [] }) => {
  // Filter out entries with no complexity (comments, brackets, etc.)
  const filteredLogs = logs.filter(log => log.complexity !== null);

  const getComplexityColor = (complexity: string) => {
    if (complexity === 'O(n log n)') return 'text-purple-600';
    if (complexity === 'O(n)' || complexity === 'O(n/2)') return 'text-blue-600';
    if (complexity === 'O(n²)') return 'text-red-600';
    if (complexity === 'O(log n)') return 'text-green-600';
    if (complexity === 'O(1)') return 'text-gray-600';
    return 'text-blue-600';
  };

  return (
    <div className="bg-white rounded-lg shadow-md p-6">
      <h2 className="text-xl font-semibold mb-4 text-gray-800">Line-by-Line Analysis</h2>
      <div className="space-y-2">
        {filteredLogs.map((log) => (
          <div
            key={log.line}
            className="flex items-start space-x-4 p-3 rounded-lg hover:bg-gray-50"
          >
            <div className="flex-shrink-0 w-12 text-right text-gray-500 font-mono">
              {log.line}
            </div>
            <div className="flex-grow">
              <div className="font-mono text-sm bg-gray-50 p-2 rounded">
                {log.code}
              </div>
              <div className="mt-1 flex items-center space-x-2">
                <span className={`text-sm font-medium ${getComplexityColor(log.complexity || '')}`}>
                  {log.complexity}
                </span>
                {log.explanation && (
                  <span className="text-sm text-gray-600">
                    ({log.explanation})
                  </span>
                )}
              </div>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
};

export default ComplexityLog; 