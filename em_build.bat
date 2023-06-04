em++ -std=c++11 -o main.js src/main.cpp src/cgidata.cpp src/animationCurve.cpp src/animationCurveNode.cpp src/camera.cpp src/fbxdocument.cpp src/fbxexporter.cpp src/fbximporter.cpp src/fbxnode.cpp src/fbxobject.cpp src/fbxproperty.cpp src/fbxtime.cpp src/fbxutil.cpp src/miniz.cpp src/model.cpp src/scene.cpp -s ALLOW_MEMORY_GROWTH=1 --shell-file html_template/shell_minimal.html -s NO_EXIT_RUNTIME=1 -s "EXPORTED_RUNTIME_METHODS=['ccall']" -s EXPORTED_FUNCTIONS="['_main', '_malloc', '_free']" --embed-file assets/tdcamera.fbx