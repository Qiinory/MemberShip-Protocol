cmake_minimum_required(VERSION 3.14)
project(mp1_SWIM)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(testcases)

add_executable(mp1_SWIM
        testcases/msgdropsinglefailure.conf
        testcases/multifailure.conf
        testcases/singlefailure.conf
        Application.cpp
        Application.h
        EmulNet.cpp
        EmulNet.h
        Grader.sh
        Log.cpp
        Log.h
        Makefile
        Member.cpp
        Member.h
        mp1_specifications.pdf
        MP1Node.cpp
        MP1Node.h
        Params.cpp
        Params.h
        Queue.h
        run.sh
        stdincludes.h
        submit.py)
