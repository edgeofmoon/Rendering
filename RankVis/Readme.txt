RankVis
10/23/2017
Guohao Zhang
bd71571@umbc.edu
https://github.com/edgeofmoon/Rendering

Description:
This solution contains multiple project. RankVis is used for the final experiments.
RankVis was used for two experiments:
1) The dMRI visualization study
This study compared four encoding methods combined with two shapes for an aggregated comparison task; 
and four rendering methods combined with two shapes for tract tracing task and tumor contact task.
2) The color study
This study compared six color maps for an aggregated estimation task and four coloring methods for tract tracing task.

Commandline parameters:
The program has multiple mode which can be specified in the command line parameters.
Go to MyVisRankingApp.h to see the value of each mode.
APP_MODE_TRAINING is used for building the training program (provide correct answer after participant answered)
APP_MODE_STUDY is the formal study program.
APP_MODE_LIGHTING is used for testing the lighting profile of each rendering methods.
APP_MODE_OCCLUSION is used for testing the occlusion profile of different shapes (glyph vs tube)
APP_MODE_PRINTDATA prints the vis config information for the current subject to a file before proceeding to other mode.

Data:
The path to all data folders are specified in MyVisRankingApp.cpp.

Build:
For dMRI visualization study, include MyVisRankingApp_UI_old.cpp.
For color study, include MyVisRankingApp_UI.cpp instead.
Exclude MyVisRankingApp_UI_color1.cpp for both cases.
All *.cpp files with a main function are for testing purposes and only one should be included for building an executable.
The appTest.cpp should be used for building the experiment program.
The tractView.cpp should be used for building the viewer where the five major bundles are colored and labelled.