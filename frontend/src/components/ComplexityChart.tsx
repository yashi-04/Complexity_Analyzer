import React from 'react';
import { Line } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

interface ComplexityChartProps {
  bestTime: string;
  averageTime: string;
  worstTime: string;
  bestSpace: string;
  averageSpace: string;
  worstSpace: string;
}

const ComplexityChart: React.FC<ComplexityChartProps> = ({
  bestTime,
  averageTime,
  worstTime,
  bestSpace,
  averageSpace,
  worstSpace,
}) => {
  // Generate more granular data points for better visualization
  const inputSizes = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024];
  
  const getComplexityValue = (complexity: string, n: number) => {
    // Normalize values to make them more comparable
    const normalize = (value: number) => Math.log2(value + 1);
    
    switch (complexity) {
      case 'O(1)':
        return 0;
      case 'O(log n)':
        return normalize(Math.log2(n));
      case 'O(n)':
        return normalize(n);
      case 'O(n log n)':
        return normalize(n * Math.log2(n));
      case 'O(n²)':
        return normalize(n * n);
      case 'O(2^n)':
        return normalize(Math.pow(2, Math.min(n, 10))); // Cap exponential growth
      default:
        return normalize(n);
    }
  };

  // Helper function to calculate factorial
  const factorial = (n: number): number => {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
  };

  const timeData = {
    labels: inputSizes.map(n => n.toString()),
    datasets: [
      {
        label: `Best Case: ${bestTime}`,
        data: inputSizes.map(n => getComplexityValue(bestTime, n)),
        borderColor: 'rgb(75, 192, 192)',  // Teal
        backgroundColor: 'rgba(75, 192, 192, 0.5)',
        tension: 0.4,
        pointRadius: 0,
        pointHoverRadius: 0,
        borderWidth: 2,
        borderDash: [],  // Solid line
      },
      {
        label: `Average Case: ${averageTime}`,
        data: inputSizes.map(n => getComplexityValue(averageTime, n)),
        borderColor: 'rgb(255, 159, 64)',  // Orange
        backgroundColor: 'rgba(255, 159, 64, 0.5)',
        tension: 0.4,
        pointRadius: 0,
        pointHoverRadius: 0,
        borderWidth: 2,
        borderDash: [5, 5],  // Dashed line
      },
      {
        label: `Worst Case: ${worstTime}`,
        data: inputSizes.map(n => getComplexityValue(worstTime, n)),
        borderColor: 'rgb(255, 99, 132)',  // Red
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
        tension: 0.4,
        pointRadius: 0,
        pointHoverRadius: 0,
        borderWidth: 2,
        borderDash: [2, 2],  // Dotted line
      },
    ],
  };

  const spaceData = {
    labels: inputSizes.map(n => n.toString()),
    datasets: [
      {
        label: `Best Case Space: ${bestSpace}`,
        data: inputSizes.map(n => getComplexityValue(bestSpace, n)),
        borderColor: 'rgb(75, 192, 192)',  // Teal
        backgroundColor: 'rgba(75, 192, 192, 0.5)',
        tension: 0.4,
        pointRadius: 0,
        pointHoverRadius: 0,
        borderWidth: 2,
        borderDash: [],  // Solid line
      },
      {
        label: `Average Case Space: ${averageSpace}`,
        data: inputSizes.map(n => getComplexityValue(averageSpace, n)),
        borderColor: 'rgb(255, 159, 64)',  // Orange
        backgroundColor: 'rgba(255, 159, 64, 0.5)',
        tension: 0.4,
        pointRadius: 0,
        pointHoverRadius: 0,
        borderWidth: 2,
        borderDash: [5, 5],  // Dashed line
      },
      {
        label: `Worst Case Space: ${worstSpace}`,
        data: inputSizes.map(n => getComplexityValue(worstSpace, n)),
        borderColor: 'rgb(255, 99, 132)',  // Red
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
        tension: 0.4,
        pointRadius: 0,
        pointHoverRadius: 0,
        borderWidth: 2,
        borderDash: [2, 2],  // Dotted line
      },
    ],
  };

  const options = {
    responsive: true,
    maintainAspectRatio: true,
    aspectRatio: 1,
    plugins: {
      legend: {
        position: 'top' as const,
        labels: {
          font: {
            size: 14
          }
        }
      },
      title: {
        display: true,
        text: 'Complexity Analysis',
        font: {
          size: 16
        }
      },
      tooltip: {
        enabled: true,
        mode: 'index' as const,
        intersect: false,
        callbacks: {
          label: function(context: any) {
            let label = context.dataset.label || '';
            if (label) {
              label += ': ';
            }
            if (context.parsed.y !== null) {
              // Calculate estimated execution time in nanoseconds
              const n = parseInt(context.label);
              const complexity = context.dataset.label.split(': ')[1];
              const estimatedTime = getEstimatedTime(complexity, n);
              label += `\nEstimated time: ${formatTime(estimatedTime)}`;
            }
            return label;
          }
        }
      }
    },
    scales: {
      x: {
        title: {
          display: true,
          text: 'Input Size (n)',
          font: {
            size: 14
          }
        },
        grid: {
          display: false,
        },
        ticks: {
          display: false, // Remove x-axis numbers
        },
        min: 0,
      },
      y: {
        title: {
          display: true,
          text: 'Complexity (log scale)',
          font: {
            size: 14
          }
        },
        beginAtZero: true,
        grid: {
          display: false,
        },
        ticks: {
          display: false, // Remove y-axis numbers
        },
        min: 0,
      },
    },
    interaction: {
      mode: 'nearest' as const,
      axis: 'x' as const,
      intersect: false
    }
  };

  // Helper function to calculate estimated execution time
  const getEstimatedTime = (complexity: string, n: number): number => {
    // Assuming each basic operation takes 1 nanosecond
    const baseTime = 1e-9; // 1 nanosecond
    
    switch (complexity) {
      case 'O(1)':
        return baseTime;
      case 'O(log n)':
        return baseTime * Math.log2(n);
      case 'O(n)':
        return baseTime * n;
      case 'O(n log n)':
        return baseTime * n * Math.log2(n);
      case 'O(n²)':
        return baseTime * n * n;
      case 'O(2^n)':
        return baseTime * Math.pow(2, Math.min(n, 10)); // Cap exponential growth
      default:
        return baseTime * n;
    }
  };

  // Helper function to format time
  const formatTime = (seconds: number): string => {
    if (seconds < 1e-9) {
      return `${(seconds * 1e12).toFixed(2)} picoseconds`;
    } else if (seconds < 1e-6) {
      return `${(seconds * 1e9).toFixed(2)} nanoseconds`;
    } else if (seconds < 1e-3) {
      return `${(seconds * 1e6).toFixed(2)} microseconds`;
    } else if (seconds < 1) {
      return `${(seconds * 1e3).toFixed(2)} milliseconds`;
    } else if (seconds < 60) {
      return `${seconds.toFixed(2)} seconds`;
    } else if (seconds < 3600) {
      return `${(seconds / 60).toFixed(2)} minutes`;
    } else {
      return `${(seconds / 3600).toFixed(2)} hours`;
    }
  };

  return (
    <div className="bg-white rounded-lg shadow-md p-6">
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <div className="w-full aspect-square">
          <Line 
            data={timeData} 
            options={{
              ...options,
              plugins: {
                ...options.plugins,
                title: {
                  display: true,
                  text: 'Time Complexity Analysis',
                }
              }
            }}
          />
        </div>
        <div className="w-full aspect-square">
          <Line 
            data={spaceData} 
            options={{
              ...options,
              plugins: {
                ...options.plugins,
                title: {
                  display: true,
                  text: 'Space Complexity Analysis',
                }
              }
            }}
          />
        </div>
      </div>

      {/* Estimated Execution Times Container */}
      <div className="mt-8">
        <h3 className="text-xl font-semibold mb-4 text-gray-800">Estimated Execution Times</h3>
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          {/* Best Case */}
          <div className="bg-gradient-to-r from-green-50 to-emerald-50 rounded-xl p-4 border border-green-100">
            <h4 className="text-sm font-semibold text-green-700 mb-2">Best Case ({bestTime})</h4>
            <div className="space-y-2">
              {[10, 100, 1000, 10000].map(n => (
                <div key={`best-${n}`} className="flex justify-between items-center text-sm">
                  <span className="text-gray-600">n = {n}:</span>
                  <span className="font-mono text-green-600">
                    {formatTime(getEstimatedTime(bestTime, n))}
                  </span>
                </div>
              ))}
            </div>
          </div>

          {/* Average Case */}
          <div className="bg-gradient-to-r from-blue-50 to-indigo-50 rounded-xl p-4 border border-blue-100">
            <h4 className="text-sm font-semibold text-blue-700 mb-2">Average Case ({averageTime})</h4>
            <div className="space-y-2">
              {[10, 100, 1000, 10000].map(n => (
                <div key={`avg-${n}`} className="flex justify-between items-center text-sm">
                  <span className="text-gray-600">n = {n}:</span>
                  <span className="font-mono text-blue-600">
                    {formatTime(getEstimatedTime(averageTime, n))}
                  </span>
                </div>
              ))}
            </div>
          </div>

          {/* Worst Case */}
          <div className="bg-gradient-to-r from-red-50 to-pink-50 rounded-xl p-4 border border-red-100">
            <h4 className="text-sm font-semibold text-red-700 mb-2">Worst Case ({worstTime})</h4>
            <div className="space-y-2">
              {[10, 100, 1000, 10000].map(n => (
                <div key={`worst-${n}`} className="flex justify-between items-center text-sm">
                  <span className="text-gray-600">n = {n}:</span>
                  <span className="font-mono text-red-600">
                    {formatTime(getEstimatedTime(worstTime, n))}
                  </span>
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default ComplexityChart; 
