import sys

tps = 1.0e12         # default to 1 THz (1 Tick == 1 ps)
tps_fixed = False    # once set to true, can't be changed

# fix the global frequency and tell C++ about it
def fixGlobalFrequency():
    import internal
    global tps, tps_fixed
    if not tps_fixed:
        tps_fixed = True
        internal.core.setClockFrequency(int(tps))
        print "Global frequency set at %d ticks per second" % int(tps)

def setGlobalFrequency(ticksPerSecond):
    import convert

    global tps, tps_fixed

    if tps_fixed:
        raise AttributeError, \
              "Global frequency already fixed at %f ticks/s." % tps

    if isinstance(ticksPerSecond, (int, long)):
        tps = ticksPerSecond
    elif isinstance(ticksPerSecond, float):
        tps = ticksPerSecond
    elif isinstance(ticksPerSecond, str):
        tps = round(convert.anyToFrequency(ticksPerSecond))
    else:
        raise TypeError, \
              "wrong type '%s' for ticksPerSecond" % type(ticksPerSecond)

# how big does a rounding error need to be before we warn about it?
frequency_tolerance = 0.001  # 0.1%

def fromSeconds(value):
    if not isinstance(value, float):
        raise TypeError, "can't convert '%s' to type tick" % type(value)

    # once someone needs to convert to seconds, the global frequency
    # had better be fixed
    if not tps_fixed:
        raise AttributeError, \
              "In order to do conversions, the global frequency must be fixed"

    if value == 0:
        return 0

    # convert the value from time to ticks
    value *= tps

    int_value = int(round(value))
    err = (value - int_value) / value
    if err > frequency_tolerance:
        print >>sys.stderr, "Warning: rounding error > tolerance"
        print >>sys.stderr, "    %f rounded to %d" % (value, int_value)
    return int_value

__all__ = [ 'setGlobalFrequency', 'fixGlobalFrequency', 'fromSeconds',
            'frequency_tolerance' ]
