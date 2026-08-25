CXX ?= g++
CXXFLAGS ?= -O3 -march=native -std=c++17
EXE ?= blueberry

ifeq ($(OS),Windows_NT)
    GIT_SHA := $(shell git rev-parse --short HEAD 2>nul)
    GIT_DATE := $(shell git show -s --format=%%cd --date=format:%%d%%m%%Y HEAD 2>nul)
else
    GIT_SHA := $(shell git rev-parse --short HEAD 2>/dev/null)
    GIT_DATE := $(shell git show -s --format=%cd --date=format:%d%m%Y HEAD 2>/dev/null)
endif

ifeq ($(GIT_SHA),)
    GIT_SHA := N/A

    ifeq ($(OS),Windows_NT)
        GIT_DATE := $(shell powershell -NoProfile -Command "Get-Date -Format ddMMyyyy")
    else
        GIT_DATE := $(shell date +%d%m%Y)
    endif
endif

CXXFLAGS += -DGIT_SHA=\"$(GIT_SHA)\" -DGIT_DATE=\"$(GIT_DATE)\"

all:
	$(CXX) $(CXXFLAGS) *.cpp -o $(EXE) 