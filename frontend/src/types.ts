export interface LineAnalysis {
  line: number;
  time_complexity: string;
  space_complexity: string;
  explanation: string;
}

export interface AnalysisData {
  line_analysis: Array<{
    line: number;
    time_complexity: string;
    space_complexity: string;
    explanation: string;
  }>;
  best_time: string;
  average_time: string;
  worst_time: string;
  best_space: string;
  average_space: string;
  worst_space: string;
  implementation_details: string;
} 