# Java as a Scripting Language in C++
Demo loads a Java jar at Runtime and demonstrates executing Java code from C++ 
as well as C++ re-entry from Java.

# Requirements
* java 8+
* g++

# How to Build
``` bash
$ gradle -PJAVA_HOME=$JAVA_HOME assemble jar
```

# How to Run
``` bash
$ LD_LIBRARY_PATH=$JAVA_HOME/lib/server build/exe/nativeExe/nativeExe
```

# Notes:
* Application expects the java code to be stored at build/libs/jvm_scripting_cpp.jar