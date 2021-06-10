# How fast is CHOpt

CHOpt is now fast enough that I can throw multiple setlists at it, so for fun I
decided to do so and see what results come up. Don't take this too seriously:
for one I only ran this on all these songs once. I did this with CHOpt 1.3.1.

## The songs

I ran CHOpt on every song from the following setlists and packs with Expert
Lead Guitar, except Classical Thump, Dawn Patrol, Imagine, and Space Cowboy for
which I did Expert Bass.

* Angevil Hero II
* Anti Hero
* Anti Hero 2
* Anti Hero 2 Charity Drive
* Anti Hero: Beach Episode
* Anti Hero: Beach Episode - The Beach Sides
* Band Hero
* Blanket Statement
* Brand New Hero
* Carpal Tunnel Hero: Remastered
* Carpal Tunnel Hero 2
* CHARTS
* CHARTS 2
* CHARTS 2 DLC (Pack 1 - Pack 2)
* Circuit Breaker
* Community Track Packs (Pack 6 - Pack X)
* Cow Hero
* CSC Monthly Packs (June 2018 - June 2021)
* DF Discography CH
* Digitizer
* Dissonance Hero
* Dissonance Hero Free-LC (Pack 1)
* DJ Hero
* Djent Hero Collection (Pack #1 - Pack #6)
* DJMax Packs (Pack I - Pack II)
* Facelift (Pack 1 - Pack 2)
* Focal Point
* Focal Point 2
* Game Changer
* GaMetal Power Pack
* Glitter Hero
* Green Day Rock Band
* Guitar Hero
* Guitar Hero II
* Guitar Hero II DLC
* Guitar Hero Encore: Rocks the 80s
* Guitar Hero III: Legends of Rock
* Guitar Hero III: Legends of Rock DLC
* Guitar Hero: Aerosmith
* Guitar Hero World Tour
* Guitar Hero World Tour DLC
* Guitar Hero: Metallica
* Guitar Hero: Metallica DLC
* Guitar Hero Smash Hits
* Guitar Hero: Van Halen
* Guitar Hero 5
* Guitar Hero 5 DLC
* Guitar Hero: Warriors of Rock
* Guitar Hero: Warriors of Rock DLC
* Guitar Hero On Tour
* Guitar Hero On Tour: Decades
* Guitar Hero On Tour: Modern Hits
* Guitar Hero: Guitar Zero
* Guitar Hero: Guitar Zero DLC (DLC #1 - DLC #9)
* Guitar Hero X
* Holiday Overcharts (2017 - 2020)
* Koreaboo Hero
* Koreaboo Hero 2
* Lego Rock Band
* Marathon Hero
* Marathon Hero 2
* Marathon Hero 2 DLC
* Paradigm
* Phase Shift Guitar Project 4
* Plastic Shred Hero: Legends of Apathetic Charting
* Project Strandberger
* Redemption Arc
* Rock Band
* Rock Band 2
* Rock Band 3
* Rock Band ACDC Live Track Pack
* Rock Band Blitz
* The Beatles Rock Band
* The Fall of Troy Hero
* Trunks252JM's Classic Charts
* Verified Unverified
* Vortex Hero
* Xroniàl Xéro
* Zero Gravity
* Zero Gravity: Space Battle

If there's a setlist or pack you'd like to see on here, let me know.

## Results

### Overall

The combined runtime was 75m2s. Below is a histogram of the runtime for each
song, with the top 1% combined as outliers.

![Runtime histogram](runtime-histogram.svg)

The 25 slowest songs are as follows:

| Song                                             | Setlist                      | Time (s) |
| ------------------------------------------------ | ---------------------------- | -------- |
| The Forever Saga                                 | Marathon Hero 2              | 58.436   |
| CHARTS 2: Endless Setlist                        | CHARTS 2                     | 47.535   |
| CHARTS: The (almost) Endless Setlist             | CHARTS                       | 30.696   |
| Way Too Damn Much Anti Hero 2                    | Anti Hero 2 Charity Drive    | 18.708   |
| Endless Setlist: The Fall of Troy Hero           | The Fall of Troy Hero        | 18.396   |
| Vector/Virus                                     | Marathon Hero 2              | 12.919   |
| Chezy's Ultimate Solo Experience                 | CHARTS 2                     | 12.662   |
| Uranoid                                          | CHARTS                       | 11.964   |
| The Human Equation                               | Marathon Hero                | 8.710    |
| The Dark Tower (Full Album)                      | Marathon Hero 2              | 8.612    |
| The Olympian                                     | Community Track Packs        | 8.408    |
| The Tale of Iso Subject 5                        | Marathon Hero 2              | 7.605    |
| Reincarnation (Full Album)                       | Marathon Hero 2              | 7.532    |
| Catch Thirtythree                                | Djent Hero Collection        | 7.298    |
| Operation: Mindcrime (Full Album)                | Marathon Hero 2              | 7.139    |
| Under the Force of Courage [FULL ALBUM]          | Anti Hero 2 Charity Drive    | 7.132    |
| Ascendancy [FULL ALBUM]                          | Anti Hero 2 Charity Drive    | 6.590    |
| Eskapist                                         | CHARTS                       | 6.156    |
| Eskapist                                         | Marathon Hero                | 6.143    |
| Automata                                         | Marathon Hero                | 5.966    |
| Tragic Separation (Full Album)                   | Marathon Hero 2              | 5.705    |
| Periphery IV: Hail Stan (Full Album)             | CHARTS 2                     | 5.528    |
| St. Anger (Full Album)                           | Marathon Hero 2              | 5.505    |
| Coma Witch (Full Album Chart)                    | CHARTS                       | 5.421    |
| Volition (Full Album Chart)                      | CHARTS                       | 5.355    |

### By Setlist

| Setlist                                           | Number of songs | Total time (s) | Average time (s) |
| ------------------------------------------------- | --------------- | -------------- | ---------------- |
| Angevil Hero II                                   | 67              | 26.703         | 0.399            |
| Anti Hero                                         | 402             | 172.654        | 0.429            |
| Anti Hero 2                                       | 365             | 172.962        | 0.474            |
| Anti Hero 2 Charity Drive                         | 9               | 51.410         | 5.712            |
| Anti Hero: Beach Episode                          | 127             | 63.956         | 0.504            |
| Anti Hero: Beach Episode - The Beach Sides        | 25              | 11.124         | 0.445            |
| Band Hero                                         | 65              | 23.969         | 0.369            |
| Blanket Statement                                 | 115             | 43.932         | 0.382            |
| Brand New Hero                                    | 83              | 29.327         | 0.353            |
| Carpal Tunnel Hero: Remastered                    | 104             | 62.082         | 0.597            |
| Carpal Tunnel Hero 2                              | 309             | 167.814        | 0.543            |
| CHARTS                                            | 646             | 331.052        | 0.512            |
| CHARTS 2                                          | 139             | 123.012        | 0.885            |
| CHARTS 2 DLC                                      | 94              | 48.144         | 0.512            |
| Circuit Breaker                                   | 116             | 46.461         | 0.401            |
| Community Track Packs                             | 337             | 191.734        | 0.569            |
| Cow Hero                                          | 71              | 27.808         | 0.392            |
| CSC Monthly Packs                                 | 1124            | 430.173        | 0.383            |
| DF Discography CH                                 | 80              | 47.761         | 0.597            |
| Digitizer                                         | 82              | 31.328         | 0.382            |
| Dissonance Hero                                   | 106             | 53.352         | 0.503            |
| Dissonance Hero Free-LC                           | 22              | 8.151          | 0.370            |
| DJ Hero                                           | 10              | 2.544          | 0.254            |
| Djent Hero Collection                             | 200             | 100.947        | 0.505            |
| DJMax Packs                                       | 58              | 13.552         | 0.234            |
| Facelift                                          | 86              | 35.474         | 0.412            |
| Focal Point                                       | 170             | 83.584         | 0.492            |
| Focal Point 2                                     | 186             | 89.110         | 0.479            |
| Game Changer                                      | 63              | 33.269         | 0.528            |
| GaMetal Power Pack                                | 54              | 26.967         | 0.499            |
| Glitter Hero                                      | 26              | 8.255          | 0.318            |
| Green Day Rock Band                               | 43              | 14.881         | 0.346            |
| Guitar Hero                                       | 49              | 15.686         | 0.320            |
| Guitar Hero II                                    | 74              | 25.242         | 0.341            |
| Guitar Hero II DLC                                | 24              | 8.070          | 0.336            |
| Guitar Hero Encore: Rocks the 80s                 | 30              | 10.152         | 0.338            |
| Guitar Hero III: Legends of Rock                  | 70              | 24.885         | 0.355            |
| Guitar Hero III: Legends of Rock DLC              | 68              | 29.122         | 0.428            |
| Guitar Hero: Aerosmith                            | 41              | 13.813         | 0.337            |
| Guitar Hero World Tour                            | 84              | 31.681         | 0.377            |
| Guitar Hero World Tour DLC                        | 147             | 59.123         | 0.402            |
| Guitar Hero: Metallica                            | 49              | 26.458         | 0.540            |
| Guitar Hero: Metallica DLC                        | 10              | 7.029          | 0.703            |
| Guitar Hero Smash Hits                            | 48              | 18.136         | 0.378            |
| Guitar Hero: Van Halen                            | 47              | 19.075         | 0.406            |
| Guitar Hero 5                                     | 84              | 34.316         | 0.409            |
| Guitar Hero 5 DLC                                 | 158             | 56.747         | 0.359            |
| Guitar Hero: Warriors of Rock                     | 93              | 35.439         | 0.381            |
| Guitar Hero: Warriors of Rock DLC                 | 84              | 33.906         | 0.404            |
| Guitar Hero On Tour                               | 31              | 10.826         | 0.349            |
| Guitar Hero On Tour: Decades                      | 36              | 11.210         | 0.311            |
| Guitar Hero On Tour: Modern Hits                  | 44              | 13.770         | 0.313            |
| Guitar Hero: Guitar Zero                          | 64              | 22.094         | 0.345            |
| Guitar Hero: Guitar Zero DLC                      | 38              | 19.602         | 0.516            |
| Guitar Hero X                                     | 127             | 69.651         | 0.548            |
| Holiday Overcharts                                | 33              | 9.389          | 0.285            |
| Koreaboo Hero                                     | 51              | 15.265         | 0.299            |
| Koreaboo Hero 2                                   | 101             | 29.833         | 0.295            |
| Lego Rock Band                                    | 45              | 13.158         | 0.292            |
| Marathon Hero                                     | 49              | 90.502         | 1.847            |
| Marathon Hero 2                                   | 152             | 281.008        | 1.849            |
| Marathon Hero 2 DLC                               | 216             | 113.502        | 0.525            |
| Paradigm                                          | 101             | 46.141         | 0.457            |
| Phase Shift Guitar Project 4                      | 162             | 71.957         | 0.444            |
| Plastic Shred Hero: Legends of Apathetic Charting | 169             | 79.838         | 0.472            |
| Project Strandberger                              | 100             | 47.727         | 0.477            |
| Redemption Arc                                    | 100             | 42.771         | 0.428            |
| Rock Band                                         | 58              | 25.348         | 0.437            |
| Rock Band 2                                       | 84              | 29.263         | 0.348            |
| Rock Band 3                                       | 83              | 28.717         | 0.346            |
| Rock Band ACDC Live Track Pack                    | 18              | 8.337          | 0.463            |
| Rock Band Blitz                                   | 25              | 8.572          | 0.343            |
| The Beatles Rock Band                             | 45              | 11.747         | 0.261            |
| The Fall of Troy Hero                             | 70              | 59.915         | 0.856            |
| Trunks252JM's Classic Charts                      | 56              | 23.137         | 0.413            |
| Verified Unverified                               | 21              | 9.400          | 0.448            |
| Vortex Hero                                       | 222             | 93.927         | 0.423            |
| Xroniàl Xéro                                      | 17              | 9.160          | 0.539            |
| Zero Gravity                                      | 179             | 71.892         | 0.402            |
| Zero Gravity: Space Battle                        | 236             | 112.758        | 0.478            |

## Average Multiplier Outliers

I've started gathering more information when I measure performance, including
average multiplier. The 25 songs with the highest optimal average multiplier are
as follows:

| Song                                          | Setlist                                           | Average Multiplier |
| --------------------------------------------- | ------------------------------------------------- | ------------------ |
| Sin of Gluttony                               | Community Track Packs                             | 7.556x             |
| Sugar Foot Rag                                | Carpal Tunnel Hero 2                              | 7.276x             |
| Κούνια                                         | Community Track Packs                             | 7.237x             |
| This Went Too Far                             | Community Track Packs                             | 7.204x             |
| SLOW DANCING IN THE DARK                      | Glitter Hero                                      | 7.165x             |
| Ode To THR3A                                  | Community Track Packs                             | 7.128x             |
| Trojans                                       | Carpal Tunnel Hero 2                              | 7.070x             |
| Nocturnal Keys 3                              | Community Track Packs                             | 7.030x             |
| Minute of what?                               | Community Track Packs                             | 7.029x             |
| VIP Party 2                                   | Community Track Packs                             | 6.980x             |
| Shiver vCH                                    | Community Track Packs                             | 6.936x             |
| Mozart and Scatology                          | Community Track Packs                             | 6.917x             |
| Fyighfreak's Solo 2                           | Community Track Packs                             | 6.903x             |
| Trepak                                        | Holiday Overcharts                                | 6.888x             |
| Icicles                                       | Community Track Packs                             | 6.879x             |
| Downfall of Gaia                              | CSC Monthly Packs                                 | 6.874x             |
| Shards of Scorched Flesh                      | Plastic Shred Hero: Legends of Apathetic Charting | 6.835x             |
| Gee-Wiz                                       | Carpal Tunnel Hero 2                              | 6.824x             |
| Star X Speed Story Solo Medley                | Anti Hero: Beach Episode                          | 6.784x             |
| Thunder And Lightning                         | Carpal Tunnel Hero 2                              | 6.752x             |
| Dithering                                     | Paradigm                                          | 6.751x             |
| Bytes                                         | Community Track Packs                             | 6.738x             |
| Trumpet Christmas                             | Holiday Overcharts                                | 6.733x             |
| Black Hole Sun                                | Rock Band                                         | 6.696x             |
| Fat Refund                                    | Plastic Shred Hero: Legends of Apathetic Charting | 6.683x             |

There are 117 songs that cannot be 7 starred. 73 of them do not have any Star
Power; the 44 that do are as follows:

| Song                                                       | Setlist                                           | Average Multiplier |
| ---------------------------------------------------------- | ------------------------------------------------- | ------------------ |
| You Suffer                                                 | Plastic Shred Hero: Legends of Apathetic Charting | 1.000x             |
| All                                                        | CSC Monthly Packs                                 | 1.016x             |
| No, All!                                                   | CSC Monthly Packs                                 | 1.095x             |
| You Suffer (S3RL Remix)                                    | CSC Monthly Packs                                 | 1.333x             |
| March of the Machines                                      | Marathon Hero 2 DLC                               | 1.664x             |
| Curtain                                                    | Marathon Hero 2 DLC                               | 2.217x             |
| Daniel's Vision                                            | Marathon Hero 2 DLC                               | 3.039x             |
| Fuck the Kids I & II                                       | Game Changer                                      | 3.268x             |
| They Don't Have To Believe...                              | CHARTS                                            | 3.594x             |
| Hillside                                                   | CHARTS 2 DLC                                      | 3.819x             |
| Day One: Vigil                                             | Marathon Hero 2 DLC                               | 3.885x             |
| Beaver Moon                                                | Project Strandberger                              | 4.030x             |
| Uranoid                                                    | CHARTS                                            | 4.121x             |
| Tathagata                                                  | Marathon Hero 2 DLC                               | 4.131x             |
| Z:iRNiTRA                                                  | Xroniàl Xéro                                      | 4.155x             |
| Xroniàl Xéro                                               | Xroniàl Xéro                                      | 4.168x             |
| Completeness Under Incompleteness ("true prooF" Long ver.) | Xroniàl Xéro                                      | 4.202x             |
| The Mirror Cluster Genesis Theory                          | Xroniàl Xéro                                      | 4.217x             |
| Lowermost revolt ("Jerermiad" Long ver.)                   | Xroniàl Xéro                                      | 4.241x             |
| Xéroa ("préconnaiXance" Long ver.)                         | Xroniàl Xéro                                      | 4.261x             |
| Scanlines                                                  | Plastic Shred Hero: Legends of Apathetic Charting | 4.265x             |
| The Betrayal                                               | Carpal Tunnel Hero 2                              | 4.267x             |
| BANGER A.F., BROOOO!!!                                     | Xroniàl Xéro                                      | 4.277x             |
| Diastrophism ("Anamorphism" Long ver.)                     | Xroniàl Xéro                                      | 4.279x             |
| Potential for Anything                                     | Plastic Shred Hero: Legends of Apathetic Charting | 4.282x             |
| A Place Where Bugs Live -Crawling Yet Deeper-              | Xroniàl Xéro                                      | 4.299x             |
| Bleed                                                      | CHARTS                                            | 4.311x             |
| Breakout (feat. Scandroid)                                 | Phase Shift Guitar Project 4                      | 4.332x             |
| Luca (Demo)                                                | Brand New Hero                                    | 4.335x             |
| Coffee Mug (Originally by Descendents)                     | CHARTS                                            | 4.335x             |
| MADD BLEEPP                                                | Xroniàl Xéro                                      | 4.345x             |
| Shards of Scorched Flesh                                   | Carpal Tunnel Hero 2                              | 4.349x             |
| Musical Lobster                                            | Community Track Packs                             | 4.353x             |
| ƒi?orz?                                                    | Xroniàl Xéro                                      | 4.355x             |
| Introduction - Xursed divinitiY                            | Xroniàl Xéro                                      | 4.361x             |
| Xronièr ("genèXe" Long ver.)                               | Xroniàl Xéro                                      | 4.361x             |
| Cascading Failures, Diminishing Returns                    | Carpal Tunnel Hero 2                              | 4.366x             |
| Built This Pool (blink-182 Cover)                          | CHARTS                                            | 4.368x             |
| The Pretender                                              | Paradigm                                          | 4.378x             |
| Moniker                                                    | Community Track Packs                             | 4.381x             |
| Infinitesimal to the Infinite                              | Zero Gravity                                      | 4.382x             |
| Get Possessed                                              | Anti Hero                                         | 4.386x             |
| =El=Dorado=                                                | Xroniàl Xéro                                      | 4.396x             |
| Monkey Wrench                                              | Guitar Hero II                                    | 4.400x             |

Note the average multiplier for Monkey Wrench is rounding from 4.39995x. Clone
Hero would display the average multiplier as 4.400x, but 5 more points would be
needed for the 7 star.
