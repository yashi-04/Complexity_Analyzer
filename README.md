# **Project Documentation: Complexity Analyzer for C Code**  

---

## **1. Project Overview**  
### **1.1 Description**  
A static analysis tool that estimates **time and space complexity** of C programs by analyzing code structure (loops, recursion, memory ops) without execution.  

### **1.2 Key Features**  
- Function-level complexity reports  
- Loop nesting detection  
- Recursion identification  
- Memory allocation tracking (malloc/arrays)  
- Big-O notation estimation  
- Text + JSON output formats  

---

## **2. Technology Stack**  
| **Component**       | **Technology/Tool** | **Usage %** |  
|---------------------|---------------------|-------------|  
| Core Language       | C (ANSI C11)        | 100%        |  
| Compiler            | GCC                 | 30%         |  
| Build System        | Make                | 15%         |  
| Version Control     | Git                 | 10%         |  
| Debugging           | Valgrind            | 5%          |  

---

## **3. System Architecture**  
```
flowchart 
    A[Parser Module] -->|Cleaned Code| B[Analyzer Module]  
    B -->|Analysis Metrics| C[Reporter Module]  
```  

### **3.1 Modules**  
1. **Parser**  
   - Input: C source file  
   - Output: Structured code (functions, loops, cleaned lines)  
   - Key Functions: `analyze_file()`, `is_function_start()`  

2. **Analyzer**  
   - Input: Parsed code  
   - Output: `CodeAnalysis` struct (loop depth, recursion flags, etc.)  
   - Key Functions: `is_loop_start()`, `is_recursive_call()`  

3. **Reporter**  
   - Input: Analysis results  
   - Output: Text/JSON reports  
   - Key Functions: `print_analysis_report()`, `estimate_complexity()`  

---

## **4. Team Roles & Responsibilities**  
### **4.1 Detailed Breakdown**  
| **Member**   | **Role**          | **Responsibilities**                                                         | **Deliverables**                          |  
|--------------|-------------------|------------------------------------------------------------------------------|-------------------------------------------|  
| **Member 1** | Parser Lead       | - File I/O & preprocessing<br>- Function detection<br>- Brace matching       | `analyze_file()`, Cleaned code output     |  
| **Member 2** | Analyzer Lead     | - Loop/recursion detection<br>- Memory ops tracking<br>- Metrics calculation | `is_loop_start()`, `CodeAnalysis` struct  |  
| **Member 3** | Reporter Lead     | - Big-O estimation<br>- Report formatting<br>- JSON output                   | `print_analysis_report()`, JSON exporter  |  

---

## **4. Build & Execution**  
### **4.1 Compilation**  
* parser.c analyzer.c reporter.c main.c => complexity_analyzer.c
```bash  
gcc complexity_analyzer.c -o analyzer   
```  

### **4.2 Usage**  
```bash  
./analyzer examples/quick_sort.c  
```  
**Sample Output:**  
```text  
=== Complexity Report ===  
Functions: 3 | Loops: 5 | Max Nested: 2  
Time Complexity: O(n log n)  
Space Complexity: O(n)  
```  

---

## **6. Example Test Cases**  
| **File**           | **Expected Complexity**      |  
|--------------------|------------------------------|  
| `linear_search.c`  | Time: O(n), Space: O(1)      |  
| `binary_tree.c`    | Time: O(log n), Space: O(n)  |  
| `bubble_sort.c`    | Time: O(n²), Space: O(1)     |  

---

## **7. Contact & Credits**  
| **Name**                | **Role**          |
|-------------------------|-------------------|
| Mansi Rajan Gupta       | Parser Lead       |   
| Yashi Sharma            | Analyzer Lead     |    
| Nishtha Gupta           | Reporter Lead     | 

