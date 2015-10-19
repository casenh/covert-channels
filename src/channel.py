import run
import importlib

for package in run.__all__ :
    print("running {}".format(package))
    importlib.import_module("run." + package).channel.run()
