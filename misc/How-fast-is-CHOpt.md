# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.0.2.

## The songs

I ran CHOpt on every song from the following setlists/packs.

* Angevil Hero II
* Anti Hero
* Anti Hero: Beach Episode
* Anti Hero 2
* Blanket Statement
* Carpal Tunnel Hero: Remastered
* Carpal Tunnel Hero 2
* CHARTS
* CHARTS 2
* Circuit Breaker
* Cow Hero
* CSC Monthly Packs (June 2018 - September 2020)
* Digitizer
* Dissonance Hero
* Djent Hero Collection (Pack #1 - Pack #4)
* Focal Point
* GaMetal Power Pack
* Guitar Hero: Guitar Zero
* Guitar Hero X
* Koreaboo Hero
* Koreaboo Hero 2
* Marathon Hero
* Paradigm
* Redemption Arc
* The Fall of Troy Hero
* Vortex Hero
* Zero Gravity

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 43m10s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                      | Setlist                        | Time (s) |
| ----------------------------------------- | ------------------------------ | -------- |
| CHARTS 2: Endless Setlist                 | CHARTS 2                       | 59.261   |
| Uranoid                                   | CHARTS                         | 37.878   |
| CHARTS: The (almost) Endless Setlist      | CHARTS                         | 33.135   |
| Catch Thirtythree                         | Djent Hero Collection          | 19.485   |
| Endless Setlist: The Fall of Troy Hero    | The Fall of Troy Hero          | 18.886   |
| Chezy's Ultimate Solo Experience          | CHARTS 2                       | 13.194   |
| The Human Equation                        | Marathon Hero                  | 11.752   |
| Solace                                    | Djent Hero Collection          | 10.811   |
| Eskapist                                  | Marathon Hero                  | 10.523   |
| Eskapist                                  | CHARTS                         | 10.488   |
| The Future in Whose Eyes? (Full Album)    | CHARTS 2                       | 8.414    |
| Coma Witch (Full Album Chart)             | CHARTS                         | 8.109    |
| Periphery IV: Hail Stan (Full Album)      | CHARTS 2                       | 7.260    |
| Automata                                  | Marathon Hero                  | 6.676    |
| Time Traveler                             | Circuit Breaker                | 6.365    |
| The Source/Language I & II                | Marathon Hero                  | 6.302    |
| 8                                         | Marathon Hero                  | 6.172    |
| Warp                                      | CSC Monthly Packs              | 6.135    |
| Nothing More [FULL ALBUM]                 | CSC Monthly Packs              | 6.112    |
| Volition (Full Album Chart)               | CHARTS                         | 5.881    |
| That's The Spirit (Full Album Chart)      | CHARTS                         | 5.706    |
| The Baying of the Hounds                  | Carpal Tunnel Hero: Remastered | 5.562    |
| Live at Dynamo Open Air 1998 [FULL ALBUM] | CSC Monthly Packs              | 5.159    |
| Repine                                    | CHARTS                         | 5.056    |
| Bloodwork (Full Album Chart)              | CHARTS                         | 4.908    |

### By Setlist

| Setlist                        | Number of songs | Total time (s) | Average time (s) |
| ------------------------------ | --------------- | -------------- | ---------------- |
| Angevil Hero II                | 67              | 26.523         | 0.396            |
| Anti Hero                      | 402             | 172.714        | 0.430            |
| Anti Hero: Beach Episode       | 127             | 67.292         | 0.530            |
| Anti Hero 2                    | 365             | 173.629        | 0.476            |
| Blanket Statement              | 115             | 46.540         | 0.405            |
| Carpal Tunnel Hero: Remastered | 103             | 67.503         | 0.655            |
| Carpal Tunnel Hero 2           | 309             | 180.571        | 0.584            |
| CHARTS                         | 646             | 386.259        | 0.598            |
| CHARTS 2                       | 139             | 148.203        | 1.066            |         
| Circuit Breaker                | 116             | 51.676         | 0.445            |
| Cow Hero                       | 71              | 29.101         | 0.410            |
| CSC Monthly Packs              | 762             | 333.305        | 0.437            |
| Digitizer                      | 82              | 37.331         | 0.455            |
| Dissonance Hero                | 106             | 58.285         | 0.550            |
| Djent Hero Collection          | 131             | 101.862        | 0.778            |
| Focal Point                    | 170             | 86.186         | 0.507            |
| GaMetal Power Pack             | 54              | 29.617         | 0.548            |
| Guitar Hero: Guitar Zero       | 64              | 23.789         | 0.372            |
| Guitar Hero X                  | 127             | 73.523         | 0.579            |
| Koreaboo Hero                  | 51              | 16.017         | 0.314            |
| Koreaboo Hero 2                | 101             | 31.323         | 0.310            |
| Marathon Hero                  | 49              | 118.904        | 2.427            |
| Paradigm                       | 101             | 50.771         | 0.503            |
| Redemption Arc                 | 100             | 41.699         | 0.417            |
| The Fall of Troy Hero          | 70              | 60.014         | 0.857            |
| Vortex Hero                    | 222             | 97.129         | 0.438            |
| Zero Gravity                   | 178             | 80.580         | 0.453            |
