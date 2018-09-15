import sys
from . import Pipeline

sys.dont_write_bytecode = True

if len(sys.argv) > 1:
    root_path = sys.argv[1]
    savePath = None
    if len(sys.argv) > 2:
        savePath = sys.argv[2]

    p = Pipeline(args = sys.argv[3:])
    p.load(root_path)
    p.save(savePath)

else:
    print("Required arguments: <generator script>")
