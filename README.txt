SMART STUDY GROUP FORMATION SYSTEM  -  DSA Mini Project
=========================================================
Frontend : HTML5 + CSS3 + vanilla JavaScript   (frontend/)
Backend  : C++17, own HTTP server, no libraries (backend/)
Storage  : JSON files (data/students.json, data/groups.json)

IMPORTANT - GETTING THE .EXE   (please read)
--------------------------------------------
This ZIP contains the COMPLETE C++ source code, but it does NOT contain
backend\studygroup_server.exe. The environment this project was built in
had no Windows compiler and no internet, so a Windows .exe could not be
produced. The backend WAS compiled and tested on Linux (see
docs\TEST_REPORT.txt); the Windows-specific socket code (Winsock) could
NOT be compiled or run here and is therefore untested.

Easiest way to get the .exe (about 2 minutes, nothing to install):
  1. Download the x64 release asset from:
       https://github.com/skeeto/w64devkit/releases
     The release download is a self-extracting .exe archive, not a regular
     .zip file. Run the downloaded file and choose the tools\w64devkit folder
     as the extraction destination. If it will not open, use 7-Zip to extract
     the .exe archive.
  2. Check that this file exists:
       tools\w64devkit\bin\g++.exe
  3. Double-click RUN.bat. It sees the server .exe is missing, builds it
     automatically with BUILD.bat, starts the server and opens the browser.
Alternatives: any g++ already on PATH, or Visual Studio's "Developer
Command Prompt" (cl) are detected automatically by BUILD.bat.
Once backend\studygroup_server.exe exists, RUN.bat just starts it.

HOW TO RUN
----------
  Double-click RUN.bat  ->  browser opens http://localhost:8080
  Double-click STOP.bat ->  stops the server
  (VS Code: Terminal > Run Task > "Run application")
If you see "cannot start the server on port 8080", run STOP.bat first.

FOLDER STRUCTURE
----------------
  RUN.bat / STOP.bat / BUILD.bat   one-click scripts
  backend/    main.cpp (server + REST API), Json.h,
              Student.h/.cpp        student record + JSON persistence
              Compatibility.h/.cpp  weighted score + group explanation
              Graph.h/.cpp          adjacency-list graph
              GreedyGrouping.*      greedy grouping (uses DSU)
              DSU.*                 disjoint set union
              BFSGrouping.*         BFS connected-component grouping
  frontend/   index.html, css/style.css, js/app.js
  data/       students.json (8 sample students), groups.json
  docs/       DSA_EXPLANATION.txt (viva notes), TEST_REPORT.txt

ALGORITHMS (all in C++)
-----------------------
  Compatibility = subjects 40 + timings 30 + goals 20 + skills 10 (/100)
  Graph         : adjacency list, undirected, weight = compatibility
                  (edge only if common subject AND common time AND score >= threshold)
  Greedy        : sort edges high->low, merge groups with DSU union
  BFS           : connected components, cut to preferred size
  DSU           : parent[], find() with path compression, union by size
See docs\DSA_EXPLANATION.txt and the "Algorithm Analysis" page.
