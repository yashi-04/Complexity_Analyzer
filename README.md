# CompO-meter: C Code Complexity Analyzer

## Description
CompO-meter is a sophisticated tool that analyzes C code to determine its time and space complexity. It provides a modern web interface where developers can paste their C code and receive detailed complexity analysis reports.

## Features
- **Modern Web Interface**: Built with React and Tailwind CSS
- **Real-time Analysis**: Instant complexity analysis of C code
- **Advanced Pattern Detection**:
  - Classic algorithm recognition (merge sort, quick sort, binary search, etc.)
  - Generic structural analysis for custom code
  - Recursive function detection
  - Loop nesting analysis
  - Memory allocation tracking
- **Detailed Reports**: 
  - Overall Time and Space Complexity
  - Best, Average, and Worst-case complexities
  - Line-by-line complexity breakdown
  - Explanations for complexity calculations


## Project Structure
```
CompO-meter/
├── frontend/          # React-based web interface
├── backend/           # Flask server
├── analyzer/          # Core C-based complexity analyzer
```

## Prerequisites
- Python 3.8+
- Node.js 16+
- GCC/Clang compiler
- CMake 3.10+
- libclang development libraries
- json-c library (for JSON parsing)

## Installation

1. **Clone the repository**
```bash
git clone https://github.com/yashi-04/CompO-Meter.git
cd CompO-meter
```

2. **Set up Python environment**
```bash
python -m venv venv
source venv/bin/activate  # On Unix/macOS
# or
.\venv\Scripts\activate  # On Windows
```

3. **Install Python dependencies**
```bash
cd backend
pip install -r requirements.txt
```

4. **Install Node.js dependencies**
```bash
cd ../frontend
npm install
```

5. **Install json-c library**
```bash
# On macOS with Homebrew
brew install json-c

# On Ubuntu/Debian
sudo apt-get install libjson-c-dev


```

6. **Compile the C analyzer**
```bash
cd ../../analyzer
make
```

## Running the Application

1. **Start the backend server**
```bash
cd backend
python app.py
```

2. **Start the frontend development server**
```bash
cd frontend
npm start
```

3. **Access the application**
Open your browser and navigate to `http://localhost:3000`

## Usage

1. **Input Code**: Paste your C code into the text editor
2. **Analyze**: Click the "Analyze" button
3. **View Results**: 
   - Overall complexity metrics
   - Best, average, and worst-case complexities
   - Line-by-line breakdown
   - Complexity explanations
   - Algorithm pattern detection results
4. **Estimated time to run the program**


## Contributrs
- Yashi Sharma
- Mansi Ranjan Gupta
- Nishtha Gupta

## License
This project is licensed under the MIT License - see the LICENSE file for details.

