import subprocess

program = "./build/chopt"
song_dir = "integration_tests/songs"

songs = ["cups-and-cakes.mid", "satch-boogie-live.chart"]
paths = ["3", "2-2-2-2-1(+1)-2-4-1"]
scores = [(50801, 71009), (676357, 892633)]
activations = [
    [(36.625, 43.625)],
    [
        (37.75, 44.9161),
        (60.7854, 66.1099),
        (74.5, 78.5591),
        (86.9161, 92.6672),
        (101.842, 110.969),
        (116.125, 120.188),
        (160.459, 168.533),
        (174.917, 180.15),
    ],
]

outputs = []
for path, score_pair, acts in zip(paths, scores, activations):
    output_lines = ["Optimising, please wait..."]
    output_lines.append(f"Path: {path}")
    output_lines.append(f"No SP score: {score_pair[0]}")
    output_lines.append(f"Total score: {score_pair[1]}")
    for i, act in enumerate(acts):
        act_range = f"Measure {act[0]} to Measure {act[1]}"
        output_lines.append(f"Activation {i + 1}: {act_range}")
    output_lines.append("")
    outputs.append("\n".join(output_lines).encode("utf-8"))

for song, output in zip(songs, outputs):
    result = subprocess.run([program, "-f", f"{song_dir}/{song}"], capture_output=True)
    result.check_returncode()
    if result.stdout != output:
        raise RuntimeError(f"Song {song} has incorrect output")
