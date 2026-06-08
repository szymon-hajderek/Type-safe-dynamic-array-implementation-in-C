#!/bin/bash

# --- Formatting Variables ---
GREEN='\033[1;32m'
RED='\033[1;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}====================================================${NC}"
echo -e "${YELLOW}   Starting Memory Leak & Sanitizer Diagnostics     ${NC}"
echo -e "${YELLOW}====================================================${NC}"

# --- Step 1: Valgrind Check ---
echo -e "\n${GREEN}[1/2] Building standard target for Valgrind...${NC}"
make test

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Compilation failed for 'make test'. Aborting.${NC}"
    exit 1
fi

echo -e "${GREEN}>>> Running Valgrind...${NC}"
# --error-exitcode=1 forces valgrind to return a failure status if leaks/errors are found
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --error-exitcode=1 \
         ./test

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Valgrind detected memory errors or leaks! Stop.${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Valgrind check passed flawlessly!${NC}"

# --- Step 2: Address Sanitizer Check ---
echo -e "\n${GREEN}[2/2] Building ASan target...${NC}"
# Clean the previous build just to be absolutely sure we recompile with ASan flags
rm -f test 
make test-asan

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Compilation failed for 'make test-asan'. Aborting.${NC}"
    exit 1
fi

echo -e "${GREEN}>>> Running ASan binary...${NC}"
# ASan will automatically crash the program and return a non-zero exit code on failure
./test

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Address Sanitizer detected undefined behavior or memory errors! Stop.${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Address Sanitizer check passed flawlessly!${NC}"

# --- Completion ---
echo -e "\n${YELLOW}====================================================${NC}"
echo -e "${GREEN}🎉 All tests passed! Your vector implementation is clean. ${NC}"
echo -e "${YELLOW}====================================================${NC}"