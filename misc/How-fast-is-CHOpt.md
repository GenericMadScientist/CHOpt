# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.1.1.

## The songs

I ran CHOpt on every song from the following setlists and packs with Expert
Lead Guitar, except for Classical Thump and Space Cowboy for which I did Expert
Bass.

* Angevil Hero II
* Anti Hero
* Anti Hero: Beach Episode
* Anti Hero: Beach Episode - The Beach Sides
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
* Guitar Hero: Guitar Zero DLC (DLC #1 to DLC #8)
* Guitar Hero X
* Koreaboo Hero
* Koreaboo Hero 2
* Marathon Hero
* Paradigm
* Phase Shift Guitar Project 4
* Redemption Arc
* The Fall of Troy Hero
* Trunks252JM's Classic Charts
* Vortex Hero
* Zero Gravity

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 41m34s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                             | Setlist                      | Time (s) |
| ------------------------------------------------ | ---------------------------- | -------- |
| CHARTS 2: Endless Setlist                        | CHARTS 2                     | 50.066   |
| CHARTS: The (almost) Endless Setlist             | CHARTS                       | 30.898   |
| Endless Setlist: The Fall of Troy Hero           | The Fall of Troy Hero        | 18.308   |
| Chezy's Ultimate Solo Experience                 | CHARTS 2                     | 11.926   |
| Uranoid                                          | CHARTS                       | 11.566   |
| The Human Equation                               | Marathon Hero                | 9.181    |
| Catch Thirtythree                                | Djent Hero Collection        | 8.686    |
| Eskapist                                         | Marathon Hero                | 6.423    |
| Eskapist                                         | CHARTS                       | 6.378    |
| Automata                                         | Marathon Hero                | 6.117    |
| Periphery IV: Hail Stan (Full Album)             | CHARTS 2                     | 5.799    |
| Coma Witch (Full Album Chart)                    | CHARTS                       | 5.747    |
| The Future in Whose Eyes? (Full Album)           | CHARTS 2                     | 5.632    |
| Volition (Full Album Chart)                      | CHARTS                       | 5.328    |
| Extreme Power Metal (Full Album)                 | Guitar Hero: Guitar Zero DLC | 5.078    |
| Nothing More [FULL ALBUM]                        | CSC Monthly Packs            | 5.001    |
| Live at Dynamo Open Air 1998 [FULL ALBUM]        | CSC Monthly Packs            | 4.659    |
| That's The Spirit (Full Album Chart)             | CHARTS                       | 4.358    |
| Jane Doe (Full Album)                            | Dissonance Hero              | 4.305    |
| Bloodwork (Full Album Chart)                     | CHARTS                       | 4.180    |
| Low Teens (Full Album Chart)                     | CHARTS                       | 4.082    |
| Time Will Die And Love Will Bury It (Full Album) | Dissonance Hero              | 3.857    |
| Culture Scars (Full Album Chart)                 | CHARTS                       | 3.755    |
| GaMetal Solo Medley                              | GaMetal Power Pack           | 3.752    |
| In the Unlikely Event                            | The Fall of Troy Hero        | 3.734    |

### By Setlist

| Setlist                                    | Number of songs | Total time (s) | Average time (s) |
| ------------------------------------------ | --------------- | -------------- | ---------------- |
| Angevil Hero II                            | 67              | 25.652         | 0.383            |
| Anti Hero                                  | 402             | 161.173        | 0.401            |
| Anti Hero: Beach Episode                   | 127             | 59.104         | 0.465            |
| Anti Hero: Beach Episode - The Beach Sides | 25              | 9.332          | 0.373            |
| Anti Hero 2                                | 365             | 151.417        | 0.415            |
| Blanket Statement                          | 115             | 40.009         | 0.348            |
| Brand New Hero                             | 83              | 27.633         | 0.333            |
| Carpal Tunnel Hero: Remastered             | 104             | 58.540         | 0.563            |
| Carpal Tunnel Hero 2                       | 309             | 160.817        | 0.520            |
| CHARTS                                     | 646             | 312.289        | 0.483            |
| CHARTS 2                                   | 139             | 122.726        | 0.883            |         
| Circuit Breaker                            | 116             | 42.837         | 0.369            |
| Cow Hero                                   | 71              | 25.751         | 0.363            |
| CSC Monthly Packs                          | 762             | 277.247        | 0.364            |
| DF Discography CH                          | 80              | 45.269         | 0.566            |
| Digitizer                                  | 82              | 29.156         | 0.356            |
| Dissonance Hero                            | 106             | 50.461         | 0.476            |
| Djent Hero Collection                      | 161             | 80.029         | 0.497            |
| Facelift                                   | 35              | 12.589         | 0.360            |
| Focal Point                                | 170             | 77.523         | 0.456            |
| Focal Point 2                              | 186             | 82.128         | 0.442            |
| GaMetal Power Pack                         | 54              | 25.589         | 0.474            |
| Guitar Hero: Guitar Zero                   | 64              | 20.044         | 0.313            |
| Guitar Hero: Guitar Zero DLC               | 35              | 17.769         | 0.508            |
| Guitar Hero X                              | 127             | 64.922         | 0.511            |
| Koreaboo Hero                              | 51              | 14.106         | 0.277            |
| Koreaboo Hero 2                            | 101             | 27.324         | 0.271            |
| Marathon Hero                              | 49              | 91.578         | 1.869            |
| Paradigm                                   | 101             | 42.858         | 0.424            |
| Phase Shift Guitar Project 4               | 162             | 66.115         | 0.408            |
| Redemption Arc                             | 100             | 39.136         | 0.391            |
| The Fall of Troy Hero                      | 70              | 56.974         | 0.814            |
| Trunks252JM's Classic Charts               | 56              | 21.413         | 0.382            |
| Vortex Hero                                | 222             | 88.519         | 0.399            |
| Zero Gravity                               | 179             | 66.122         | 0.369            |

## Average Multiplier Outliers

I've started gathering more information when I measure performance, including
average multiplier. The 25 songs with the highest optimal average multiplier are
as follows:

| Song                                          | Setlist                  | Average Multiplier |
| --------------------------------------------- | ------------------------ | ------------------ |
| Sugar Foot Rag                                | Carpal Tunnel Hero 2     | 7.276x             |
| Trojans                                       | Carpal Tunnel Hero 2     | 7.070x             |
| Downfall of Gaia                              | CSC Monthly Packs        | 6.874x             |
| Gee-Wiz                                       | Carpal Tunnel Hero 2     | 6.824x             |
| Star X Speed Story Solo Medley                | Anti Hero: Beach Episode | 6.784x             |
| Thunder And Lightning                         | Carpal Tunnel Hero 2     | 6.752x             |
| Dithering                                     | Paradigm                 | 6.751x             |
| lifeisgood                                    | CHARTS 2                 | 6.658x             |
| All Is One                                    | Vortex Hero              | 6.572x             |
| Solace                                        | Djent Hero Collection    | 6.554x             |
| Away / Poetic Justice                         | Vortex Hero              | 6.463x             |
| Christmas Time is Here (Vince Guaraldi cover) | CSC Monthly Packs        | 6.455x             |
| ETERNAL NOW                                   | Paradigm                 | 6.433x             |
| PN35                                          | CHARTS 2                 | 6.427x             |
| PN35                                          | Circuit Breaker          | 6.427x             |
| Gamer National Anthem (feat. Coey)            | CSC Monthly Packs        | 6.418x             |
| Neonatalimpalionecrophiliation                | Carpal Tunnel Hero 2     | 6.415x             |
| Tapping Boogie                                | Paradigm                 | 6.352x             |
| Mostly Hair And Bones Now                     | Dissonance Hero          | 6.344x             |
| Rock Smash                                    | Anti Hero 2              | 6.342x             |
| Because We Can                                | Anti Hero: Beach Episode | 6.335x             |
| Nocturne                                      | Focal Point 2            | 6.332x             |
| Into Thin Air                                 | Paradigm                 | 6.331x             |
| M.G. III                                      | Anti Hero                | 6.321x             |
| Experiment                                    | Carpal Tunnel Hero 2     | 6.312x             |

There are 42 songs that cannot be 7 starred. 29 of them do not have any Star
Power; the 13 that do are as follows:

| Song                                    | Setlist                      | Average Multiplier |
| --------------------------------------- | ---------------------------- | ------------------ |
| They Don't Have To Believe...           | CHARTS                       | 3.594x             |
| Uranoid                                 | CHARTS                       | 4.121x             |
| The Betrayal                            | Carpal Tunnel Hero 2         | 4.267x             |
| Bleed                                   | CHARTS                       | 4.311x             |
| Breakout (feat. Scandroid)              | Phase Shift Guitar Project 4 | 4.332x             |
| Luca (Demo)                             | Brand New Hero               | 4.335x             |
| Coffee Mug (Originally by Descendents)  | CHARTS                       | 4.335x             |
| Shards of Scorched Flesh                | Carpal Tunnel Hero 2         | 4.349x             |
| Cascading Failures, Diminishing Returns | Carpal Tunnel Hero 2         | 4.366x             |
| Built This Pool (blink-182 Cover)       | CHARTS                       | 4.368x             |
| The Pretender                           | Paradigm                     | 4.378x             |
| Infinitesimal to the Infinite           | Zero Gravity                 | 4.382x             |
| Get Possessed                           | Anti Hero                    | 4.386x             |
