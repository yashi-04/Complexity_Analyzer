import React, { useState } from 'react';

interface CodeEditorProps {
  onAnalyze: (code: string) => void;
}

const CodeEditor: React.FC<CodeEditorProps> = ({ onAnalyze }) => {
  const [code, setCode] = useState('');

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (code.trim()) {
      onAnalyze(code);
    }
  };

  return (
    <div className="bg-white shadow sm:rounded-lg">
      <div className="px-4 py-5 sm:p-6">
        <h3 className="text-lg leading-6 font-medium text-gray-900">
          Enter C Code
        </h3>
        <div className="mt-2 max-w-xl text-sm text-gray-500">
          <p>Paste your C code below for complexity analysis.</p>
        </div>
        <form onSubmit={handleSubmit} className="mt-5">
          <div className="mt-1">
            <textarea
              rows={10}
              className="shadow-sm focus:ring-indigo-500 focus:border-indigo-500 block w-full sm:text-sm border-gray-300 rounded-md font-mono"
              value={code}
              onChange={(e) => setCode(e.target.value)}
              placeholder="// Enter your C code here..."
            />
          </div>
          <div className="mt-5">
            <button
              type="submit"
              disabled={!code.trim()}
              className="inline-flex items-center px-4 py-2 border border-transparent text-sm font-medium rounded-md shadow-sm text-white bg-indigo-600 hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500 disabled:bg-gray-400 disabled:cursor-not-allowed"
            >
              Analyze Code
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};

export default CodeEditor; 