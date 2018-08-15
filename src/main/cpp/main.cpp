#include <cstddef>

#include <dlfcn.h>
#include <jni.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

typedef jint(*CreateJvmFuncPtr) (JavaVM**, void**, JavaVMInitArgs*);

namespace {
    // JNI function; this is linked later in the application.
    JNIEXPORT void JNICALL __sayHelloCPP(JNIEnv * pEnv, jclass, jstring name) {
        auto chars = pEnv->GetStringUTFChars(name, nullptr);

        std::cout << "Hello from C++, "
            << chars
            << std::endl;

        pEnv->ReleaseStringUTFChars(name, chars);
    }
}

int main(int argc, char** argv) {
    auto jvmOpts = std::vector<JavaVMOption> ();

    {
        JavaVMOption op {};

        op.optionString = const_cast<char *>("-Xmx256M");
        jvmOpts.push_back(op);

        // change this line to point to wherever the application Jar is 
        op.optionString = const_cast<char *> ("-Djava.class.path=build/libs/jvm_scripting_cpp.jar");
        jvmOpts.push_back(op);
    }

    auto jvmArgs = JavaVMInitArgs {};

    // change to JNI_VERSION_10 for modules
    jvmArgs.version = JNI_VERSION_1_8;
    jvmArgs.nOptions = static_cast<jint> (jvmOpts.size());
    jvmArgs.options = jvmOpts.data();
    jvmArgs.ignoreUnrecognized = JNI_TRUE;

    std::cout << "Option Count: " << std::dec << jvmOpts.size() << "\n";

    for (auto& op: jvmOpts) {
        std::cout << "Op: " << op.optionString << std::endl;
    }

    // this function is specific to Linux
    auto pJVMLib = dlopen("libjvm.so", RTLD_NOW);

    {
        auto error = dlerror();

        if (error) {
            throw std::runtime_error(error);
        }
    }

    JavaVM* pJavaVM = nullptr;
    JNIEnv* pJNIEnv = nullptr;

    // Create the JavaVM and JNI Environment
    //TODO: check if JNI must be executed on the thread the JVM was created.
    auto pfnJNI_CreateJVM = reinterpret_cast<CreateJvmFuncPtr> (dlsym(pJVMLib, "JNI_CreateJavaVM"));
    auto flag = pfnJNI_CreateJVM(&pJavaVM, reinterpret_cast<void**> (&pJNIEnv), &jvmArgs);

    if (JNI_ERR == flag) {
        std::stringstream msg;

        msg << "Error 0x" << std::hex << flag << " while creating JavaVM!";

        throw std::runtime_error(msg.str());
    }

    const char * CLASS_NAME = "test/Script";
    auto jclass = pJNIEnv->FindClass(CLASS_NAME);

    if (jclass) {
        // link the JNI call
        {
            JNINativeMethod pfnSayHelloCPP {};
    
            pfnSayHelloCPP.name = const_cast<char * > ("sayHelloCPP");
            pfnSayHelloCPP.signature = const_cast<char * > ("(Ljava/lang/String;)V");
            pfnSayHelloCPP.fnPtr = reinterpret_cast<void *> (&__sayHelloCPP);

            pJNIEnv->RegisterNatives(jclass, &pfnSayHelloCPP, 1);
        }
        
        auto jfnSayHello = pJNIEnv->GetStaticMethodID(jclass, "sayHello", "(Ljava/lang/String;)V");

        if (jfnSayHello) {
            auto str = pJNIEnv->NewStringUTF("World");

            pJNIEnv->CallStaticVoidMethod(jclass, jfnSayHello, str);
        } else {
            std::stringstream msg;

            msg << "Could not find Java Static Method: \"" 
                << CLASS_NAME 
                << "."
                << "sayHello\"";

            throw std::runtime_error(msg.str());
        }
    } else {
        std::stringstream msg;

        msg << "Could not find Java Class: \"" 
            << CLASS_NAME
            << "\"";

        throw std::runtime_error(msg.str());
    }

    pJavaVM->DestroyJavaVM();
    dlclose(pJVMLib);
    
    return 0;
}