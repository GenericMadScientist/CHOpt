# The location of CHOpt.exe (specifically, the CLI version). This path must
# include the CHOpt.exe at the end (or whatever is appropriate, if the exe has
# been renamed).
$CHOPT_PATH = 'path\to\CHOpt\here'
# The location of the setlist you want to optimise
$SETLIST_PATH = 'path\to\setlist\here'
# The extra arguments (other than path and song location) you wish to pass to
# CHOpt, for example "--sqz 50 --ew 40" for 50% squeeze/40% whammy
$CHOPT_ARGS = ''
# The name of the image file you want (e.g., sqz-50.png)
$IMAGE_NAME = 'path.png'

$files = Get-ChildItem -LiteralPath $SETLIST_PATH -Recurse | Where-Object { ! $_.PSIsContainer }
$charts = $files | Where-Object { $_.Name -eq 'notes.chart' } | Select-Object Directory
$mids = $files | Where-Object { $_.Name -eq 'notes.mid' } | Select-Object Directory

# We do the notes.chart files before the notes.mid files because some songs
# will have both, and my understanding is Clone Hero will prioritise the
# notes.mid files in that situation.

foreach ($chart in $charts) {
    $chartPath = Join-Path -Path $chart.Directory -ChildPath 'notes.chart'
    $imgPath = Join-Path -Path $chart.Directory -ChildPath $IMAGE_NAME
    $fullArgs = "-f `"$chartPath`" -o `"$imgPath`" $CHOPT_ARGS"
    Start-Process -NoNewWindow -Wait -FilePath $CHOPT_PATH -ArgumentList $fullArgs
}

foreach ($mid in $mids) {
    $midPath = Join-Path -Path $mid.Directory -ChildPath 'notes.mid'
    $imgPath = Join-Path -Path $mid.Directory -ChildPath $IMAGE_NAME
    $fullArgs = "-f `"$midPath`" -o `"$imgPath`" $CHOPT_ARGS"
    Start-Process -NoNewWindow -Wait -FilePath $CHOPT_PATH -ArgumentList $fullArgs
}