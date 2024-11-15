
if __name__ == "__main__":

    # Start actual tests
    import PyProfile
    print(f"PyProfile.__nbTimings__: {PyProfile.__nbTimings__}")
    print(f"PyProfile.__nbTracks__: {PyProfile.__nbTracks__}")

    profiler = PyProfile.Profiler()
    print(f"Profiler: {profiler}")

    # Test Profiler.name
    ## Default name
    print(f"Profiler.name: {profiler.name}")
    ## Set name
    profiler.name = "Test"
    print(f"New Profiler.name: {profiler.name}")
    