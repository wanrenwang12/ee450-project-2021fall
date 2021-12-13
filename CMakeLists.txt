cmake_minimum_required(VERSION 3.12)
project(ee450_project)

set(CMAKE_CXX_STANDARD 14)

add_executable(ee450_project
        central
        central.cpp
        clientA
        clientA.cpp
        clientB
        clientB.cpp
        serverT.cpp
        serverT
        serverS.cpp
        serverS
        serverP.cpp
        serverP
        makefile
        edgelist.txt
        scores.txt
        readme.txt
        )
