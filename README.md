# Rendering
This visual studio 2013 project implements:
1) the muti-pass SSAO rendering. The process and shader code mostly follow the instructions from http://www.learnopengl.com/#!Advanced-Lighting/SSAO.
2) Depth dependent halos. (http://ieeexplore.ieee.org/ieee_pilot/articles/06/ttg2009061299/article.html)
3) LineAO. (http://ieeexplore.ieee.org/document/6216373/)

Preview:
![SSAO](preview.png?raw=true "Program Interface")
![Depth Dependent Halos](ddh_ACR.JPG?raw=true "Depth Dependent Halos")
![LineAO-Line](lineAO_line.JPG?raw=true "LineAO (line)")
![LineAO-Tube](lineAO_tube.JPG?raw=true "LineAO (tube)")

Requires:
1. freeglut (http://freeglut.sourceforge.net/);
2. glew(http://glew.sourceforge.net/);
3. glui (http://glui.sourceforge.net/).
