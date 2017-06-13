
p = pipeline

p.add({
    "name": "a",
    "command": "echo AAA"
})

p.add({
    "name": "b",
    "command": "echo BBB",
    "dependencies": "a"
})

p.add({
    "name": "c",
    "command": "echo CCC > c.out",
    "output_files": "c.out"
})

p.add({
    "name": "d",
    "command": "echo DDD > d.out",
    "input_files": "c.out",
    "output_files": "d.out",
})

if False:
    p.add({
        "name": "self-dependent",
        "command": "echo nope",
        "dependencies": "self-dependent"
    })

if False:
    p.add({
        "name": "file-loop",
        "command": "echo nope",
        "input_files": "d.out",
        "output_files": "c.out"
    })

if False:
    p.add({
        "name": "missing-input",
        "command": "echo nope",
        "input_files": "missing.in"
    })

pipeline.load("subdir/test2.py")
