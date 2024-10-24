
if __name__ == "__main__":
    import os
    import sys

    pyProfileCompiler = "MSVC" # or "Clang", "GCC"
    pyProfileBuildPlatform = "x64" # or "x86"
    pyProfileBuildConfiguration = "Release" # or "Debug"
    pyProfileBuildPath = "../../../out/build/" + pyProfileCompiler + ("-" + pyProfileBuildPlatform if pyProfileCompiler == "MSVC" else "") + "/src/Bindings/"+ pyProfileBuildConfiguration
    pyProfilePythonVersion = "311" # or "310", "39", "38", etc...

    print("pyProfileBuildPath: " + pyProfileBuildPath)
    
    # Check if the path exists
    if os.path.exists(pyProfileBuildPath)==False:
        print("ERROR: The directory " + pyProfileBuildPath + " that should contain the PyProfile library does not exist. Are you sure you built the project?")
        sys.exit(1)
    
    # Checks is the PyProfile module files are present in the directory
    expectedBuildFileNames= ["PyProfile.cp"+pyProfilePythonVersion+"-win_amd64.pyd", "PyProfile.lib", "PyProfile.exp"]
    for expectedBuildFileName in expectedBuildFileNames:
        if os.path.exists(pyProfileBuildPath + "/" + expectedBuildFileName)==False:
            print("ERROR: The file " + expectedBuildFileName + " is missing in the directory " + pyProfileBuildPath + ". Try to rebuild the project.")
            sys.exit(1)

    # if the PyProfile modules files are also present in the current directory, remove them
    for expectedBuildFileName in expectedBuildFileNames:
        if os.path.exists("./" + expectedBuildFileName):
            os.remove("./" + expectedBuildFileName)
    
    # Copy the PyProfile module files to the current directory
    import shutil
    shutil.copy(pyProfileBuildPath + "/PyProfile.cp" + pyProfilePythonVersion + "-win_amd64.pyd", "./PyProfile.cp" + pyProfilePythonVersion + "-win_amd64.pyd")
    shutil.copy(pyProfileBuildPath + "/PyProfile.lib", "./PyProfile.lib")
    shutil.copy(pyProfileBuildPath + "/PyProfile.exp", "./PyProfile.exp")

    # Start actual tests
    import PyProfile
    print(PyProfile.add(1, 2))