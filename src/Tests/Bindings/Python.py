
if __name__ == "__main__":

    # Start actual tests
    import PyProfile
    print(f"PyProfile.__nbTimings__: {PyProfile.__nbTimings__}")
    print(f"PyProfile.__nbTracks__: {PyProfile.__nbTracks__}")

    profiler = PyProfile.Profiler()
    print(f"Profiler: {profiler}")
    # Initializing profiler
    print("From here, the profiler's clock starts ticking")
    profiler.Initialize()

    # Test Set/Get Profiler
    print ("Setting the profiler")
    PyProfile.SetProfiler(profiler)
    print(f"Set Profiler == Get Profiler: {PyProfile.GetProfiler() == profiler}")

    # Test Profiler.name
    ## Default name
    print(f"By default, Profiler.name: {profiler.name}")
    ## Set name
    profiler.name = "NEW NAME"
    print(f"New Profiler.name: {profiler.name}")
    
    ##Set Track Name
    profiler.SetTrackName(0, "PythonTestTrack")

    # Test the core profiling functionality
    ## Declaration of an array that will store integegers to simulate an activity
    size = 1024*1024
    array = [0]*size # given that the size of ints is 4 bytes, this array will take 4MB of memory

    # Test the core profiling functionality
    ## Try profile a block of code including the time and bandwidth
    for i in range(0, size):
        PyProfile.ProfileBlockTimeBandwidth("TestBlock_TIME_BANDWIDTH", 0, 4)
        array[i] = i

    ## Try profile a block of code including only the time
    for i in range(0, size):
        PyProfile.ProfileBlockTime("TestBlock_TIME_ONLY", 0)
        array[i] = i

    ## try profiling a function with time and bandwidth
    ### Definition of the test function
    def TestFunction_TIME_BANDWIDTH(nb_iterations=100):
        PyProfile.ProfileFunctionTimeBandwidth(0, 4*nb_iterations)
        for i in range(0, nb_iterations):
            array[i] = i
    
    ### Call the test function
    TestFunction_TIME_BANDWIDTH(size)

    ## try profiling a function with time only
    ### Definition of the test function
    def TestFunction_TIME_ONLY(nb_iterations=100):
        PyProfile.ProfileFunctionTime(0)
        for i in range(0, nb_iterations):
            array[i] = i
    
    ### Call the test function
    TestFunction_TIME_ONLY(size)
    
    # Ending profiler
    print("From here, the profiler's clock stops ticking")
    profiler.End()

    ## Print profiler report
    profiler.Report()