import sqlite3
import subprocess

program = "./build/chopt"
song_dir = "integration_tests/songs"

conn = sqlite3.connect("integration_tests/tests.db")
c = conn.cursor()

c.execute("select ID, File, Difficulty, Path, BaseScore, TotalScore from Songs")
songs = list(c.fetchall())

outputs = []
for song in songs:
    song_id, _, _, path, base_score, total_score = song
    output_lines = ["Optimising, please wait..."]
    output_lines.append(f"Path: {path}")
    output_lines.append(f"No SP score: {base_score}")
    output_lines.append(f"Total score: {total_score}")
    c.execute(
        "select ActivationNumber, Start, End from Activations where SongID = ? order by ActivationNumber",
        (song_id,),
    )
    for act_num, start, end in c.fetchall():
        act_range = f"Measure {start} to Measure {end}"
        output_lines.append(f"Activation {act_num}: {act_range}")
    output_lines.append("")
    outputs.append("\n".join(output_lines).encode("utf-8"))

conn.close()

for song, output in zip(songs, outputs):
    _, file, difficulty, _, _, _ = song
    result = subprocess.run(
        [program, "-f", f"{song_dir}/{file}", "-d", difficulty], capture_output=True
    )
    result.check_returncode()
    if result.stdout != output:
        raise RuntimeError(f"Song {file} has incorrect output")
