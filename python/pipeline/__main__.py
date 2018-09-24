import sys
from . import Pipeline
from pathlib import Path

sys.dont_write_bytecode = True

if len(sys.argv) > 1:
    root_file = Path(sys.argv[1])
    save_file = None
    if len(sys.argv) > 2:
        save_file = sys.argv[2]

    p = Pipeline(root_file.parent, args = sys.argv[3:])
    p.load(root_file.name)
    p.save(save_file)

else:
    print("Required arguments: <generator script>")
