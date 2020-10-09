# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.1.0.

## The songs

I ran CHOpt on every song from the following setlists/packs that had Expert
Lead Guitar charted.

* Angevil Hero II
* Anti Hero
* Anti Hero: Beach Episode
* Anti Hero 2
* Blanket Statement
* Brand New Hero
* Carpal Tunnel Hero: Remastered
* Carpal Tunnel Hero 2
* CHARTS
* CHARTS 2
* Circuit Breaker
* Cow Hero
* CSC Monthly Packs (June 2018 - September 2020)
* DF Discography CH
* Digitizer
* Dissonance Hero
* Djent Hero Collection (Pack #1 - Pack #5)
* Facelift (Pack 1)
* Focal Point
* Focal Point 2
* GaMetal Power Pack
* Guitar Hero: Guitar Zero
* Guitar Hero X
* Koreaboo Hero
* Koreaboo Hero 2
* Marathon Hero
* Paradigm
* Phase Shift Guitar Project 4
* Redemption Arc
* The Fall of Troy Hero
* Vortex Hero
* Zero Gravity

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 44m31s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                             | Setlist               | Time (s) |
| ------------------------------------------------ | --------------------- | -------- |
| CHARTS 2: Endless Setlist                        | CHARTS 2              | 54.860   |
| CHARTS: The (almost) Endless Setlist             | CHARTS                | 31.799   |
| Endless Setlist: The Fall of Troy Hero           | The Fall of Troy Hero | 17.928   |
| Chezy's Ultimate Solo Experience                 | CHARTS 2              | 11.945   |
| Uranoid                                          | CHARTS                | 11.064   |
| The Human Equation                               | Marathon Hero         | 8.517    |
| Catch Thirtythree                                | Djent Hero Collection | 7.889    |
| Eskapist                                         | Marathon Hero         | 6.556    |
| Periphery IV: Hail Stan (Full Album)             | CHARTS 2              | 6.404    |
| Eskapist                                         | CHARTS                | 6.352    |
| Automata                                         | Marathon Hero         | 6.245    |
| Coma Witch (Full Album Chart)                    | CHARTS                | 5.635    |
| Nothing More [FULL ALBUM]                        | CSC Monthly Packs     | 5.605    |
| Live at Dynamo Open Air 1998 [FULL ALBUM]        | CSC Monthly Packs     | 5.591    |
| Volition (Full Album Chart)                      | CHARTS                | 5.546    |
| The Future in Whose Eyes? (Full Album)           | CHARTS 2              | 4.642    |
| Jane Doe (Full Album)                            | Dissonance Hero       | 4.305    |
| Bloodwork (Full Album Chart)                     | CHARTS                | 4.236    |
| Low Teens (Full Album Chart)                     | CHARTS                | 4.213    |
| In the Unlikely Event                            | The Fall of Troy Hero | 4.165    |
| That's The Spirit (Full Album Chart)             | CHARTS                | 4.115    |
| Time Will Die And Love Will Bury It (Full Album) | Dissonance Hero       | 3.971    |
| Culture Scars (Full Album Chart)                 | CHARTS                | 3.798    |
| Doppelgänger                                     | The Fall of Troy Hero | 3.700    |
| Fortress (Full Album Chart)                      | CHARTS                | 3.617    |

### By Setlist

| Setlist                        | Number of songs | Total time (s) | Average time (s) |
| ------------------------------ | --------------- | -------------- | ---------------- |
| Angevil Hero II                | 67              | 28.325         | 0.423            |
| Anti Hero                      | 402             | 169.893        | 0.423            |
| Anti Hero: Beach Episode       | 127             | 61.791         | 0.487            |
| Anti Hero 2                    | 365             | 161.668        | 0.443            |
| Blanket Statement              | 115             | 43.293         | 0.376            |
| Brand New Hero                 | 83              | 28.968         | 0.349            |
| Carpal Tunnel Hero: Remastered | 103             | 62.981         | 0.611            |
| Carpal Tunnel Hero 2           | 309             | 176.063        | 0.570            |
| CHARTS                         | 646             | 327.485        | 0.507            |
| CHARTS 2                       | 139             | 134.043        | 0.964            |         
| Circuit Breaker                | 116             | 50.338         | 0.434            |
| Cow Hero                       | 71              | 28.858         | 0.406            |
| CSC Monthly Packs              | 762             | 327.192        | 0.429            |
| DF Discography CH              | 80              | 56.439         | 0.705            |
| Digitizer                      | 82              | 32.142         | 0.392            |
| Dissonance Hero                | 106             | 54.899         | 0.518            |
| Djent Hero Collection          | 161             | 86.131         | 0.535            |
| Facelift                       | 35              | 14.246         | 0.407            |
| Focal Point                    | 170             | 81.546         | 0.480            |
| Focal Point 2                  | 186             | 87.554         | 0.471            |
| GaMetal Power Pack             | 54              | 26.062         | 0.483            |
| Guitar Hero: Guitar Zero       | 64              | 23.528         | 0.368            |
| Guitar Hero X                  | 127             | 77.984         | 0.614            |
| Koreaboo Hero                  | 51              | 15.316         | 0.300            |
| Koreaboo Hero 2                | 101             | 29.909         | 0.296            |
| Marathon Hero                  | 49              | 94.139         | 1.921            |
| Paradigm                       | 101             | 49.276         | 0.488            |
| Phase Shift Guitar Project 4   | 162             | 69.315         | 0.428            |
| Redemption Arc                 | 100             | 42.690         | 0.427            |
| The Fall of Troy Hero          | 70              | 61.074         | 0.872            |
| Vortex Hero                    | 222             | 95.530         | 0.430            |
| Zero Gravity                   | 178             | 72.189         | 0.406            |
