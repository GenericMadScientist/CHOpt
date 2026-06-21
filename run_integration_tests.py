import json
from pathlib import Path
import subprocess
import sys


def expected_path_output(path):
    lines = ["Optimising, please wait..."]
    lines.append(f"Path: {path['summary']}")
    lines.append(f"No SP score: {path['no_sp_score']}")
    lines.append(f"Total score: {path['total_score']}")
    if "average_multiplier" in path:
        lines.append(f"Average multiplier: {path['average_multiplier']:.3f}x")
    lines += path["activations"]
    lines.append("")
    return "\n".join(lines)


def chart_path(directory):
    chart_file_extensions = [".chart", ".mid", ".mid.qb.xen"]

    for path, _, files in directory.walk():
        for f in files:
            if any(f.endswith(ext) for ext in chart_file_extensions):
                return path.joinpath(f)

    raise RuntimeError(f"No song found in {directory}")


def actual_path_output(path, song_dir):
    args = [
        "build/chopt",
        "--file",
        chart_path(song_dir),
    ]
    optional_keys = [
        "difficulty",
        "early_whammy",
        "engine",
        "instrument",
        "speed",
        "squeeze",
    ]
    for k in optional_keys:
        if k in path:
            if k == "difficulty":
                args.append("--diff")
            else:
                args.append("--" + k.replace("_", "-"))
            args.append(str(path[k]))

    result = subprocess.run(args, capture_output=True)
    result.check_returncode()
    Path("path.png").unlink()
    return result.stdout.decode("utf-8")


def check_path(path, song_dir):
    expected = expected_path_output(path)

    try:
        actual = actual_path_output(path, song_dir)
    except subprocess.CalledProcessError as e:
        print(e.stderr.decode("utf-8"))
        raise

    if expected != actual:
        print(f"Disagreement on {song_dir}", file=sys.stderr)
        print("Expected:", file=sys.stderr)
        print(expected, file=sys.stderr)
        print("Actual:", file=sys.stderr)
        print(actual, file=sys.stderr)
        raise RuntimeError("Test failure")


def song_paths(song_dir):
    songs_json = song_dir.joinpath("paths.json")
    with songs_json.open() as f:
        return json.load(f)


for song_dir in Path("integration_tests").iterdir():
    for path in song_paths(song_dir):
        check_path(path, song_dir)
