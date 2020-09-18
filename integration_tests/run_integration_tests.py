import subprocess

program = "./build/chopt"
song_dir = "integration_tests/songs"

songs = ["cups-and-cakes.mid"]
paths = ["3"]
scores = [(50801, 71009)]
activations = [[(36.625, 43.625)]]

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
    outputs.append("\n".join(output_lines))

for song, output in zip(songs, outputs):
    result = subprocess.run([program, "-f", f"{song_dir}/{song}"], capture_output=True)
    result.check_returncode()
    if result.stdout.decode("utf-8") != output:
        raise RuntimeError(f"Song {song} has incorrect output")
