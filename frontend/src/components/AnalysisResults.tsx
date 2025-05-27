import React from 'react';
import { AnalysisData } from '../types';
import ComplexityChart from './ComplexityChart';

interface AnalysisResultsProps {
  data: AnalysisData;
}

const AnalysisResults: React.FC<AnalysisResultsProps> = ({ data }) => {
  return (
    <div className="bg-white shadow sm:rounded-lg">
      <div className="px-4 py-5 sm:p-6">
        <h3 className="text-lg leading-6 font-medium text-gray-900">
          Analysis Results
        </h3>
        
        <div className="mt-5">
          <div className="bg-gray-50 px-4 py-5 sm:grid sm:grid-cols-3 sm:gap-4 sm:px-6">
            <dt className="text-sm font-medium text-gray-500">Best Case Time Complexity</dt>
            <dd className="mt-1 text-sm text-gray-900 sm:mt-0 sm:col-span-2">
              {data.best_time}
            </dd>
          </div>
          <div className="bg-white px-4 py-5 sm:grid sm:grid-cols-3 sm:gap-4 sm:px-6">
            <dt className="text-sm font-medium text-gray-500">Average Case Time Complexity</dt>
            <dd className="mt-1 text-sm text-gray-900 sm:mt-0 sm:col-span-2">
              {data.average_time}
            </dd>
          </div>
          <div className="bg-gray-50 px-4 py-5 sm:grid sm:grid-cols-3 sm:gap-4 sm:px-6">
            <dt className="text-sm font-medium text-gray-500">Worst Case Time Complexity</dt>
            <dd className="mt-1 text-sm text-gray-900 sm:mt-0 sm:col-span-2">
              {data.worst_time}
            </dd>
          </div>
          <div className="bg-white px-4 py-5 sm:grid sm:grid-cols-3 sm:gap-4 sm:px-6">
            <dt className="text-sm font-medium text-gray-500">Space Complexity</dt>
            <dd className="mt-1 text-sm text-gray-900 sm:mt-0 sm:col-span-2">
              <div className="space-y-2">
                <div>
                  <span className="font-medium text-green-600">Best Case:</span> {data.best_space}
                </div>
                <div>
                  <span className="font-medium text-blue-600">Average Case:</span> {data.average_space}
                </div>
                <div>
                  <span className="font-medium text-red-600">Worst Case:</span> {data.worst_space}
                </div>
              </div>
            </dd>
          </div>
        </div>

        {/* Complexity Charts */}
        <div className="mt-8">
          <h4 className="text-md font-medium text-gray-900 mb-4">Complexity Visualization</h4>
          <ComplexityChart
            bestTime={data.best_time}
            averageTime={data.average_time}
            worstTime={data.worst_time}
            bestSpace={data.best_space}
            averageSpace={data.average_space}
            worstSpace={data.worst_space}
          />
        </div>

        {/* Implementation Details */}
        <div className="mt-8">
          <h4 className="text-md font-medium text-gray-900 mb-4">Implementation Details</h4>
          <div className="bg-gray-50 p-4 rounded-lg">
            <p className="text-sm text-gray-700">{data.implementation_details}</p>
          </div>
        </div>

        {/* Line-by-Line Analysis */}
        <div className="mt-8">
          <h4 className="text-md font-medium text-gray-900">Line-by-Line Analysis</h4>
          <div className="mt-4 flow-root">
            <div className="-mx-4 -my-2 overflow-x-auto sm:-mx-6 lg:-mx-8">
              <div className="inline-block min-w-full py-2 align-middle sm:px-6 lg:px-8">
                <table className="min-w-full divide-y divide-gray-300">
                  <thead>
                    <tr>
                      <th scope="col" className="py-3.5 pl-4 pr-3 text-left text-sm font-semibold text-gray-900 sm:pl-0">
                        Line
                      </th>
                      <th scope="col" className="px-3 py-3.5 text-left text-sm font-semibold text-gray-900">
                        Time Complexity
                      </th>
                      <th scope="col" className="px-3 py-3.5 text-left text-sm font-semibold text-gray-900">
                        Space Complexity
                      </th>
                      <th scope="col" className="px-3 py-3.5 text-left text-sm font-semibold text-gray-900">
                        Explanation
                      </th>
                    </tr>
                  </thead>
                  <tbody className="divide-y divide-gray-200">
                    {data.line_analysis.map((line, index) => (
                      <tr key={index} className="hover:bg-gray-50">
                        <td className="whitespace-nowrap py-4 pl-4 pr-3 text-sm font-medium text-gray-900 sm:pl-0">
                          {line.line}
                        </td>
                        <td className="whitespace-nowrap px-3 py-4 text-sm text-gray-500">
                          {line.time_complexity}
                        </td>
                        <td className="whitespace-nowrap px-3 py-4 text-sm text-gray-500">
                          {line.space_complexity}
                        </td>
                        <td className="px-3 py-4 text-sm text-gray-500">
                          {line.explanation}
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default AnalysisResults; 
