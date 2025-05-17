#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' 

echo -e "${BLUE}Starting CompO-meter...${NC}"

# Build AST Generator
echo -e "${GREEN}Building AST Generator...${NC}"
cd ast_generator/cpp
mkdir -p build
cd build
cmake ..
make
cd ../../..

# Compile C Analyzer
echo -e "${GREEN}Compiling C Analyzer...${NC}"
cd analyzer
make clean
make
cd ..

# Start Backend
echo -e "${GREEN}Starting Flask Backend...${NC}"
cd backend
python app.py &
BACKEND_PID=$!
cd ..

# Start Frontend
echo -e "${GREEN}Starting React Frontend...${NC}"
cd frontend
npm start &
FRONTEND_PID=$!
cd ..

# Function to handle script termination
cleanup() {
    echo -e "${BLUE}Shutting down CompO-meter...${NC}"
    kill $BACKEND_PID
    kill $FRONTEND_PID
    exit
}

# Set up trap for cleanup on script termination
trap cleanup SIGINT SIGTERM

# Keep script running
echo -e "${BLUE}CompO-meter is running!${NC}"
echo -e "Frontend: http://localhost:3000"
echo -e "Backend: http://localhost:5001"
echo -e "Press Ctrl+C to stop all services"
wait 