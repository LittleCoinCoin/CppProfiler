
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
    
    profiler.SetTrackName(0, "PythonTestTrack")
    ## Try profile a block of code
    for i in range(0, 10):
        PyProfile.ProfileBlockTimeBandwidth__("TestBlock", 0, 26, 0)
    
    # Ending profiler
    print("From here, the profiler's clock stops ticking")
    profiler.End()

    ## Print profiler report
    profiler.Report()