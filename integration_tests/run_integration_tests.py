from contextlib import closing
import sqlite3
import subprocess
import sys
import tempfile


def expected_path_output(path):
    lines = ["Optimising, please wait..."]
    path_summary = "-".join(a["phrase_count"] for a in path["activations"])
    lines.append(f"Path: {path_summary}")
    lines.append(f"No SP score: {path['no_sp_score']}")
    lines.append(f"Total score: {path['total_score']}")
    if path["average_multiplier"] is not None:
        lines.append(f"Average multiplier: {path['average_multiplier']:.3f}x")
    for act in path["activations"]:
        if act["description"] is None:
            continue
        line = f"{act['phrase_count']}: {act['description']}"
        if act["activation_end"] is not None:
            line += f" ({act['activation_end']})"
        lines.append(line)
    lines.append("")
    return "\n".join(lines)


def actual_path_output(path):
    with tempfile.TemporaryDirectory() as tempdir:
        with open(f"{tempdir}/song.ini", "w") as f:
            f.write(path["ini"])

        if path["chart_type"] == "chart":
            filename = "notes.chart"
        elif path["chart_type"] == "mid":
            filename = "notes.mid"
        else:
            filename = path["chart_type"]

        with open(f"{tempdir}/{filename}", "wb") as f:
            f.write(path["chart"])

        result = subprocess.run(
            [
                "build/chopt",
                "--file",
                f"{tempdir}/{filename}",
                "--diff",
                path["difficulty"],
                "--instrument",
                path["instrument"],
                "--engine",
                path["engine"],
                "--speed",
                str(path["speed"]),
                "--squeeze",
                str(path["squeeze"]),
                "--early-whammy",
                str(path["early_whammy"]),
                "--output",
                f"{tempdir}/path.png",
            ],
            capture_output=True,
        )
        result.check_returncode()
        return result.stdout.decode("utf-8")


def paths():
    with closing(sqlite3.connect("integration_tests/tests.db")) as conn:
        conn.row_factory = sqlite3.Row
        c = conn.cursor()
        c.execute("SELECT * FROM paths INNER JOIN songs USING (song_id)")
        path_map = {r["path_id"]: dict(r) | {"activations": []} for r in c.fetchall()}

        c.execute("SELECT * FROM activations ORDER BY activation_number")
        for act in c.fetchall():
            path_map[act["path_id"]]["activations"].append(dict(act))

        return path_map.values()


for path in paths():
    expected = expected_path_output(path)
    try:
        actual = actual_path_output(path)
    except subprocess.CalledProcessError as e:
        print(e.stderr.decode("utf-8"))
        raise
    if expected != actual:
        print(f"Disagreement on {path['name']}", file=sys.stderr)
        print("Expected:", file=sys.stderr)
        print(expected, file=sys.stderr)
        print("Actual:", file=sys.stderr)
        print(actual, file=sys.stderr)
        raise RuntimeError("Test failure")
