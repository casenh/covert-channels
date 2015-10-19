import argparse

def mkparser (channelName) :
    parser = argparse.ArgumentParser(description = "run the " + channelName + " covert channel")
    parser.add_argument('--buddy','-b', type=str, help='the buddythread noise type to create [optional]')
    parser.add_argument('--learn','-l', action='store_true', default=False)
    return parser
