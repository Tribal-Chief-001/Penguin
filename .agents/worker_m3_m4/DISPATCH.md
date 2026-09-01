## 2026-08-31T17:40:19Z
You are the Implementation Worker for Milestone 3 (Linux Desktop Integration, MPRIS2 & CLI) and Milestone 4 (Media Library & State Persistence) for Penguin.
Working directory: /home/lucifer/Documents/Projects/Penguin/.agents/worker_m3_m4
Project Root: /home/lucifer/Documents/Projects/Penguin

Input files to read:
- /home/lucifer/Documents/Projects/Penguin/.agents/ORIGINAL_REQUEST.md
- /home/lucifer/Documents/Projects/Penguin/PROJECT.md
- /home/lucifer/Documents/Projects/Penguin/TEST_INFRA.md
- /home/lucifer/Documents/Projects/Penguin/.agents/spec_miner_survey/spec_report.md
- /home/lucifer/Documents/Projects/Penguin/.agents/explorer_survey_2/integration_test_analysis.md

MANDATORY INTEGRITY WARNING:
DO NOT CHEAT. All implementations must be genuine. DO NOT hardcode test results, create dummy/facade implementations, or circumvent the intended task. A auditor will independently verify your work. Integrity violations WILL be detected and your work WILL be rejected.

Objective:
Implement Milestone 3 (\`src/desktop/\`, \`penguin.desktop\`, \`icons/\`, \`src/main.cpp\`) and Milestone 4 (\`src/library/\`) and integrate with the full application:
1. Milestone 3: Linux Desktop Integration, MPRIS2 & CLI
2. Milestone 4: Media Library & State Persistence
3. Integration:
   - Connect DatabaseManager and StatePersistence with MainWindow.
   - Connect MPRIS2Adaptor with PlaybackEngine.
   - Update penguin.pro and CMakeLists.txt.
   - Implement unit tests tests/test_m3_m4.cpp.
   - Compile full binary, run all unit tests, verify scripts/run_tests.sh passes 100%.
