/*
 * Copyright (C) 2017-2018 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "environment.h"
#include "runtime/os_interface/os_library.h"
#include "unit_tests/custom_event_listener.h"
#include "unit_tests/helpers/test_files.h"

#include "limits.h"

#ifdef WIN32
const char *fSeparator = "\\";
#elif defined(__linux__)
const char *fSeparator = "/";
#endif
std::string cwd;

Environment *gEnvironment;

extern bool printMemoryOpCallStack;

std::string getRunPath() {
    std::string res;
#if defined(__linux__)
    res = getcwd(nullptr, 0);
#else
    res = cwd;
#endif
    return res;
}

int main(int argc, char **argv) {
    int retVal = 0;
    std::string execPath(argv[0]);
    char separator = '/';
    if (execPath.find_last_of("\\") != std::string::npos)
        separator = '\\';
    cwd = execPath.substr(0, execPath.find_last_of(separator));

    bool useDefaultListener = false;
    std::string devicePrefix("skl");
    std::string familyNameWithType("Gen9core");

#if defined(__linux__)
    if (getenv("CLOC_SELFTEST") == nullptr) {
        setenv("CLOC_SELFTEST", "YES", 1);

        char *ldLibraryPath = getenv("LD_LIBRARY_PATH");

        if (ldLibraryPath == nullptr) {
            setenv("LD_LIBRARY_PATH", getRunPath().c_str(), 1);
        } else {
            std::string ldLibraryPathConcat = getRunPath() + ":" + std::string(ldLibraryPath);
            setenv("LD_LIBRARY_PATH", ldLibraryPathConcat.c_str(), 1);
        }

        execv(argv[0], argv);
        //execv failed, we return with error
        printf("FATAL ERROR: cannot self-exec test!\n");
        return -1;
    }
#endif

    ::testing::InitGoogleTest(&argc, argv);
    if (argc > 0) {
        // parse remaining args assuming they're mine
        for (int i = 0; i < argc; i++) {
            if (strcmp("--use_default_listener", argv[i]) == 0) {
                useDefaultListener = true;
            } else if (strcmp("--device", argv[i]) == 0) {
                ++i;
                devicePrefix = argv[i];
            } else if (strcmp("--family_type", argv[i]) == 0) {
                ++i;
                familyNameWithType = argv[i];
            }
        }
    }

    // we look for test files always relative to binary location
    // this simplifies multi-process execution and using different
    // working directories
    std::string nTestFiles = getRunPath();
    nTestFiles.append("/");
    nTestFiles.append(familyNameWithType);
    nTestFiles.append("/");
    nTestFiles.append(testFiles);
    testFiles = nTestFiles;
    binaryNameSuffix.append(familyNameWithType);

#ifdef WIN32
#include <direct.h>
    if (_chdir(familyNameWithType.c_str())) {
        std::cout << "chdir into " << familyNameWithType << " directory failed.\nThis might cause test failures." << std::endl;
    }
#elif defined(__linux__)
#include <unistd.h>
    if (chdir(familyNameWithType.c_str()) != 0) {
        std::cout << "chdir into " << familyNameWithType << " directory failed.\nThis might cause test failures." << std::endl;
    }
#endif

    if (useDefaultListener == false) {
        ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
        ::testing::TestEventListener *defaultListener = listeners.default_result_printer();

        auto customEventListener = new CCustomEventListener(defaultListener);

        listeners.Release(listeners.default_result_printer());
        listeners.Append(customEventListener);
    }

    gEnvironment = reinterpret_cast<Environment *>(::testing::AddGlobalTestEnvironment(new Environment(devicePrefix, familyNameWithType)));

    retVal = RUN_ALL_TESTS();

    return retVal;
}
